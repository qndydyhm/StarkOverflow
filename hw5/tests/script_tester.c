#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "pbx.h"
#include "server.h"
#include "__test_includes.h"
#include "debug.h"

#define NUM_STATES 7
#define NUM_COMMANDS 5
#define DELAY_COMMAND (NUM_COMMANDS-1)

/*
 * Table of expected next states.
 * Each entry is a bitmap that specifies a set of possible next states, given
 * the current state and the last command that was issued.
 *
 * An issue that this tester has to handle is that commands to the server can
 * "cross in transit" asynchronous state-change notifications coming back from the server.
 * If we are currently in the TU_ON_HOOK state and we send a TU_PICKUP_CMD, it might
 * be that the TU_PICKUP_CMD crosses in transit a TU_RINGING notification being sent
 * back to us.  What we will see is a next-state notification of TU_RINGING, rather
 * than the TU_DIAL_TONE notification that we would otherwise expect.
 *
 * To handle this, there are two classes of expected states encoded in each entry of
 * the table.  The "normal case" encodes a TU_STATE s as the bit value 1<<s, and it
 * indicates a state that we would expect to see if there were no "crossing in transit".
 * The "abnormal case" encodes additional states that we might see when messages
 * cross in transit.  These are encoded as 1<<(s+RESYNC), where RESYNC is larger than
 * any TU_STATE value.  When we receive a state notification, it is checked against
 * the expected state bitmap.  If we find that state among the "normal case" states,
 * then nothing special happens and we proceed on to selecting the next command to send.
 * On the other hand, if we find that state among the "abnormal case" states, then
 * a "resync" flag is set and we do not immediately select a new command to send.
 * Instead, we assume that what we have just received is an asynchronous state-change
 * notification that crossed in transit our last command, and that the response to
 * our last command is still forthcoming.  In this situation, we redetermine the set
 * of expected events based on the new state, but the last command that we sent.
 * When we finally do receive a "normal case" response, then the resynchronization is
 * over and we proceed to send another command.
 *
 * A deficiency in the current implementation is that there ought to be a timeout after
 * which we declare failure if a resynchronization has not completed within a short
 * period of time.
 *
 * Another deficiency at the moment is that the tester tests that "bad things don't happen",
 * but it doesn't really check that "good things do happen" (e.g. that calls get connected).
 *
 * One other deficiency is in the treatment of delays.  When the action chosen from a state
 * is to delay, the delays will continue until a non-delay action is chosen, without reading
 * any notifications from the server until the delay period is over.  It would be better if
 * the arrival of notifications from the server was checked after each basic delay, but that
 * would further complicate the program and it has not been implemented at this time.
 */

#define RESYNC NUM_STATES

int next_states[NUM_STATES][NUM_COMMANDS] = {
  [TU_ON_HOOK] {
      1<<TU_DIAL_TONE | 1<<(TU_RINGING+RESYNC) | 1<<(TU_ON_HOOK+RESYNC),    // TU_PICKUP_CMD
      1<<TU_ON_HOOK | 1<<(TU_RINGING+RESYNC),                               // TU_HANGUP_CMD
      1<<TU_ON_HOOK | 1<<(TU_RINGING+RESYNC),                               // TU_DIAL_CMD
      1<<TU_ON_HOOK | 1<<(TU_RINGING+RESYNC),                               // TU_CHAT_CMD
      1<<(TU_ON_HOOK+RESYNC) | 1<<(TU_RINGING+RESYNC)                       // DELAY
  },
  [TU_RINGING] {
      1<<TU_CONNECTED | 1<<(TU_ON_HOOK+RESYNC) | 1<<(TU_RINGING+RESYNC),    // TU_PICKUP_CMD
      1<<TU_ON_HOOK | 1<<(TU_RINGING+RESYNC),                               // TU_HANGUP_CMD
      1<<TU_RINGING | 1<<(TU_ON_HOOK+RESYNC),                               // TU_DIAL_CMD
      1<<TU_RINGING | 1<<(TU_ON_HOOK+RESYNC),                               // TU_CHAT_CMD
      1<<(TU_RINGING+RESYNC) | 1<<(TU_ON_HOOK+RESYNC)                       // DELAY
  },
  [TU_DIAL_TONE] {
      1<<TU_DIAL_TONE,                                                      // TU_PICKUP_CMD
      1<<TU_ON_HOOK | 1<<(TU_DIAL_TONE+RESYNC),                             // TU_HANGUP_CMD
      1<<TU_RING_BACK | 1<<TU_BUSY_SIGNAL | 1<<TU_ERROR
                      | 1<<(TU_DIAL_TONE+RESYNC),                           // TU_DIAL_CMD
      1<<TU_DIAL_TONE,                                                      // TU_CHAT_CMD
      1<<(TU_DIAL_TONE+RESYNC)                                              // DELAY
  },
  [TU_RING_BACK] {
      1<<TU_RING_BACK | 1<<(TU_CONNECTED+RESYNC) | 1<<(TU_DIAL_TONE+RESYNC),// TU_PICKUP_CMD
      1<<TU_ON_HOOK | 1<<(TU_CONNECTED+RESYNC) | 1<<(TU_DIAL_TONE+RESYNC)
                    | 1<<(TU_RING_BACK+RESYNC),                             // TU_HANGUP_CMD
      1<<TU_RING_BACK | 1<<(TU_CONNECTED+RESYNC) | 1<<(TU_DIAL_TONE+RESYNC),// TU_DIAL_CMD
      1<<TU_RING_BACK | 1<<(TU_CONNECTED+RESYNC) | 1<<(TU_DIAL_TONE+RESYNC),// TU_CHAT_CMD
      1<<(TU_RING_BACK+RESYNC) | 1<<(TU_CONNECTED+RESYNC) | 1<<(TU_DIAL_TONE+RESYNC) // DELAY
  },
  [TU_BUSY_SIGNAL] {
      1<<TU_BUSY_SIGNAL,                                                    // TU_PICKUP_CMD
      1<<TU_ON_HOOK | 1<<(TU_BUSY_SIGNAL+RESYNC),                           // TU_HANGUP_CMD
      1<<TU_BUSY_SIGNAL,                                                    // TU_DIAL_CMD
      1<<TU_BUSY_SIGNAL,                                                    // TU_CHAT_CMD
      1<<(TU_BUSY_SIGNAL+RESYNC)                                            // DELAY
  },
  [TU_CONNECTED] {
      1<<TU_CONNECTED | 1<<(TU_DIAL_TONE+RESYNC) | 1<<(TU_CONNECTED+RESYNC),// TU_PICKUP_CMD
      1<<TU_ON_HOOK | 1<<(TU_DIAL_TONE+RESYNC) | 1<<(TU_CONNECTED+RESYNC),  // TU_HANGUP_CMD
      1<<TU_CONNECTED | 1<<(TU_DIAL_TONE+RESYNC),                           // TU_DIAL_CMD
      1<<TU_CONNECTED | 1<<(TU_DIAL_TONE+RESYNC),                           // TU_CHAT_CMD
      1<<(TU_CONNECTED+RESYNC) | 1<<(TU_DIAL_TONE+RESYNC)                   // DELAY
  },
  [TU_ERROR] {
      1<<TU_ERROR,                                                          // TU_PICKUP_CMD
      1<<TU_ON_HOOK | 1<<(TU_ERROR+RESYNC),                                 // TU_HANGUP_CMD
      1<<TU_ERROR,                                                          // TU_DIAL_CMD
      1<<TU_ERROR,                                                          // TU_CHAT_CMD
      1<<(TU_ERROR+RESYNC)                                                  // DELAY
  }
};

/*
 * Structure that records the state of a single TU under test.
 */
typedef struct tu {
    /* File descriptor for input from server connection, or 0 if not connected. */
    int infd;

    /* File descriptor for output to server connection, or 0 if not connected. */
    int outfd;

    /* Input stream from the server. */
    FILE *in;
   
    /* Output stream to the server. */
    FILE *out;

    /* Extension number we have been assigned by the server. */
    int extension;

    /* Extension to which we are connnected. */
    int peer;

    /* The current state of the TU simulated by the tester. */
    TU_STATE current_state;

    /*
     * A bitmap that specifies the set of next states we expect, as described above.
     * A transition to a state not in this set results in a test failure.
     */
    int expected_states;
    
    /*
     * Flag that indicates whether we are currently resynchronizing, as a result of
     * due to messages that "crossed in transit".
     */
    int resync;

    /*
     * The last command sent to the server.  This is used during resynchronization,
     * to determine a new set of expected states as each successive asynchronous
     * notification is received.
     */
    TU_COMMAND last_command;
} TU;

/*
 * Table giving the states of all TUs under test.
 */
#define MAX_TUS 20
TU tus[MAX_TUS];

#define TU_ID(tu) ((tu) - &tus[0])

/*
 * "Meta-commands" for the test script.
 * These are not actual TU commands, but rather specify other actions to
 * be performed during a test script.
 */
#if 0
#define TU_NO_CMD          100
#define TU_CONNECT_CMD     101
#define TU_DISCONNECT_CMD  102
#define TU_AWAIT_CMD	   103  // Await a specified TU state (e.g. TU_RINGING)
#define TU_DELAY_CMD       104  // Timeout specifies time delay
#endif

/*
 * Sample test script.
 * Later these will be provided by individual Criterion tests.
 */
#if 0
TEST_STEP sample_test_script[] = {
    // ID,  COMMAND,          ID_TO_DIAL,    RESPONSE,       TIMEOUT
    {   0,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     { 1, 0 }},
    {   1,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     { 1, 0 }},
    {   1,  TU_PICKUP_CMD,     -1,           TU_DIAL_TONE,   { 1, 0 }},
    {   1,  TU_DIAL_CMD,        0,           TU_RING_BACK,   { 1, 0 }},
    {   0,  TU_AWAIT_CMD,      -1,           TU_RINGING,     { 1, 0 }},
    {   0,  TU_PICKUP_CMD,     -1,           TU_CONNECTED,   { 1, 0 }},
    {   0,  TU_DISCONNECT_CMD, -1,           -1,             { 1, 0 }},
    {   1,  TU_DISCONNECT_CMD, -1,           -1,             { 1, 0 }},
    {   -1, -1,                -1,           -1,             { 0, 0 }}
};
#endif

/* Prototypes for functions that appear below. */
static void test(FILE *in, FILE *out, int cmds);
static int choose_action(void);
static TU_STATE parse_message(char *msg, char **arg);
static char *unparse_state_set(int set);
static void trim_eol(char *msg);
static char *timestamp(void);
static int connect_command(TU *tu, int port);
static void disconnect_command(TU *tu);
static int connect_to_server(struct in_addr *addr, int port);
static int read_responses(TU *tu, TU_STATE exp, struct timeval tv);

/*
 * Temporary main until this is fleshed out.
 * Then it will be hooked to Criterion.
 */
#if 0
int main(int argc, char *argv[]) {
    if(run_test_script(sample_test_script, SERVER_PORT) == -1) {
	debug("Script failed");
	return EXIT_FAILURE;
    } else {
	debug("Script succeeded");
	return EXIT_SUCCESS;
    }
}
#endif

static void alert(int sig) {
    fprintf(stderr, "Unexpected signal: %d\n", sig);
    abort();
}

/*
 * Run a test script provided as a parameter.
 * Returns 0 on success, -1 on failure.
 */
int run_test_script(char *name, TEST_STEP *scr, int port) {
    fprintf(stderr, "Running test %s\n", name);
    signal(SIGPIPE, alert);
    signal(SIGSEGV, alert);
    signal(SIGHUP, alert);
    signal(SIGINT, alert);
    TEST_STEP *ts = scr;
    struct timespec tms = {0};
    while(ts->id != -1) {
	if(ts->id >= MAX_TUS) {
	    fprintf(stderr, "Script error: TU ID %d too large (>= %d)\n", ts->id, MAX_TUS);
	    return -1;
	}
	int cmd = ts->command;
	int ext = -1;
	TU *tu = &tus[ts->id];

	// First, deal with performing any explicit action.
	switch(ts->command) {
	// Meta-commands
	case TU_NO_CMD:
	    fprintf(stderr, "%s: [%ld] (step #%ld) TU_NO_CMD\n", timestamp(), TU_ID(tu), ts - scr);
	    break;
	case TU_CONNECT_CMD:
	    fprintf(stderr, "%s: [%ld] (step #%ld) TU_CONNECT_CMD\n", timestamp(), TU_ID(tu), ts - scr);
	    if(tu->infd) {
		fprintf(stderr, "%s: [%ld] Test error: already connected\n",
			timestamp(), TU_ID(tu));
		return -1;
	    }
	    // Otherwise connect to server and update state.
	    if(connect_command(tu, port) == -1)
		return -1;
	    break;
	case TU_DISCONNECT_CMD:
	    fprintf(stderr, "%s: [%ld] (step #%ld) TU_DISCONNECT_CMD\n", timestamp(), TU_ID(tu), ts - scr);
	    if(!tu->infd) {
		fprintf(stderr, "%s: [%ld] Test error: not connected\n", timestamp(), TU_ID(tu));
		return -1;
	    }
	    disconnect_command(tu);
	    break;
	case TU_DELAY_CMD:
	    fprintf(stderr, "%s: [%ld] (step #%ld) TU_DELAY_CMD\n", timestamp(), TU_ID(tu), ts - scr);
	    // Pause for the specified amount of time.
	    tms.tv_sec = ts->timeout.tv_sec;
	    tms.tv_nsec = ts->timeout.tv_usec * 1000l;
	    nanosleep(&tms, NULL);
	    break;
	case TU_AWAIT_CMD:
	    fprintf(stderr, "%s: [%ld] (step #%ld) TU_AWAIT_CMD\n", timestamp(), TU_ID(tu), ts - scr);
	    // Process incoming messages until specified state seen
	    // or timeout occurs.
	    break;
	
	// Real commands
	case TU_PICKUP_CMD:
	case TU_HANGUP_CMD:
	    fprintf(stderr, "%s: [%ld] (step #%ld) %s\n",
		    timestamp(), TU_ID(tu), ts - scr, tu_command_names[cmd]);
	    fprintf(tu->out, "%s%s", tu_command_names[cmd], EOL);
	    fflush(tu->out);
	    break;
	case TU_DIAL_CMD:
	    ext = tus[ts->id_to_dial].extension;
	    fprintf(stderr, "%s: [%ld] (step #%ld) %s extension %d (id %d)\n",
		    timestamp(), TU_ID(tu), ts - scr, tu_command_names[cmd], ext, ts->id_to_dial);
	    fprintf(tu->out, "%s %d%s", tu_command_names[cmd], ext, EOL);
	    fflush(tu->out);
	    break;
	case TU_CHAT_CMD:
	    fprintf(stderr, "%s: [%ld] (step #%ld) %s\n",
		    timestamp(), TU_ID(tu), ts - scr, tu_command_names[cmd]);
	    fprintf(tu->out, "%s%s", tu_command_names[cmd], EOL);
	    fflush(tu->out);
	    break;

	// Unknown command
	default:
	    fprintf(stderr, "%s: [%ld] (step #%ld) Test error: unknown command (%d)\n",
		  timestamp(), TU_ID(tu), ts - scr, cmd);
	    return -1;
	}
	if(cmd <= TU_CHAT_CMD) {
	    tu->last_command = cmd;
	    tu->expected_states = next_states[tu->current_state][cmd];
	} else if(cmd == TU_CONNECT_CMD) {
	    // This is to get the right set of expected commands on initial connect,
	    // when no previous command has actually been sent.
	    tu->last_command = TU_HANGUP_CMD;
	    tu->expected_states = next_states[tu->current_state][TU_HANGUP_CMD];
	} else if(cmd == TU_DISCONNECT_CMD) {
	    fprintf(stderr, "%s: [%ld] Disconnected, now expecting EOF\n", timestamp(), TU_ID(tu));
	    tu->last_command = cmd;
	    tu->expected_states = ~0;  // We allow anything to drain pending notifications.
	} else {
	    // For pseudo-commands, just recalculate the expected states based on
	    // the last real command.
	    tu->expected_states = next_states[tu->current_state][tu->last_command];
	}

	// Next, read responses while keeping track of timeout.
	// If expected response seen, go to next step.
	// If unexpected response seen, fail.
	// If timeout occurs, shutdown the connection so that read will fail.
	if(tu->infd && read_responses(tu, ts->response, ts->timeout) == -1)
	    return -1;

	// Advance script to next test step.
	ts++;
    }
    return 0;
}

/*
 * Connect a specified TU to the server.
 * Returns 0 on success, -1 on error.
 */
static int connect_command(TU *tu, int port) {
    char *hostname = "localhost";
    struct in_addr sa;
    struct hostent *he;
    int sfd;

    // Connect to server on specified port and set sfd.
    if((he = gethostbyname(hostname)) == NULL) {
	herror("gethostbyname");
	return -1;
    }
    memcpy(&sa, he->h_addr, sizeof(sa));
    if((sfd = connect_to_server(&sa, port)) == -1) {
	fprintf(stdout, "%s [%ld]: Failed to connect to server %s:%d\n",
		timestamp(), TU_ID(tu), hostname, port);
	return -1;
    }
    struct sockaddr_in s;
    socklen_t sl = sizeof(s);
    getsockname(sfd, (struct sockaddr *)&s, &sl);
    port = s.sin_port;
    fprintf(stdout, "%s: [%ld] Connected to server %s:%d\n",
	    timestamp(), TU_ID(tu), hostname, port);

    // Save file descriptor and set up streams and initial test state.
    memset(tu, 0, sizeof(*tu));
    tu->infd = sfd;
    tu->outfd = dup(sfd);  // So they can be closed independently.
    tu->in = fdopen(tu->infd, "r");
    tu->out = fdopen(tu->outfd, "w");

    // Initial expected state notification is TU_ON_HOOK
    tu->expected_states = 1<<TU_ON_HOOK;

    // Set initial last command to TU_HANGUP so that updating expected states
    // works properly.
    tu->last_command = TU_HANGUP_CMD;

    return 0;
}

/*
 * Disconnect a specified TU from the server.
 */
static void disconnect_command(TU *tu) {
    if(tu->outfd) {
	shutdown(tu->outfd, SHUT_WR);  // This lets us see if the server notices.
	tu->outfd = 0;
    }
    if(tu->out) {
	fclose(tu->out);
	tu->out = NULL;
    }
    // Closing the input should go where we detect EOF.
#if 0
    if(tu->in) {
	fclose(tu->in);
	tu->in = NULL;
    }
#endif
}

/* There isn't really a maximum message length, but this is just a test driver... */
#define MAX_MESSAGE_LEN 256

static struct timeval current_timeout;

static TU *tu_to_read;
static void alarm_handler(int sig) {
  fprintf(stderr, "%s: [%ld] Timeout (%ld, %ld)\n", timestamp(), TU_ID(tu_to_read),
	  current_timeout.tv_sec, current_timeout.tv_usec);
  shutdown(tu_to_read->infd, SHUT_RD);  // Force return from fgets
}

/*
 * Read responses from the server for a specified TU until an expected state is reached.
 */
static int read_responses(TU *tu, TU_STATE exp, struct timeval tv) {
    TU_STATE new;
    char msg[MAX_MESSAGE_LEN];
    char *arg;
    int ret = 0;
    fprintf(stderr, "%s: [%ld] Read responses until %s\n",
	    timestamp(), TU_ID(tu), exp == -1 ? "EOF" : tu_state_names[exp]);
    tu_to_read = tu;
    struct itimerval itv = {0};
    struct sigaction sa = {0}, oa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART;
    itv.it_value = tv;
    current_timeout = tv;
    sigaction(SIGALRM, &sa, &oa);
    setitimer(ITIMER_REAL, &itv, NULL);
    memset(&itv, 0, sizeof(itv));
    itv.it_value = tv;
    setitimer(ITIMER_REAL, &itv, NULL);
    do {
	fprintf(stderr, "%s: [%ld] Expecting: %s\n", timestamp(), TU_ID(tu),
		unparse_state_set(tu->expected_states));

	if(fgets(msg, MAX_MESSAGE_LEN, tu->in) == NULL) {
	    fprintf(stderr, "%s: [%ld] EOF reading message from server\n", timestamp(), TU_ID(tu));
	    fclose(tu->in);
	    tu->infd = 0;
	    if(tu->resync) {
		fprintf(stderr, "%s: [%ld] Premature disconnection during resync\n",
			timestamp(), TU_ID(tu));
		ret = -1;
		goto disarm;
	    } else {
		if(tu->expected_states == ~0) {
		    fprintf(stderr, "%s: [%ld] Matched EOF after disconnect\n",
			    timestamp(), TU_ID(tu));
		    goto disarm;
		} else {
		    if(exp == -1) {
			fprintf(stderr, "%s: [%ld] Expected EOF correctly seen\n",
				timestamp(), TU_ID(tu));
		    } else {
			fprintf(stderr, "%s: [%ld] EOF seen when it shouldn't have been\n",
				timestamp(), TU_ID(tu));
			ret = -1;
		    }
		    goto disarm;
		}
	    }
	}
	trim_eol(msg);
	fprintf(stderr, "%s: [%ld] Message from server: %s\n", timestamp(), TU_ID(tu), msg);
	new = parse_message(msg, &arg);
	if(new > NUM_STATES) {
	    // Tracing output already produced by parse_message.
	    ret = -1;
	    goto disarm;
	}
	if(new == NUM_STATES) {
	    // The message is chat.  There is no state transition, but we must be
	    // in the connected state.
	    if(tu->current_state != TU_CONNECTED) {
		fprintf(stderr, "%s: [%ld] Chat received when not in state %s\n",
			timestamp(), TU_ID(tu), tu_state_names[tu->current_state]);
		ret = -1;
		goto disarm;
	    }
	    continue;
	}

	// Check state transition to see if it is as expected.
	if(1<<new & tu->expected_states) {
	    // OK
	    tu->resync = 0;
	} else if(1<<(new+RESYNC) & tu->expected_states) {
	    // OK, but set resync because messages crossed in transit.
	    fprintf(stderr, "%s: [%ld] Resync: state %s, expecting %s\n",
		    timestamp(), TU_ID(tu), tu_state_names[new],
		    unparse_state_set(tu->expected_states));
	    tu->resync = 1;
	} else {
	    // New state is not one that is expected -- testing fails.
	    fprintf(stderr, "%s: [%ld] New state %s is not in expected set %s\n",
		    timestamp(), TU_ID(tu), tu_state_names[new],
		    unparse_state_set(tu->expected_states));
		ret = -1;
		goto disarm;
	}

	// Update current state to that specified in message
	fprintf(stderr, "%s: [%ld] Change state: %s -> %s\n",
		timestamp(), TU_ID(tu), tu_state_names[tu->current_state], tu_state_names[new]);
	tu->current_state = new;
	if(new == TU_ON_HOOK) {
	    int ext = atoi(arg);
	    if(tu->extension != ext) {
		tu->extension = ext;
	    }
	}
	if(new == TU_CONNECTED) {
	    int ext = atoi(arg);
	    tu->peer = ext;
	} else {
	    tu->peer = -1;
	}

	if(tu->resync) {
	    // If resyncing, update expected states based on last command sent,
	    // unless we are draining to get EOF.
	    //fprintf(stderr, "%s: [%ld] Resync\n", timestamp(), TU_ID(tu));
	    if(tu->expected_states != ~0)
		tu->expected_states = next_states[new][tu->last_command];
	}
    } while(tu->current_state != exp);

 disarm:
    itv = (struct itimerval) {0};
    setitimer(ITIMER_REAL, &itv, NULL);
    sigaction(SIGALRM, &oa, NULL);
    tu_to_read = NULL;
    return ret;
}

/*
 * Parse a message from the PBX, determining the new state.
 */
static TU_STATE parse_message(char *msg, char **arg) {
    for(int i = 0; i < NUM_STATES; i++) {
      if(strstr(msg, tu_state_names[i]) == msg) {
	  if(arg)
	      *arg = msg + strlen(tu_state_names[i]);
	  return i;
      }
    }
    if(strstr(msg, "CHAT") == msg) {
	  if(arg)
	      *arg = msg + strlen("CHAT");
	  return NUM_STATES;
    }
    fprintf(stderr, "%s: Unrecognized message: %s\n", timestamp(), msg);
    return NUM_STATES+1;
}

/*
 * Connect to the server at a specified address.
 *
 * Returns: connection file descriptor in case of success.
 * Returns -1 and sets errno in case of error.
 */
static int connect_to_server(struct in_addr *addr, int port) {
    struct sockaddr_in sa;
    int sfd;

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	return(-1);
    }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    memcpy(&sa.sin_addr.s_addr, addr, sizeof(struct in_addr));
    if(connect(sfd, (struct sockaddr *)(&sa), sizeof(sa)) < 0) {
	close(sfd);
	return(-1);
    }
    return sfd;
}

/*
 * Construct a string representation of an expected state bitmap.
 */
static char *unparse_state_set(int set) {
    static char buf[100];
    buf[0] = '\0';
    strcat(buf, "{ ");
    for(int i = 0; i < NUM_STATES; i++) {
	if(set & (1<<i) || set & (1<<(i+RESYNC))) {
	    strcat(buf, tu_state_names[i]);
	    if(set & (1<<(i+RESYNC)))
		strcat(buf, "*");
	    strcat(buf, " ");
	}
    }
    strcat(buf, "}");
    return buf;
}

/*
 * Trim EOL characters from the end of a message.
 */
static void trim_eol(char *msg) {
    for(char *mp = msg; *mp != '\0'; mp++) {
	if(*mp == '\n' || *mp == '\r')
	    *mp = '\0';
    }
}

/*
 * Construct a timestamp string for tracing printout.
 */
static char *timestamp() {
    static char buf[100];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    sprintf(buf, "%ld.%06ld", tv.tv_sec, tv.tv_usec);
    return buf;
}
