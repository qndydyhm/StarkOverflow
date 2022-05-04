/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef PBX_H
#define PBX_H

#include <unistd.h>
#include <sys/select.h>

#include "tu.h"

/*
 * Structure types representing objects manipulated by the PBX module.
 *
 *   PBX: Represents the current state of a private branch exchange.
 *   TU: Represents the current state of a telephone unit.
 *
 * NOTE: These types are "opaque": the actual structure definitions are not
 * given here and it is not intended that a client of the PBX module should
 * know what they are.  The actual structure definitions are local to the
 * implementation of the PBX module and are not exported.
 */
typedef struct pbx PBX;

/*
 * Maximum number of extensions supported by a PBX.
 */
#define PBX_MAX_EXTENSIONS FD_SETSIZE

/*
 * End-of-line sequence used in communication with client.
 */
#define EOL "\r\n"

/*
 * Global variable that provides access to the PBX instance.
 */
extern PBX *pbx;

PBX *pbx_init();
void pbx_shutdown(PBX *pbx);
int pbx_register(PBX *pbx, TU *tu, int ext);
int pbx_unregister(PBX *pbx, TU *tu);
int pbx_dial(PBX *pbx, TU *tu, int ext);

#endif
