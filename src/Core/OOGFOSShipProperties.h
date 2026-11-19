/* GFOS native ship property seam, for original Oolite OOJSShip's numeric properties.
 * GPL-2.0-or-later. Not a ShipEntity replacement.
 */
#ifndef OOLITE_GFOS_SHIP_PROPERTIES_H
#define OOLITE_GFOS_SHIP_PROPERTIES_H
#include <libraries/javascript.h>

/* Application-side ShipEntity bridge must supply thread-safe lifecycle and
 * accessors. These functions are called from JavaScriptEngine's task, NOT
 * the Oolite simulation thread. Do not directly touch ShipEntity here. */
struct OOGFOSShipBackend {
    int (*getLong)(void *ship, unsigned property, LONG *value);
    int (*setLong)(void *ship, unsigned property, LONG value);
};
enum { OOGFOS_SHIP_BOUNTY=1, OOGFOS_SHIP_HOME_SYSTEM=2,
       OOGFOS_SHIP_MAX_ESCORTS=3, OOGFOS_SHIP_ENERGY=4,
       OOGFOS_SHIP_MAX_ENERGY=5 };
struct OOGFOSShipHandle {
    void *ship;
    const struct OOGFOSShipBackend *backend;
    volatile LONG valid;
};
const struct JSHostClass *OOGFOSShipClass(void);
void OOGFOSShipInvalidate(struct OOGFOSShipHandle *handle);
#endif
