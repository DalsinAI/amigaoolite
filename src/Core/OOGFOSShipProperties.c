#include "OOGFOSShipProperties.h"
#include <proto/javascript.h>

/* Actual OOJSShip numeric property names, with explicit, read/write native
 * callbacks. No direct ShipEntity access from JavaScriptEngine's task. */
static JSValue getNumber(JSTask task, APTR self, unsigned property)
{
    struct OOGFOSShipHandle *h=(struct OOGFOSShipHandle *)self;
    LONG value=0;
    if (!h || !h->valid || !h->backend || !h->backend->getLong ||
        !h->backend->getLong(h->ship, property, &value))
        return JS_Throw(task,(CONST_STRPTR)"Oolite ship unavailable");
    return JS_NewLong(task,value);
}
static LONG setNumber(JSTask task, APTR self, JSValue input, unsigned property)
{
    struct OOGFOSShipHandle *h=(struct OOGFOSShipHandle *)self;
    LONG value=0;
    if (!h || !h->valid || !h->backend || !h->backend->setLong ||
        JS_ToLong(task,input,&value) != JSERR_OK ||
        !h->backend->setLong(h->ship, property, value)) {
        JS_Throw(task,(CONST_STRPTR)"Oolite ship write rejected");
        return JSERR_EXCEPTION;
    }
    return JSERR_OK;
}
#define SHIP_ACCESSOR(name, property) \
static JSValue get_##name(JSTask task, APTR self, APTR data) { \
    (void)data; return getNumber(task,self,property); } \
static LONG set_##name(JSTask task, APTR self, JSValue input, APTR data) { \
    (void)data; return setNumber(task,self,input,property); }
SHIP_ACCESSOR(bounty, OOGFOS_SHIP_BOUNTY)
SHIP_ACCESSOR(homeSystem, OOGFOS_SHIP_HOME_SYSTEM)
SHIP_ACCESSOR(maxEscorts, OOGFOS_SHIP_MAX_ESCORTS)
static const struct JSHostSlot shipSlots[]={
    { (CONST_STRPTR)"bounty", JSS_ACCESSOR,0,(APTR)get_bounty,(APTR)set_bounty,0,0 },
    { (CONST_STRPTR)"homeSystem", JSS_ACCESSOR,0,(APTR)get_homeSystem,(APTR)set_homeSystem,0,0 },
    { (CONST_STRPTR)"maxEscorts", JSS_ACCESSOR,0,(APTR)get_maxEscorts,(APTR)set_maxEscorts,0,0 },
    {NULL,0,0,NULL,NULL,0,0}
};
const struct JSHostClass *OOGFOSShipClass(void)
{
    static const struct JSHostClass cls={ (CONST_STRPTR)"OoliteShipProperties",NULL,shipSlots,NULL,NULL };
    return &cls;
}
void OOGFOSShipInvalidate(struct OOGFOSShipHandle *h)
{
    if (h) h->valid=0;
}
