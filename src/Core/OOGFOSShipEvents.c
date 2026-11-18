#include "OOGFOSShipEvents.h"
#include "OOGFOSShipProperties.h"
#include "OOGFOSGameDispatch.h"
#include <proto/javascript.h>

static JSValue legacy(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    struct OOGFOSShipHandle *h=(struct OOGFOSShipHandle *)self;
    JSValue length=NULL;
    LONG count=0,result=0;
    (void)data;
    if (!h || !h->valid || !h->ship || argc<2)
        return JS_Throw(task,(CONST_STRPTR)"Oolite legacy ship action unavailable");
    length=JS_GetProp(task,argv[1],(CONST_STRPTR)"length");
    if (!length || JS_ToLong(task,length,&count)!=JSERR_OK) {
        if(length)JS_Free(task,length);
        return JS_Throw(task,(CONST_STRPTR)"Oolite invalid legacy action list");
    }
    JS_Free(task,length);
    if(count<0 || count>4096 || !OOGFOSGameShipAction((struct OOGFOSGameShipProxy *)h->ship,
        OOGFOS_SHIP_ACTION_LEGACY,count,&result))
        return JS_Throw(task,(CONST_STRPTR)"Oolite legacy action dispatch rejected");
    return JS_NewLong(task,result);
}
static const struct JSHostSlot eventSlots[]={
    {(CONST_STRPTR)"__runLegacyScriptActions",JSS_METHOD,0,(APTR)legacy,NULL,0,2},
    {NULL,0,0,NULL,NULL,0,0}
};
const struct JSHostClass *OOGFOSShipClassWithEvents(void)
{
    static struct JSHostClass cls={
        (CONST_STRPTR)"OoliteShipWithGameEvents",NULL,eventSlots,NULL,NULL
    };
    /* Class and slots are static for lifetime of every JavaScript task. */
    cls.jc_Parent=OOGFOSShipClass();
    return &cls;
}
