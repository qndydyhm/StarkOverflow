#include "pbx.h"
#include "server.h"

#define QUOTE1(x) #x
#define QUOTE(x) QUOTE1(x)
#define SCRIPT1(x) x##_script
#define SCRIPT(x) SCRIPT1(x)

#define SERVER_PORT 9999
#define SERVER_PORT_STR "9999"
#define SERVER_HOSTNAME "localhost"

#define NUM_STATES 7
#define NUM_COMMANDS 5
#define DELAY_COMMAND (NUM_COMMANDS-1)

#define ZERO_SEC { 0, 0 }
#define ONE_USEC { 0, 1 }
#define ONE_MSEC { 0, 1000 }
#define TEN_MSEC { 0, 10000 }
#define FTY_MSEC { 0, 50000 }
#define HND_MSEC { 0, 100000 }
#define QTR_SEC  { 0, 250000 }
#define ONE_SEC { 1, 0 }

#define SERVER_STARTUP_SLEEP 1
#define SERVER_SHUTDOWN_SLEEP 1

/*
 * Structure describing a single step in a test script.
 */
typedef struct test_step {
    int id;                        // Index of TU performing test, or -1 if end.
    TU_COMMAND command;            // Command to send.
    int id_to_dial;                // ID of TU to dial, for TU_DIAL command.
    TU_STATE response;		   // Expected response.
    struct timeval timeout;        // Limit on time to wait for response (zero for no limit)
                                   // or time to delay.
} TEST_STEP;

int run_test_script(char *name, TEST_STEP *scr, int port);
