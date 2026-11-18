/* Plain C scalar selector ABI; intentionally avoids Amiga exec/types.h because
 * it typedefs BOOL as 16-bit while GNU Objective-C runtime BOOL is 8-bit. */
#ifndef OOLITE_GFOS_SHIP_ENTITY_SELECTORS_H
#define OOLITE_GFOS_SHIP_ENTITY_SELECTORS_H
#include <stdint.h>
/* These numeric codes must match OOGFOS_SHIP_* in OOGFOSShipProperties.h. */
enum { OOGFOS_OOJS_BOUNTY=1, OOGFOS_OOJS_HOME_SYSTEM=2,
       OOGFOS_OOJS_MAX_ESCORTS=3 };
int OOGFOSShipObjCGetLong(void *ship, unsigned property, int32_t *out);
int OOGFOSShipObjCSetLong(void *ship, unsigned property, int32_t value);
#endif
