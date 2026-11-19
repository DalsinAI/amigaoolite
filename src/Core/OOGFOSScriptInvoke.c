/* Oolite canonical JavaScript game-task synchronous scheduler.
 * Prevents JS-to-ShipEntity RPC deadlocks while keeping script execution
 * on the single JS engine task. GPL-2.0-or-later.
 */
#include "OOGFOSScriptInvoke.h"
#include <proto/javascript.h>
#include <proto/exec.h>
struct OOGFOSInvocation {
    struct OOGFOSGameDispatch *dispatch;
    OOGFOSScriptInvokeFunc callback;
    APTR user;
    LONG rc,value,delivered;
};
static LONG runAndComplete(JSTask task,APTR data)
{
    struct OOGFOSInvocation *job=(struct OOGFOSInvocation *)data;
    LONG value=0;
    job->rc=job->callback(task,job->user,&value);
    job->value=value;
    /* Always send completion, including after a JS exception, so the
     * waiting owner task is never blocked expecting a missing DONE. */
    job->delivered=OOGFOSGameDispatchDone(job->dispatch,job->rc,value);
    return job->rc;
}
static LONG join(JSTask task,APTR data)
{
    (void)task;(void)data;
    return JSERR_OK;
}
LONG OOGFOSScriptInvoke(struct OOGFOSJS *js,
                       struct OOGFOSGameDispatch *dispatch,
                       OOGFOSScriptInvokeFunc callback,
                       APTR user, LONG *result)
{
    struct OOGFOSInvocation job;
    LONG rc;
    int event;
    if(result)*result=0;
    if(!js || !js->task || !js->library || !dispatch ||
       !dispatch->gamePort || dispatch->sealed ||
       !callback || !result || FindTask(NULL)!=dispatch->gameTask)
        return JSERR_BADARGS;
    JavaScriptBase=js->library;
    job.dispatch=dispatch;
    job.callback=callback;
    job.user=user;
    job.rc=JSERR_BADARGS;
    job.value=0;
    job.delivered=0;
    rc=JS_Post(js->task,runAndComplete,&job,0);
    if(rc!=JSERR_OK)return rc;
    do {
        event=OOGFOSGameDispatchServe(dispatch);
    } while(event==0);
    /* Completion RPC has been replied to. Wait for worker to leave its
     * callback before reusing its stack-backed invocation record. */
    rc=JS_Post(js->task,join,NULL,1);
    if(rc!=JSERR_OK || event!=1 || !job.delivered)
        return rc!=JSERR_OK ? rc : JSERR_BADARGS;
    if(job.rc==JSERR_OK) *result=job.value;
    return job.rc;
}

/* OOJavaScriptEngine -callJSFunction: parity boundary for canonical GFOS.
 * The callback's JS_Call result is an owned handle. A bare result pointer
 * must not be confused with SpiderMonkey's in-place jsval word. */
struct OOGFOSFunctionCall {
    JSTask expectedTask;
    JSValue function, self;
    LONG argc;
    JSValue *argv;
    JSValue returned;
};
static LONG executeFunction(JSTask task, APTR user, LONG *ignored)
{
    struct OOGFOSFunctionCall *call=(struct OOGFOSFunctionCall *)user;
    LONG rc;
    if(ignored) *ignored=0;
    if(!call || task!=call->expectedTask) return JSERR_BADARGS;
    call->returned=NULL;
    rc=JS_Call(task,call->function,call->self,call->argc,
               call->argv,&call->returned);
    if(rc!=JSERR_OK && call->returned){
        JS_Free(task,call->returned);
        call->returned=NULL;
    }
    return rc;
}
LONG OOGFOSScriptInvokeFunction(struct OOGFOSJS *js,
                               struct OOGFOSGameDispatch *dispatch,
                               JSValue function, JSValue self,
                               LONG argc, JSValue *argv, JSValue *result)
{
    struct OOGFOSFunctionCall call;
    LONG ignored=0,rc;
    if(result) *result=NULL;
    if(!js || !js->task || !dispatch || !function ||
       argc<0 || (argc>0 && !argv) || !result) return JSERR_BADARGS;
    call.expectedTask=js->task;
    call.function=function;
    call.self=self;
    call.argc=argc;
    call.argv=argv;
    call.returned=NULL;
    rc=OOGFOSScriptInvoke(js,dispatch,executeFunction,&call,&ignored);
    if(rc==JSERR_OK)*result=call.returned;
    else if(call.returned){
        /* A failed posted callback should already have released its
         * result, but avoid carrying a stale value across Oolite tasks. */
        JS_Free(js->task,call.returned);
    }
    return rc;
}
