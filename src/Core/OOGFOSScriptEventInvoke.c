/* Synchronous Oolite OOJSScript event boundary over the canonical task.
 * GPL-2.0-or-later.
 */
#include "OOGFOSScriptEventInvoke.h"
#include "OOGFOSScriptObjects.h"
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
