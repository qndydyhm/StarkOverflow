/*
 * Important: the Criterion tests in this file have to be run with -j1,
 * because they each start a separate server instance and if they are
 * run concurrently only one server will be able to bind the server port
 * and the others will fail.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <criterion/criterion.h>
#include <pthread.h>

#include "__test_includes.h"

static int server_pid;

static void wait_for_server() {
    int ret;
    int i = 0;
    do {
        fprintf(stderr, "Waiting for server to start (i = %d)\n", i);
	ret = system("netstat -an | grep 'LISTEN[ ]*$' | grep ':"SERVER_PORT_STR"'");
	sleep(SERVER_STARTUP_SLEEP);
    } while(++i < 30 && WEXITSTATUS(ret));
}

static void wait_for_no_server() {
    int ret;
    do {
	ret = system("netstat -an | grep 'LISTEN[ ]*$' | grep ':"SERVER_PORT_STR"'");
	if(WEXITSTATUS(ret) == 0) {
	    fprintf(stderr, "Waiting for server port to clear...\n");
	    system("killall -s KILL pbx > /dev/null");
	    sleep(1);
	} else {
	    break;
	}
    } while(1);
}

static void init() {
    server_pid = 0;
    wait_for_no_server();
    fprintf(stderr, "***Starting server...");
    if((server_pid = fork()) == 0) {
	execlp("bin/pbx", "pbx", "-p", SERVER_PORT_STR, NULL);
	fprintf(stderr, "Failed to exec server\n");
	abort();
    }
    fprintf(stderr, "pid = %d\n", server_pid);
    // Wait for server to start before returning
    wait_for_server();
}

static void fini(int chk) {
    int ret;
    cr_assert(server_pid != 0, "No server was started!\n");
    fprintf(stderr, "***Sending SIGHUP to server pid %d\n", server_pid);
    kill(server_pid, SIGHUP);
    sleep(SERVER_SHUTDOWN_SLEEP);
    kill(server_pid, SIGKILL);
    wait(&ret);
    fprintf(stderr, "***Server wait() returned = 0x%x\n", ret);
    if(chk) {
      if(WIFSIGNALED(ret))
	  cr_assert_fail("***Server terminated ungracefully with signal %d\n", WTERMSIG(ret));
      cr_assert_eq(WEXITSTATUS(ret), 0, "Server exit status was not 0");
    }
}

static void killall() {
    system("killall -s KILL pbx /usr/lib/valgrind/memcheck-amd64-linux > /dev/null 2>&1");
}


#define SUITE basecode_suite

#define TEST_NAME connect_disconnect_test
static TEST_STEP SCRIPT(TEST_NAME)[] = {
    // ID,  COMMAND,          ID_TO_DIAL,    RESPONSE,       TIMEOUT
    {   0,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     HND_MSEC },
    {   0,  TU_DISCONNECT_CMD, -1,           -1,             TEN_MSEC },
    {   -1, -1,                -1,           -1,             ZERO_SEC }
};

Test(SUITE, TEST_NAME, .init = init, .fini = killall, .timeout = 30)
{
    char *name = QUOTE(SUITE)"/"QUOTE(TEST_NAME);
    int ret = run_test_script(name, SCRIPT(TEST_NAME), SERVER_PORT);
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
    fini(0);
}
#undef TEST_NAME

#define TEST_NAME connect_disconnect2_test
static TEST_STEP SCRIPT(TEST_NAME)[] = {
    // ID,  COMMAND,          ID_TO_DIAL,    RESPONSE,       TIMEOUT
    {   0,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     HND_MSEC },
    {   1,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     HND_MSEC },
    {   0,  TU_DISCONNECT_CMD, -1,           -1,             TEN_MSEC },
    {   1,  TU_DISCONNECT_CMD, -1,           -1,             TEN_MSEC },
    {   -1, -1,                -1,           -1,             ZERO_SEC }
};

Test(SUITE, TEST_NAME, .init = init, .fini = killall, .timeout = 30) {
    char *name = QUOTE(SUITE)"/"QUOTE(TEST_NAME);
    int ret = run_test_script(name, SCRIPT(TEST_NAME), SERVER_PORT);
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
    fini(0);
}
#undef TEST_NAME

#define TEST_NAME pickup_hangup_test
static TEST_STEP SCRIPT(TEST_NAME)[] = {
    // ID,  COMMAND,          ID_TO_DIAL,    RESPONSE,       TIMEOUT
    {   0,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     HND_MSEC },
    {   0,  TU_PICKUP_CMD,     -1,           TU_DIAL_TONE,   TEN_MSEC },
    {   0,  TU_HANGUP_CMD,     -1,           TU_ON_HOOK,     TEN_MSEC },
    {   0,  TU_DISCONNECT_CMD, -1,           -1,             TEN_MSEC },
    {   -1, -1,                -1,           -1,             ZERO_SEC }
};

Test(SUITE, TEST_NAME, .init = init, .fini = killall, .timeout = 30) {
    char *name = QUOTE(SUITE)"/"QUOTE(TEST_NAME);
    int ret = run_test_script(name, SCRIPT(TEST_NAME), SERVER_PORT);
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
    fini(0);
}
#undef TEST_NAME

#define TEST_NAME dial_answer_test
static TEST_STEP SCRIPT(TEST_NAME)[] = {
    // ID,  COMMAND,          ID_TO_DIAL,    RESPONSE,       TIMEOUT
    {   0,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     HND_MSEC },
    {   1,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     HND_MSEC },
    {   0,  TU_PICKUP_CMD,     -1,           TU_DIAL_TONE,   TEN_MSEC },
    {   0,  TU_DIAL_CMD,        1,           TU_RING_BACK,   TEN_MSEC },
    {   1,  TU_PICKUP_CMD,     -1,           TU_CONNECTED,   TEN_MSEC },
    {   0,  TU_HANGUP_CMD,     -1,           TU_ON_HOOK,     FTY_MSEC },
    {   1,  TU_DISCONNECT_CMD, -1,           -1,             TEN_MSEC },
    {   0,  TU_DISCONNECT_CMD, -1,           -1,             TEN_MSEC },
    {   -1, -1,                -1,           -1,             ZERO_SEC }
};

Test(SUITE, TEST_NAME, .init = init, .fini = killall, .timeout = 30) {
    char *name = QUOTE(SUITE)"/"QUOTE(TEST_NAME);
    int ret = run_test_script(name, SCRIPT(TEST_NAME), SERVER_PORT);
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
    fini(0);
}
#undef TEST_NAME

#define TEST_NAME dial_disconnect_test
static TEST_STEP SCRIPT(TEST_NAME)[] = {
    // ID,  COMMAND,          ID_TO_DIAL,    RESPONSE,       TIMEOUT
    {   0,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     HND_MSEC },
    {   1,  TU_CONNECT_CMD,    -1,           TU_ON_HOOK,     HND_MSEC },
    {   0,  TU_PICKUP_CMD,     -1,           TU_DIAL_TONE,   TEN_MSEC },
    {   0,  TU_DIAL_CMD,        1,           TU_RING_BACK,   TEN_MSEC },
    {   1,  TU_PICKUP_CMD,     -1,           TU_CONNECTED,   TEN_MSEC },
    {   1,  TU_DISCONNECT_CMD, -1,           -1,             TEN_MSEC },
    {   0,  TU_HANGUP_CMD,     -1,           TU_ON_HOOK,     FTY_MSEC },
    {   0,  TU_DISCONNECT_CMD, -1,           -1,             TEN_MSEC },
    {   -1, -1,                -1,           -1,             ZERO_SEC }
};

Test(SUITE, TEST_NAME, .init = init, .fini = killall, .timeout = 30) {
    char *name = QUOTE(SUITE)"/"QUOTE(TEST_NAME);
    int ret = run_test_script(name, SCRIPT(TEST_NAME), SERVER_PORT);
    cr_assert_eq(ret, 0, "expected %d, was %d\n", 0, ret);
    fini(0);
}
#undef TEST_NAME
