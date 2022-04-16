/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef TU_H
#define TU_H

/*
 * Structure types representing objects manipulated by the TU module.
 *
 *   TU: Represents the current state of a telephone unit.
 *
 * NOTE: These types are "opaque": the actual structure definitions are not
 * given here and it is not intended that a client of the TU module should
 * know what they are.  The actual structure definitions are local to the
 * implementation of the TU module and are not exported.
 */
typedef struct tu TU;

/*
 * The possible states that a TU can be in.
 */
typedef enum tu_state {
    TU_ON_HOOK, TU_RINGING, TU_DIAL_TONE, TU_RING_BACK, TU_BUSY_SIGNAL,
    TU_CONNECTED, TU_ERROR
} TU_STATE;

/*
 * Array that specifies a printable name for each of the TU states.
 * These names should be used when sending state-change notifications to
 * the underlying network clients.  They may also be used for debugging
 * purposes.
 */
extern char *tu_state_names[];

TU *tu_init(int fd);
void tu_ref(TU *tu, char *reason);
void tu_unref(TU *tu, char *reason);
int tu_fileno(TU *tu);
int tu_extension(TU *tu);
int tu_set_extension(TU *tu, int ext);
int tu_pickup(TU *tu);
int tu_hangup(TU *tu);
int tu_dial(TU *tu, TU *target);
int tu_chat(TU *tu, char *msg);

#endif
