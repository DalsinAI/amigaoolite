/* C-only Oolite ShipEntity registry bridge. No Objective-C or Foundation
 * types cross the GFOS Exec IPC boundary. GPL-2.0-or-later. */
#ifndef OOLITE_GFOS_SHIP_ENTITY_BACKEND_H
#define OOLITE_GFOS_SHIP_ENTITY_BACKEND_H
#include "OOGFOSShipTokens.h"
#include "OOGFOSGameDispatch.h"
struct OOGFOSShipEntityRegistry {
    struct OOGFOSShipTokenTable tokens;
};
void OOGFOSShipEntityRegistryInit(struct OOGFOSShipEntityRegistry *registry);
ULONG OOGFOSShipEntityRegister(struct OOGFOSShipEntityRegistry *registry,void *ship);
int OOGFOSShipEntityUnregister(struct OOGFOSShipEntityRegistry *registry,ULONG token);
const struct OOGFOSGameRouteBackend *OOGFOSShipEntityGameBackend(void);
#endif
