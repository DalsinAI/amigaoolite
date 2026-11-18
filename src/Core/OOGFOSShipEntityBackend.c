/* Game-task-only token -> Oolite Objective-C selector dispatch. GPL-2.0+. */
#include "OOGFOSShipEntityBackend.h"
#include "OOGFOSShipEntitySelectors.h"
#include <stdint.h>
_Static_assert(sizeof(LONG)==sizeof(int32_t),"GFOS script LONG must be 32-bit");
_Static_assert((int)OOGFOS_SHIP_BOUNTY==(int)OOGFOS_OOJS_BOUNTY,"bounty code mismatch");
_Static_assert((int)OOGFOS_SHIP_HOME_SYSTEM==(int)OOGFOS_OOJS_HOME_SYSTEM,"homeSystem code mismatch");
_Static_assert((int)OOGFOS_SHIP_MAX_ESCORTS==(int)OOGFOS_OOJS_MAX_ESCORTS,"escort code mismatch");
void OOGFOSShipEntityRegistryInit(struct OOGFOSShipEntityRegistry *r)
{
    if(r) OOGFOSShipTokensInit(&r->tokens);
}
ULONG OOGFOSShipEntityRegister(struct OOGFOSShipEntityRegistry *r,void *ship)
{
    return r && ship ? OOGFOSShipTokensAdd(&r->tokens,ship) : 0;
}
int OOGFOSShipEntityUnregister(struct OOGFOSShipEntityRegistry *r,ULONG token)
{
    return r ? OOGFOSShipTokensRemove(&r->tokens,token) : 0;
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
