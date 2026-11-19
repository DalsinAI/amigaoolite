#include "OOGFOSShipProperties.h"
#include <proto/javascript.h>
#include <math.h>
#include <float.h>
#include <string.h>
_Static_assert(sizeof(float)==sizeof(LONG),"Oolite GLfloat scalar transport requires 32-bit float");

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
/* LONG on the Exec route carries the exact 32-bit IEEE-754 float bits,
 * never an integer approximation. The Objective-C Entity selectors execute
 * only on the game task; JavaScriptEngine may not touch ShipEntity directly. */
static JSValue getFloat(JSTask task, APTR self, unsigned property)
{
    struct OOGFOSShipHandle *h=(struct OOGFOSShipHandle *)self;
    LONG raw;
    float value;
    DOUBLE converted;
    if(!h || !h->valid || !h->backend || !h->backend->getLong ||
       !h->backend->getLong(h->ship,property,&raw))
        return JS_Throw(task,(CONST_STRPTR)"Oolite entity unavailable");
    memcpy(&value,&raw,sizeof(value));
    converted=(DOUBLE)value;
    return JS_NewDouble(task,&converted);
}
static LONG setFloat(JSTask task, APTR self, JSValue input, unsigned property)
{
    struct OOGFOSShipHandle *h=(struct OOGFOSShipHandle *)self;
    DOUBLE converted;
    float value;
    LONG raw;
    if(!h || !h->valid || !h->backend || !h->backend->setLong ||
       JS_ToDouble(task,input,&converted)!=JSERR_OK ||
       !isfinite(converted) || converted>FLT_MAX || converted<-FLT_MAX) {
        JS_Throw(task,(CONST_STRPTR)"Oolite invalid entity energy");
        return JSERR_EXCEPTION;
    }
    value=(float)converted;
    memcpy(&raw,&value,sizeof(raw));
    if(!h->backend->setLong(h->ship,property,raw)){
        JS_Throw(task,(CONST_STRPTR)"Oolite entity energy write rejected");
        return JSERR_EXCEPTION;
    }
    return JSERR_OK;
}
/* Original OOJSEntity's classification flags are JS Boolean values,
 * never numerical 0/1. A DEAD status still has a retained ObjC owner,
 * but the script-visible isValid flag correctly becomes false. */
static JSValue getBoolean(JSTask task, APTR self, unsigned property)
{
    struct OOGFOSShipHandle *h=(struct OOGFOSShipHandle *)self;
    LONG raw=0;
    if(!h || !h->valid || !h->backend || !h->backend->getLong ||
       !h->backend->getLong(h->ship,property,&raw))
        return JS_Throw(task,(CONST_STRPTR)"Oolite entity classification unavailable");
    return JS_NewBool(task,raw!=0);
}
#define SHIP_BOOLEAN_GETTER(name,property) \
static JSValue get_##name(JSTask task,APTR self,APTR data) { \
    (void)data;return getBoolean(task,self,property); }
SHIP_BOOLEAN_GETTER(isValid, OOGFOS_SHIP_IS_VALID)
SHIP_BOOLEAN_GETTER(isShip, OOGFOS_SHIP_IS_SHIP)
SHIP_BOOLEAN_GETTER(isStation, OOGFOS_SHIP_IS_STATION)
SHIP_BOOLEAN_GETTER(isPlayer, OOGFOS_SHIP_IS_PLAYER)
SHIP_BOOLEAN_GETTER(isDock, OOGFOS_SHIP_IS_DOCK)
SHIP_BOOLEAN_GETTER(isInSpace, OOGFOS_SHIP_IS_IN_SPACE)

#define SHIP_FLOAT_ACCESSOR(name,property) \
static JSValue get_##name(JSTask task,APTR self,APTR data) { \
    (void)data;return getFloat(task,self,property); } \
static LONG set_##name(JSTask task,APTR self,JSValue input,APTR data) { \
    (void)data;return setFloat(task,self,input,property); }
SHIP_FLOAT_ACCESSOR(energy, OOGFOS_SHIP_ENERGY)
SHIP_FLOAT_ACCESSOR(maxEnergy, OOGFOS_SHIP_MAX_ENERGY)

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
    { (CONST_STRPTR)"energy", JSS_ACCESSOR,0,(APTR)get_energy,(APTR)set_energy,0,0 },
    { (CONST_STRPTR)"maxEnergy", JSS_ACCESSOR,0,(APTR)get_maxEnergy,(APTR)set_maxEnergy,0,0 },
    { (CONST_STRPTR)"isValid", JSS_ACCESSOR,0,(APTR)get_isValid,NULL,0,0 },
    { (CONST_STRPTR)"isShip", JSS_ACCESSOR,0,(APTR)get_isShip,NULL,0,0 },
    { (CONST_STRPTR)"isStation", JSS_ACCESSOR,0,(APTR)get_isStation,NULL,0,0 },
    { (CONST_STRPTR)"isPlayer", JSS_ACCESSOR,0,(APTR)get_isPlayer,NULL,0,0 },
    { (CONST_STRPTR)"isDock", JSS_ACCESSOR,0,(APTR)get_isDock,NULL,0,0 },
    { (CONST_STRPTR)"isInSpace", JSS_ACCESSOR,0,(APTR)get_isInSpace,NULL,0,0 },
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
