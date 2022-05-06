/*
 * PBX: simulates a Private Branch Exchange.
 */
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>

#include "pbx.h"
#include "debug.h"

/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */
#if 1
typedef struct pbx
{
    pthread_mutex_t mutex;
    TU *tus[PBX_MAX_EXTENSIONS];
} PBX;

void pbx_unregister_helper(TU *tu);

PBX *pbx_init() {
    PBX *pbx = malloc(sizeof(PBX));
    if (!pbx)
    {
        debug("ERROR: Fail to alloc memeory for pbx");
        return NULL;
    }
    // init pbx
    for (size_t i = 0; i < PBX_MAX_EXTENSIONS; i++)
        pbx->tus[i] = NULL;
    
    pthread_mutex_init(&pbx->mutex, NULL);
    return pbx;
}
#endif

/*
 * Shut down a pbx, shutting down all network connections, waiting for all server
 * threads to terminate, and freeing all associated resources.
 * If there are any registered extensions, the associated network connections are
 * shut down, which will cause the server threads to terminate.
 * Once all the server threads have terminated, any remaining resources associated
 * with the PBX are freed.  The PBX object itself is freed, and should not be used again.
 *
 * @param pbx  The PBX to be shut down.
 */
#if 1
void pbx_shutdown(PBX *pbx) {
    if (!pbx)
    {
        debug("ERROR: pbx does not extst");
        return;
    }
    pthread_mutex_lock(&pbx->mutex);
    for (size_t i = 0; i < PBX_MAX_EXTENSIONS; i++)
    {
        if (pbx->tus[i])
        {
            pbx_unregister_helper(pbx->tus[i]);
            pbx->tus[i] = NULL;
        }
    }
    pthread_mutex_unlock(&pbx->mutex);
    pthread_mutex_destroy(&pbx->mutex);
    free(pbx);
}
#endif

/*
 * Register a telephone unit with a PBX at a specified extension number.
 * This amounts to "plugging a telephone unit into the PBX".
 * The TU is initialized to the TU_ON_HOOK state.
 * The reference count of the TU is increased and the PBX retains this reference
 *for as long as the TU remains registered.
 * A notification of the assigned extension number is sent to the underlying network
 * client.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU to be registered.
 * @param ext  The extension number on which the TU is to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */
#if 1
int pbx_register(PBX *pbx, TU *tu, int ext) {
    if (!pbx || !tu)
    {
        debug("ERROR: pbx or tu does not exist");
        return -1;
    }
    
    if (ext > PBX_MAX_EXTENSIONS + 1 || ext < 1)
    {
        debug("ERROR: ext is out of range");
        return -1;
    }
    
    pthread_mutex_lock(&pbx->mutex);
    if (pbx->tus[ext])
    {
        debug("ERROR: ext already exist");
        pthread_mutex_unlock(&pbx->mutex);
        return -1;
    }
    tu_ref(tu, "Register tu to pbx");
    tu_set_extension(tu, ext);
    pbx->tus[ext] = tu;
    pthread_mutex_unlock(&pbx->mutex);
    return 0;
}
#endif

/*
 * Unregister a TU from a PBX.
 * This amounts to "unplugging a telephone unit from the PBX".
 * The TU is disassociated from its extension number.
 * Then a hangup operation is performed on the TU to cancel any
 * call that might be in progress.
 * Finally, the reference held by the PBX to the TU is released.
 *
 * @param pbx  The PBX.
 * @param tu  The TU to be unregistered.
 * @return 0 if unregistration succeeds, otherwise -1.
 */
#if 1
int pbx_unregister(PBX *pbx, TU *tu) {
    if (!pbx || !tu)
    {
        debug("ERROR: pbx or tu does not exist");
        return -1;
    }
    int ext = tu_extension(tu);
    if (ext > PBX_MAX_EXTENSIONS + 1 || ext < 1)
    {
        debug("ERROR: ext is out of range");
        return -1;
    }
    pthread_mutex_lock(&pbx->mutex);
    if (!pbx->tus[ext])
    {
        debug("ERROR: ext does not exist");
        pthread_mutex_unlock(&pbx->mutex);
        return -1;
    }
    pbx_unregister_helper(tu);
    pbx->tus[ext] = NULL;
    pthread_mutex_unlock(&pbx->mutex);
    return 0;
}
#endif

void pbx_unregister_helper(TU *tu) {
    tu_hangup(tu);
    shutdown(tu_fileno(tu), SHUT_RD);
    tu_unref(tu, "Unregister tu from pbx");
}

/*
 * Use the PBX to initiate a call from a specified TU to a specified extension.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU that is initiating the call.
 * @param ext  The extension number to be called.
 * @return 0 if dialing succeeds, otherwise -1.
 */
#if 1
int pbx_dial(PBX *pbx, TU *tu, int ext) {
    if (!pbx || !tu)
    {
        debug("ERROR: pbx or tu does not exist");
        return -1;
    }
    if (ext > PBX_MAX_EXTENSIONS || ext < 0)
    {
        debug("ERROR: ext is out of range");
        return -1;
    }
    pthread_mutex_lock(&pbx->mutex);
    int status = tu_dial(tu, pbx->tus[ext]);
    pthread_mutex_unlock(&pbx->mutex);
    return status;
}
#endif
