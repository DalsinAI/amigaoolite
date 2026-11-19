/* OOJavaScriptEngine -callJSFunction: GFOS source compatibility gate.
 * Verify receiver/argv, opaque owned object result, JavaScript exception
 * and next-call recovery with real ObjC/Exec game task ShipEntity access.
 * Native AC090 68040; GPL-2.0-or-later.
 */
#include "OOGFOSScriptInvoke.h"
#include "OOGFOSShipEntityBackend.h"
#include <proto/javascript.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
int main(void)
{
    struct OOGFOSJS js={0};
    struct OOGFOSShipEntityRegistry registry;
    struct OOGFOSGameDispatch dispatch;
    struct OOGFOSGameShipProxy proxy;
    struct OOGFOSShipHandle handle;
    JSValue global=NULL,fn=NULL,throwFn=NULL,returned=NULL,sum=NULL;
    JSValue args[2]={NULL,NULL};
    void *ship=NULL;ULONG token=0;
    LONG setup=JSERR_BADARGS,a=JSERR_BADARGS,b=JSERR_BADARGS,c=JSERR_BADARGS,
         got=-1,recovered=-1;
    int registered=0,ok=0,closed=0,exceptionNull=0;
    OOGFOSShipEntityRegistryInit(&registry);
    ship=OOGFOSTestObjCShipNew();
    if(!ship)return 75;
    token=OOGFOSShipEntityRegister(&registry,ship);
    if(!token || !OOGFOSGameDispatchInitRouted(&dispatch,&registry,
             OOGFOSShipEntityGameBackend()))return 75;
    if(!OOGFOSJSOpen(&js))return 77;
    proxy.dispatch=&dispatch;proxy.token=token;
    handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
    registered=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,"ship");
    if(registered) setup=OOGFOSJSEvaluateSource(&js,
       "this.ooliteCall=function(a,b){return {sum:a+b+ship.bounty,tag:'cobra'};};"
       "this.ooliteThrow=function(){throw new Error('expected');};"
       "this.ooliteRecovery=function(){return ship.bounty*2;};",
       "oolite-function-bindings.js");
    if(setup!=JSERR_OK)goto cleanup;
    global=JS_Global(js.task);
    fn=JS_GetProp(js.task,global,"ooliteCall");
    throwFn=JS_GetProp(js.task,global,"ooliteThrow");
    args[0]=JS_NewLong(js.task,2);
    args[1]=JS_NewLong(js.task,3);
    if(fn && args[0] && args[1])
        a=OOGFOSScriptInvokeFunction(&js,&dispatch,fn,global,2,args,&returned);
    if(a==JSERR_OK && returned && JS_TypeOf(js.task,returned)==JST_OBJECT) {
        sum=JS_GetProp(js.task,returned,"sum");
        if(sum)JS_ToLong(js.task,sum,&got);
    }
    if(sum){JS_Free(js.task,sum);sum=NULL;}
    if(returned){JS_Free(js.task,returned);returned=NULL;}
    if(throwFn) b=OOGFOSScriptInvokeFunction(&js,&dispatch,
                                             throwFn,global,0,NULL,&returned);
    exceptionNull=(b==JSERR_EXCEPTION && returned==NULL);
    if(returned){JS_Free(js.task,returned);returned=NULL;}
    if(fn){JS_Free(js.task,fn);fn=NULL;}
    fn=JS_GetProp(js.task,global,"ooliteRecovery");
    if(fn)c=OOGFOSScriptInvokeFunction(&js,&dispatch,
                                      fn,global,0,NULL,&returned);
    if(c==JSERR_OK && returned)JS_ToLong(js.task,returned,&recovered);
    ok=registered&&setup==JSERR_OK&&a==JSERR_OK&&got==15&&
       b==JSERR_EXCEPTION&&exceptionNull&&returned!=NULL&&c==JSERR_OK&&recovered==20&&
       dispatch.requests==2&&dispatch.completions==3;
cleanup:
    if(returned)JS_Free(js.task,returned);
    if(fn)JS_Free(js.task,fn);
    if(throwFn)JS_Free(js.task,throwFn);
    if(global)JS_Free(js.task,global);
    if(args[0])JS_Free(js.task,args[0]);
    if(args[1])JS_Free(js.task,args[1]);
    OOGFOSShipInvalidate(&handle);
    OOGFOSJSClose(&js);
    OOGFOSShipEntityUnregister(&registry,token);
    closed=OOGFOSGameDispatchTryClose(&dispatch);
    OOGFOSTestObjCShipFree(ship);
    printf("OOLITE_JS_CALL_FUNCTION setup=%ld codes=%ld,%ld,%ld sum=%ld recovered=%ld rpc=%lu done=%lu closed=%d status=%s\n",
       (long)setup,(long)a,(long)b,(long)c,(long)got,(long)recovered,
       (unsigned long)dispatch.requests,(unsigned long)dispatch.completions,
       closed,ok&&closed?"PASS":"FAIL");
    return ok&&closed?0:2;
}
