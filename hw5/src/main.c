#define _GNU_SOURCE
#include "pbx.h"
#include "server.h"
#include "debug.h"
#include "csapp.h"

static void terminate(int status);
void sighandler(int sig);

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    // Perform required initialization of the PBX module.
    debug("Initializing PBX...");
    pbx = pbx_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    // check arg counts and second arg valid
    if (argc != 3 || argv[1][0] != '-' || argv[1][1] != 'p' || argv[1][2] != '\0')
    {
        fprintf(stderr, "Usage: ./pbx -p <port>\n");
        terminate(EXIT_FAILURE);
    }

    // check port num
    int port = 0;
    char* ptr = argv[2];
    while (*ptr)
    {
        if (*ptr > '9' || *ptr < '0' || port > 65535)
        {
            fprintf(stderr, "Please provide a valid port number\n");
            terminate(EXIT_FAILURE);
        }
        port *= 10;
        port += *ptr - '0';
        ptr ++;
    }
    
    // install SIGHUP handler
    struct sigaction act;
    act.sa_handler = sighandler;
    if(sigaction(SIGHUP, &act, NULL)<0)
    {
        fprintf(stderr, "Failed to install a SIGHUP handler");
        terminate(EXIT_FAILURE);
    }

    // multi threading & socket connection
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    listenfd = Open_listenfd(argv[2]);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Pthread_create(&tid, NULL, (void*) pbx_client_service, connfdp);
    }
    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}

void sighandler(int sig)
{
    debug("Server turned down successfully...");
    terminate(EXIT_SUCCESS);
}