/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "debug.h"
#include "pbx.h"
#include "server.h"
#include "tu.h"
/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */
#if 1
int pbx_get_next_command(FILE* f, char** str);
int pbx_run_command(TU *tu, char* cmd);

void *pbx_client_service(void *arg) {
    // detach the thread
    pthread_detach(pthread_self());

    // free file descriptor
    if (!arg)
        return NULL;
    int fd = *(int*)arg;
    free(arg);
    
    // init TU
    TU* tu = tu_init(fd);
    if (!tu)
        return NULL;

    // open socket
    FILE *f = fdopen(fd, "r");

    // register tu
    if (pbx_register(pbx, tu, fd))
    {
        // not success
        fclose(f);
        tu_unref(tu, "fail to register a TU");
        return NULL;
    }
    else {
        // success
        char* command = NULL;
        while (1)
        {
            command = NULL;
            if (pbx_get_next_command(f, &command))
                break;
            debug("Got command: %s", command);
            pbx_run_command(tu, command);
            free(command);
        }
        // no more command
        fclose(f);
        tu_unref(tu, "unregister tu");
        pbx_unregister(pbx, tu);
        return NULL;
    }
}

/** 
 * try to get the next command from f and assign to str
 * return 0 if next command exist
 * return -1 if next command does not exist or have length 0
 * return -2 if fail to alloc memory
 * return -3 if fail to realloc memory
 */
int pbx_get_next_command(FILE* f, char** str) {
    int size = 8, index = 0;
    char *cmd = malloc(sizeof(char) * size);
    if (!cmd)
    // fail to alloc memory
        return -2;
    
    char c;
    while ((c = fgetc(f)) != EOF)
    {
        if (index == size - 1)
        {
            size *= 2;
            cmd = realloc(cmd, sizeof(char)*size);
            if (!cmd)
            // fail to realloc memory
                return -3;
        }

        cmd[index++] = c;
        if (c == '\r' || c == '\n')
            cmd[index-1] = '\0';

        if (c == '\n')
            break;
    }

    // command is empty
    if (index == 0) {
        free(cmd);
        return -1;
    }

    cmd[index--] = '\0';
    *str = cmd;
    return 0;
}

int pbx_run_command(TU *tu, char* cmd) {
    if (!strcmp(tu_command_names[TU_PICKUP_CMD], cmd))
        return tu_pickup(tu);
    if (!strcmp(tu_command_names[TU_HANGUP_CMD], cmd))
        return tu_hangup(tu);
    if (cmd == strstr(cmd, tu_command_names[TU_DIAL_CMD]))
    {
        char* ptr = &cmd[strlen(tu_command_names[TU_DIAL_CMD])];
        while (*ptr == ' ')
            ptr ++;
        if (!*ptr)
            return -1;
        return pbx_dial(pbx, tu, atoi(ptr));
    }
    if (cmd == strstr(cmd, tu_command_names[TU_CHAT_CMD]))
    {
        char* ptr = &cmd[strlen(tu_command_names[TU_CHAT_CMD])];
        while (*ptr == ' ')
            ptr ++;
        return tu_chat(tu, ptr);
    }
    return -1;
}
#endif
