/*
 * TU: simulates a "telephone unit", which interfaces a client with the PBX.
 */
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "pbx.h"
#include "debug.h"

/*
 * Initialize a TU
 *
 * @param fd  The file descriptor of the underlying network connection.
 * @return  The TU, newly initialized and in the TU_ON_HOOK state, if initialization
 * was successful, otherwise NULL.
 */
#if 1
typedef struct tu
{
    int fd;
    FILE *f;
    int ext;
    int ref;
    TU *chat_TU;
    int chat_locked;
    TU_STATE state;
    pthread_mutex_t mutex;
} TU;

int tu_notify(TU *tu);
int tu_lock(TU *tu);
int tu_unlock(TU *tu);
void tu_destroy(TU* tu);

TU *tu_init(int fd) {
    TU *tu;

    tu = malloc(sizeof(TU));
    if ( !tu ) {
        debug("ERROR: fail to alloc memory for tu");
        return NULL;
    }

    tu->fd = fd;
    tu->f = fdopen(fd, "w");
    tu->ext = 0;
    tu->ref = 1;
    tu->chat_TU = NULL;
    tu->chat_locked = 0;
    tu->state = TU_ON_HOOK;
    pthread_mutex_init(&tu->mutex, NULL);
    return tu;
}
#endif

/*
 * Increment the reference count on a TU.
 *
 * @param tu  The TU whose reference count is to be incremented
 * @param reason  A string describing the reason why the count is being incremented
 * (for debugging purposes).
 */
#if 1
void tu_ref(TU *tu, char *reason) {
    if (!tu)
    {
        debug("ERROR: tu does not exist");
        return;
    }
    pthread_mutex_lock(&tu->mutex);
    // debug(reason);
    tu->ref ++;
    pthread_mutex_unlock(&tu->mutex);
}
#endif

/*
 * Decrement the reference count on a TU, freeing it if the count becomes 0.
 *
 * @param tu  The TU whose reference count is to be decremented
 * @param reason  A string describing the reason why the count is being decremented
 * (for debugging purposes).
 */
#if 1
void tu_unref(TU *tu, char *reason) {
    if (!tu)
    {
        debug("ERROR: tu does not exist");
        return;
    }
    pthread_mutex_lock(&tu->mutex);
    // debug(reason);
    tu->ref --;
    if (tu->ref == 0)
        tu_destroy(tu);
    else
        pthread_mutex_unlock(&tu->mutex);
}
#endif

/*
 * Get the file descriptor for the network connection underlying a TU.
 * This file descriptor should only be used by a server to read input from
 * the connection.  Output to the connection must only be performed within
 * the PBX functions.
 *
 * @param tu
 * @return the underlying file descriptor, if any, otherwise -1.
 */
#if 1
int tu_fileno(TU *tu) {
    if (!tu)
    {
        debug("ERROR: tu does not exist");
        return -1;
    }
    pthread_mutex_lock(&tu->mutex);
    int fd = tu->fd;
    pthread_mutex_unlock(&tu->mutex);
    return fd;
}
#endif

/*
 * Get the extension number for a TU.
 * This extension number is assigned by the PBX when a TU is registered
 * and it is used to identify a particular TU in calls to tu_dial().
 * The value returned might be the same as the value returned by tu_fileno(),
 * but is not necessarily so.
 *
 * @param tu
 * @return the extension number, if any, otherwise -1.
 */
#if 1
int tu_extension(TU *tu) {
    if (!tu)
    {
        debug("ERROR: tu does not exist");
        return -1;
    }
    pthread_mutex_lock(&tu->mutex);
    int ext = tu->ext;
    pthread_mutex_unlock(&tu->mutex);
    return ext;
}
#endif

/*
 * Set the extension number for a TU.
 * A notification is set to the client of the TU.
 * This function should be called at most once one any particular TU.
 *
 * @param tu  The TU whose extension is being set.
 */
#if 1
int tu_set_extension(TU *tu, int ext) {
    if (!tu)
    {
        debug("ERROR: tu does not exist");
        return -1;
    }
    pthread_mutex_lock(&tu->mutex);
    if (tu->ext)
    {
        debug("ERROR: extention has already been set");
        pthread_mutex_unlock(&tu->mutex);
        return -1;
    }
    tu->ext = ext;
    int status = tu_notify(tu);
    pthread_mutex_unlock(&tu->mutex);
    return status;
}
#endif

/*
 * Initiate a call from a specified originating TU to a specified target TU.
 *   If the originating TU is not in the TU_DIAL_TONE state, then there is no effect.
 *   If the target TU is the same as the originating TU, then the TU transitions
 *     to the TU_BUSY_SIGNAL state.
 *   If the target TU already has a peer, or the target TU is not in the TU_ON_HOOK
 *     state, then the originating TU transitions to the TU_BUSY_SIGNAL state.
 *   Otherwise, the originating TU and the target TU are recorded as peers of each other
 *     (this causes the reference count of each of them to be incremented),
 *     the target TU transitions to the TU_RINGING state, and the originating TU
 *     transitions to the TU_RING_BACK state.
 *
 * In all cases, a notification of the resulting state of the originating TU is sent to
 * to the associated network client.  If the target TU has changed state, then its client
 * is also notified of its new state.
 *
 * If the caller of this function was unable to determine a target TU to be called,
 * it will pass NULL as the target TU.  In this case, the originating TU will transition
 * to the TU_ERROR state if it was in the TU_DIAL_TONE state, and there will be no
 * effect otherwise.  This situation is handled here, rather than in the caller,
 * because here we have knowledge of the current TU state and we do not want to introduce
 * the possibility of transitions to a TU_ERROR state from arbitrary other states,
 * especially in states where there could be a peer TU that would have to be dealt with.
 *
 * @param tu  The originating TU.
 * @param target  The target TU, or NULL if the caller of this function was unable to
 * identify a TU to be dialed.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
#if 1
int tu_dial(TU *tu, TU *target) {
    if (!tu)
    {
        debug("ERROR: tu or target does not exist");
        return -1;
    }
    pthread_mutex_lock(&tu->mutex);
    if (tu->chat_TU)
    {
        debug("ERROR: Already in chat");
        tu_notify(tu);
        pthread_mutex_unlock(&tu->mutex);
        return -1;
    }
    if (tu->state != TU_DIAL_TONE)
    {
        debug("ERROR: not in dial state");
        tu_notify(tu);
        pthread_mutex_unlock(&tu->mutex);
        return -1;
    }
    if (target)
    {
        if (tu == target)
        {
            debug("ERROR: calling it self");
            tu->state = TU_BUSY_SIGNAL;
            int status = tu_notify(tu);
            pthread_mutex_unlock(&tu->mutex);
            return status;
        }
        if (tu->ext < target->ext)
        {
            pthread_mutex_unlock(&tu->mutex);
            pthread_mutex_lock(&tu->mutex);
            pthread_mutex_lock(&target->mutex);
        }
        else {
            pthread_mutex_unlock(&tu->mutex);
            pthread_mutex_lock(&target->mutex);
            pthread_mutex_lock(&tu->mutex);
        }
        if (target->chat_TU)
        {
            debug("ERROR: target is already in chat");
            tu->state = TU_BUSY_SIGNAL;
            int status = tu_notify(tu);
            pthread_mutex_unlock(&tu->mutex);
            pthread_mutex_unlock(&target->mutex);
            return status;
        }
        if (target->state != TU_ON_HOOK)
        {
            debug("ERROR: target not in on hook state");
            tu->state = TU_BUSY_SIGNAL;
            int status = tu_notify(tu);
            pthread_mutex_unlock(&tu->mutex);
            pthread_mutex_unlock(&target->mutex);
            return status;
        }
        debug("dialing");
        tu->ref++;
        target->ref++;
        tu->chat_TU = target;
        target->chat_TU = tu;
        tu->state = TU_RING_BACK;
        target->state = TU_RINGING;
        int status = 0;
        if (tu_notify(tu))
            status = -1;
        if (tu_notify(target))
            status = -1;
        pthread_mutex_unlock(&tu->mutex);
        pthread_mutex_unlock(&target->mutex);
        return status;
    }
    else {
        debug("ERROR: cannot find target");
        tu->state = TU_ERROR;
        int status = tu_notify(tu);
        pthread_mutex_unlock(&tu->mutex);
        return status;
    }
}
#endif

/*
 * Take a TU receiver off-hook (i.e. pick up the handset).
 *   If the TU is in neither the TU_ON_HOOK state nor the TU_RINGING state,
 *     then there is no effect.
 *   If the TU is in the TU_ON_HOOK state, it goes to the TU_DIAL_TONE state.
 *   If the TU was in the TU_RINGING state, it goes to the TU_CONNECTED state,
 *     reflecting an answered call.  In this case, the calling TU simultaneously
 *     also transitions to the TU_CONNECTED state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The TU that is to be picked up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
#if 1
int tu_pickup(TU *tu) {
    if (!tu)
    {
        debug("ERROR: tu or target does not exist");
        return -1;
    }
    tu_lock(tu);
    if (tu->state == TU_ON_HOOK)
    {
        debug("TU is on hook");
        tu->state = TU_DIAL_TONE;
        int status = tu_notify(tu);
        tu_unlock(tu);
        return status;
    }
    if (tu->state == TU_RINGING)
    {
        if (tu->chat_TU)
        {
            if (tu->chat_TU->state == TU_RING_BACK && tu == tu->chat_TU->chat_TU)
            {
                debug("connecting");
                tu->state = TU_CONNECTED;
                tu->chat_TU->state = TU_CONNECTED;
                int status = 0;
                if (tu_notify(tu))
                    status = -1;
                if (tu_notify(tu->chat_TU))
                    status = -1;
                tu_unlock(tu);
                return status;
            }
            debug("ERROR: ringing and has chat_TU but chat_TU is not in ring back or tu's chat_TU's chat_TU is not tu");
            tu_notify(tu);
            tu_unlock(tu);
            return -1;
        }
        else {
            debug("ERROR: ringing but no chat_TU");
            tu_unlock(tu);
            return -1;
        }
    }
    debug("already picked up");
    tu_notify(tu);
    tu_unlock(tu);
    return -1;
}
#endif

/*
 * Hang up a TU (i.e. replace the handset on the switchhook).
 *
 *   If the TU is in the TU_CONNECTED or TU_RINGING state, then it goes to the
 *     TU_ON_HOOK state.  In addition, in this case the peer TU (the one to which
 *     the call is currently connected) simultaneously transitions to the TU_DIAL_TONE
 *     state.
 *   If the TU was in the TU_RING_BACK state, then it goes to the TU_ON_HOOK state.
 *     In addition, in this case the calling TU (which is in the TU_RINGING state)
 *     simultaneously transitions to the TU_ON_HOOK state.
 *   If the TU was in the TU_DIAL_TONE, TU_BUSY_SIGNAL, or TU_ERROR state,
 *     then it goes to the TU_ON_HOOK state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The tu that is to be hung up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
#if 1
int tu_hangup(TU *tu) {
    if (!tu)
    {
        debug("ERROR: tu or target does not exist");
        return -1;
    }
    tu_lock(tu);

    if (tu->state == TU_ON_HOOK)
    {
        debug("already on hook");
        int status = tu_notify(tu);
        tu_unlock(tu);
        return status;
    }
    if (tu->state == TU_CONNECTED || tu->state == TU_RINGING)
    {
        if (tu->chat_TU)
        {
            debug("disconnecting peer");
            tu->state = TU_ON_HOOK;
            tu->chat_TU->state = TU_DIAL_TONE;
            int status = 0;
            if (tu_notify(tu))
                status = -1;
            if (tu_notify(tu->chat_TU))
                status = -1;
            
            tu->chat_TU->chat_TU = NULL;
            tu->chat_TU->ref --;
            if (tu->chat_TU->ref <= 0)
                tu_destroy(tu->chat_TU);
            else
                pthread_mutex_unlock(&tu->chat_TU->mutex);
            
            tu->chat_TU = NULL;
            tu->ref --;
            tu->chat_locked = 0;
            if (tu->ref == 0)
                tu_destroy(tu);
            else
                pthread_mutex_unlock(&tu->mutex);
            return status;
        }
        else {
            debug("ERROR: connecting or ringing but no peer");
            tu->state = TU_ON_HOOK;
            tu_notify(tu);
            tu_unlock(tu);
            return -1;
        }
    }
    if (tu->state == TU_RING_BACK)
    {
        if (tu->chat_TU)
        {
            debug("disconnecting peer");
            tu->state = TU_ON_HOOK;
            tu->chat_TU->state = TU_ON_HOOK;
            int status = 0;
            if (tu_notify(tu))
                status = -1;
            if (tu_notify(tu->chat_TU))
                status = -1;

            tu->chat_TU->chat_TU = NULL;
            tu->chat_TU->ref --;
            if (tu->chat_TU->ref <= 0)
                tu_destroy(tu->chat_TU);
            else
                pthread_mutex_unlock(&tu->chat_TU->mutex);
            
            tu->chat_TU = NULL;
            tu->ref --;
            tu->chat_locked = 0;
            if (tu->ref == 0)
                tu_destroy(tu);
            else
                pthread_mutex_unlock(&tu->mutex);
            return status;
        }
        else {
            debug("ERROR: ringing back but no peer");
            tu->state = TU_ON_HOOK;
            tu_notify(tu);
            tu_unlock(tu);
            return -1;
        }
    }
    if (tu->chat_TU) {
        debug("ERROR: no chat_TU in these states");
        tu->state = TU_ON_HOOK;
        tu_notify(tu);
        tu_unlock(tu);
        return -1;
    }
    tu->state = TU_ON_HOOK;
    int status = tu_notify(tu);
    tu_unlock(tu);
    return status;
}
#endif

/*
 * "Chat" over a connection.
 *
 * If the state of the TU is not TU_CONNECTED, then nothing is sent and -1 is returned.
 * Otherwise, the specified message is sent via the network connection to the peer TU.
 * In all cases, the states of the TUs are left unchanged and a notification containing
 * the current state is sent to the TU sending the chat.
 *
 * @param tu  The tu sending the chat.
 * @param msg  The message to be sent.
 * @return 0  If the chat was successfully sent, -1 if there is no call in progress
 * or some other error occurs.
 */
#if 1
int tu_chat(TU *tu, char *msg) {
    pthread_mutex_lock(&tu->mutex);
    if (tu->state == TU_CONNECTED)
    {
        TU *target = tu->chat_TU;
        if (target)
        {
            if (tu->ext < target->ext)
            {
                pthread_mutex_unlock(&tu->mutex);
                pthread_mutex_lock(&tu->mutex);
                pthread_mutex_lock(&target->mutex);
            }
            else {
                pthread_mutex_unlock(&tu->mutex);
                pthread_mutex_lock(&target->mutex);
                pthread_mutex_lock(&tu->mutex);
            }
            if (tu->chat_TU == target && target->chat_TU == tu 
                && tu->state == TU_CONNECTED && target->state == TU_CONNECTED)
            {
                debug("sending msg");
                int status = 0;
                if (fprintf(target->f, "CHAT %s\r\n", msg) < 0 || fflush(target->f) < 0)
                    status = -1;
                if (tu_notify(tu) < 0)
                    status = -1;
                pthread_mutex_unlock(&target->mutex);
                pthread_mutex_unlock(&tu->mutex);
                return status;
            }
            else
            {
                debug("ERROR: target changed or chat_TU do not match");
                tu_notify(tu);
                pthread_mutex_unlock(&target->mutex);
                pthread_mutex_unlock(&tu->mutex);
                return -1;
            }  
        }
        else {
            debug("ERROR: connected but no chat_TU");
            tu_notify(tu);
            pthread_mutex_unlock(&tu->mutex);
            return -1;
        }
    }
    else {
        debug("ERROR: not in connected state");
        tu_notify(tu);
        pthread_mutex_unlock(&tu->mutex);
        return -1;
    }
}
#endif

int tu_notify(TU *tu) {
    int status = 0;

    if (fputs(tu_state_names[tu->state], tu->f) < 0)
        status = -1;
    if (tu->state == TU_ON_HOOK)
        if (fprintf(tu->f, " %d", tu->fd) < 0)
            status = -1;
    if (tu->state == TU_CONNECTED)
        if (fprintf(tu->f, " %d", tu->chat_TU->fd) < 0)
            status = -1;
    if (fprintf(tu->f, "\r\n") < 0 || fflush(tu->f) < 0)
        status = -1;
    if (status == -1)
        debug("ERROR: fail to print message to client");
    return status;
}

int tu_lock(TU *tu) {
    while (1) {
        pthread_mutex_lock(&tu->mutex);
        TU *tmp = tu->chat_TU;
        if (!tu->chat_TU)
            return 0;
        if (tu->ext < tmp->ext)
        {
            pthread_mutex_unlock(&tu->mutex);
            pthread_mutex_lock(&tu->mutex);
            pthread_mutex_lock(&tmp->mutex);
            tu->chat_locked = 1;
        }
        else {
            pthread_mutex_unlock(&tu->mutex);
            pthread_mutex_lock(&tmp->mutex);
            pthread_mutex_lock(&tu->mutex);
            tu->chat_locked = 1;
        }
        if (tu->chat_TU == tmp)
            return 0;
        
        pthread_mutex_unlock(&tmp->mutex);
        pthread_mutex_unlock(&tu->mutex);
    }
}

int tu_unlock(TU *tu) {
    if (tu->chat_locked)
    {
        pthread_mutex_unlock(&tu->chat_TU->mutex);
        tu->chat_locked = 0;
    }
    pthread_mutex_unlock(&tu->mutex);
    return 0;
}

void tu_destroy(TU* tu) {
    pthread_mutex_unlock(&tu->mutex);
    pthread_mutex_destroy(&tu->mutex);
    free(tu);
}