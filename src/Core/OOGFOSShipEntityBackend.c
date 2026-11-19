/* Game-task-only token -> Oolite Objective-C selector dispatch. GPL-2.0+. */
#include "OOGFOSShipEntityBackend.h"
#include "OOGFOSShipEntitySelectors.h"
#include <stdint.h>
_Static_assert(sizeof(LONG)==sizeof(int32_t),"GFOS script LONG must be 32-bit");
_Static_assert((int)OOGFOS_SHIP_BOUNTY==(int)OOGFOS_OOJS_BOUNTY,"bounty code mismatch");
_Static_assert((int)OOGFOS_SHIP_HOME_SYSTEM==(int)OOGFOS_OOJS_HOME_SYSTEM,"homeSystem code mismatch");
_Static_assert((int)OOGFOS_SHIP_MAX_ESCORTS==(int)OOGFOS_OOJS_MAX_ESCORTS,"escort code mismatch");
_Static_assert((int)OOGFOS_SHIP_ENERGY==(int)OOGFOS_OOJS_ENERGY,"energy code mismatch");
_Static_assert((int)OOGFOS_SHIP_MAX_ENERGY==(int)OOGFOS_OOJS_MAX_ENERGY,"maxEnergy code mismatch");
_Static_assert((int)OOGFOS_SHIP_IS_VALID==(int)OOGFOS_OOJS_IS_VALID,"IS_VALID code mismatch");
_Static_assert((int)OOGFOS_SHIP_IS_SHIP==(int)OOGFOS_OOJS_IS_SHIP,"IS_SHIP code mismatch");
_Static_assert((int)OOGFOS_SHIP_IS_STATION==(int)OOGFOS_OOJS_IS_STATION,"IS_STATION code mismatch");
_Static_assert((int)OOGFOS_SHIP_IS_PLAYER==(int)OOGFOS_OOJS_IS_PLAYER,"IS_PLAYER code mismatch");
_Static_assert((int)OOGFOS_SHIP_IS_DOCK==(int)OOGFOS_OOJS_IS_DOCK,"IS_DOCK code mismatch");
_Static_assert((int)OOGFOS_SHIP_IS_IN_SPACE==(int)OOGFOS_OOJS_IS_IN_SPACE,"IS_IN_SPACE code mismatch");
_Static_assert((int)OOGFOS_SHIP_COLLISION_RADIUS==(int)OOGFOS_OOJS_COLLISION_RADIUS,"COLLISION_RADIUS code mismatch");
_Static_assert((int)OOGFOS_SHIP_DISTANCE_TRAVELLED==(int)OOGFOS_OOJS_DISTANCE_TRAVELLED,"DISTANCE_TRAVELLED code mismatch");
_Static_assert((int)OOGFOS_SHIP_MASS==(int)OOGFOS_OOJS_MASS,"MASS code mismatch");
_Static_assert((int)OOGFOS_SHIP_SPAWN_TIME==(int)OOGFOS_OOJS_SPAWN_TIME,"SPAWN_TIME code mismatch");
void OOGFOSShipEntityRegistryInit(struct OOGFOSShipEntityRegistry *r)
{
    if(r) OOGFOSShipTokensInit(&r->tokens);
}
ULONG OOGFOSShipEntityRegister(struct OOGFOSShipEntityRegistry *r,void *ship)
{
    void *retained;
    ULONG token;
    if (!r || !ship) return 0;
    /* Allocation and ObjC refcount operations happen on the game task.
     * A successful token owns one strong reference until unregister. */
    retained=OOGFOSShipObjCRetain(ship);
    if (!retained) return 0;
    token=OOGFOSShipTokensAdd(&r->tokens,retained);
    if (!token) OOGFOSShipObjCRelease(retained);
    return token;
}
int OOGFOSShipEntityUnregister(struct OOGFOSShipEntityRegistry *r,ULONG token)
{
    void *ship;
    if (!r) return 0;
    ship=OOGFOSShipTokensLookup(&r->tokens,token);
    if (!ship || !OOGFOSShipTokensRemove(&r->tokens,token)) return 0;
    /* Invalidate the token first. Any future JS job with this token is
     * rejected; release only after no owner-task access can resolve it. */
    OOGFOSShipObjCRelease(ship);
    return 1;
}
static int shipGet(void *context,ULONG token,unsigned property,LONG *out)
{
    struct OOGFOSShipEntityRegistry *r=(struct OOGFOSShipEntityRegistry *)context;
    void *ship=r ? OOGFOSShipTokensLookup(&r->tokens,token) : NULL;
    return ship && out ? OOGFOSShipObjCGetLong(ship,property,(int32_t *)out) : 0;
}
static int shipSet(void *context,ULONG token,unsigned property,LONG value)
{
    struct OOGFOSShipEntityRegistry *r=(struct OOGFOSShipEntityRegistry *)context;
    void *ship=r ? OOGFOSShipTokensLookup(&r->tokens,token) : NULL;
    return ship ? OOGFOSShipObjCSetLong(ship,property,(int32_t)value) : 0;
}
const struct OOGFOSGameRouteBackend *OOGFOSShipEntityGameBackend(void)
{
    static const struct OOGFOSGameRouteBackend backend={shipGet,shipSet};
    return &backend;
}
