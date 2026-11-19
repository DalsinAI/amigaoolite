/* Synchronous Oolite OOJSScript event boundary over the canonical task.
 * GPL-2.0-or-later.
 */
#include "OOGFOSScriptEventInvoke.h"
#include "OOGFOSScriptObjects.h"
#include <proto/javascript.h>
struct EventCall {
    CONST_STRPTR script,method;
    LONG argc;
    JSValue *argv;
};
static LONG runEvent(JSTask task, APTR user, LONG *result)
{
    struct EventCall *call=(struct EventCall *)user;
    return OOGFOSScriptObjectEvent(task,call->script,call->method,
                                  call->argc,call->argv,result);
}
LONG OOGFOSScriptInvokeEvent(struct OOGFOSJS *js,
                            struct OOGFOSGameDispatch *dispatch,
                            CONST_STRPTR scriptName, CONST_STRPTR eventName,
                            LONG argc, JSValue *argv, LONG *called)
{
    struct EventCall call;
    if(called) *called=0;
    if(!scriptName || !eventName || !called || argc<0 ||
       (argc && !argv)) return JSERR_BADARGS;
    call.script=scriptName;call.method=eventName;
    call.argc=argc;call.argv=argv;
    return OOGFOSScriptInvoke(js,dispatch,runEvent,&call,called);
}

/* Transport complete OOJSScript result handles across the script/game
 * task boundary, using the same guaranteed-DONE Exec RPC pump. */
struct ValueEventCall {
    CONST_STRPTR script,method;
    LONG argc;
    JSValue *argv;
    JSValue returned;
};
static LONG runValueEvent(JSTask task, APTR user, LONG *called)
{
    struct ValueEventCall *c=(struct ValueEventCall *)user;
    c->returned=NULL;
    return OOGFOSScriptObjectEventValue(task,c->script,c->method,
                                       c->argc,c->argv,&c->returned,called);
}
LONG OOGFOSScriptInvokeEventValue(struct OOGFOSJS *js,
                                 struct OOGFOSGameDispatch *dispatch,
                                 CONST_STRPTR scriptName,
                                 CONST_STRPTR eventName,
                                 LONG argc, JSValue *argv,
                                 JSValue *result, LONG *called)
{
    struct ValueEventCall event;
    LONG rc;
    if(result)*result=NULL;
    if(called)*called=0;
    if(!js || !js->task || !result || !called ||
       !scriptName || !eventName || argc<0 ||
       (argc>0 && !argv))return JSERR_BADARGS;
    event.script=scriptName;
    event.method=eventName;
    event.argc=argc;
    event.argv=argv;
    event.returned=NULL;
    rc=OOGFOSScriptInvoke(js,dispatch,runValueEvent,&event,called);
    if(rc==JSERR_OK)
        *result=event.returned;
    else if(event.returned)
        JS_Free(js->task,event.returned);
    return rc;
}
