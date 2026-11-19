/* OOJSScript -callMethod:result: native GFOS replacement: arbitrary owned
 * script object result, missing method, thrown exception, recovery and
 * ship-entity game task RPC while the main caller waits. GPL-2.0-or-later.
 */
#include "OOGFOSScriptEventInvoke.h"
#include "OOGFOSScriptObjects.h"
#include "OOGFOSShipEntityBackend.h"
#include <proto/javascript.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
struct Setup {const char *source;int create;};
static LONG setupScript(JSTask task,APTR data,LONG *out)
{
    struct Setup *setup=(struct Setup *)data;
    LONG rc=JSERR_OK;
    if(out)*out=0;
    if(setup->create) {
        rc=OOGFOSScriptObjectCreate(task,"script","ship");
        if(rc!=JSERR_OK)return rc;
    }
    return JS_Evaluate(task,setup->source,-1,"oolite-script-method-test.js",NULL);
}
static LONG readResult(JSTask task,JSValue object,CONST_STRPTR property,LONG *value)
{
    JSValue v;
    LONG rc;
    if(!object || !value)return JSERR_BADARGS;
    v=JS_GetProp(task,object,property);
    if(!v)return JSERR_TYPE;
    rc=JS_ToLong(task,v,value);
    JS_Free(task,v);
    return rc;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct OOGFOSShipEntityRegistry registry;
    struct OOGFOSGameDispatch dispatch;
    struct OOGFOSGameShipProxy proxy;
    struct OOGFOSShipHandle handle;
    struct Setup initialize={
        "script.onCargo=function(q){return {sum:q+this.ship.bounty,valid:this.ship.isValid?1:0};};",1
    };
    struct Setup inject={
        "script.onCargo=function(q){throw new Error('expected Oolite event exception');};",0
    };
    struct Setup recover={
        "script.onCargo=function(q){return {sum:q+this.ship.bounty,valid:this.ship.isShip?1:0};};",0
    };
    void *ship;ULONG token;JSValue arg=NULL,result=NULL;
    LONG setupRC=-1,throwSetRC=-1,recoverSetRC=-1,
         firstRC=-1,missingRC=-1,throwRC=-1,recoverRC=-1,
         firstCalled=-1,missingCalled=-1,throwCalled=-1,recoverCalled=-1,
         sum=-1,valid=-1,sumAfter=-1,validAfter=-1,ignored=-1;
    int registered=0,closed=0,ok=0;
    OOGFOSShipEntityRegistryInit(&registry);
    ship=OOGFOSTestObjCShipNew();if(!ship)return 75;
    token=OOGFOSShipEntityRegister(&registry,ship);
    if(!token || !OOGFOSGameDispatchInitRouted(&dispatch,&registry,
        OOGFOSShipEntityGameBackend()))return 75;
    if(!OOGFOSJSOpen(&js))return 77;
    proxy.dispatch=&dispatch;proxy.token=token;
    handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
    registered=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,"ship");
    if(registered)
        setupRC=OOGFOSScriptInvoke(&js,&dispatch,setupScript,&initialize,&ignored);
    arg=JS_NewLong(js.task,34);
    if(setupRC==JSERR_OK && arg)
        firstRC=OOGFOSScriptInvokeEventValue(&js,&dispatch,
                         "script","onCargo",1,&arg,&result,&firstCalled);
    if(firstRC==JSERR_OK && result) {
        readResult(js.task,result,"sum",&sum);
        readResult(js.task,result,"valid",&valid);
    }
    if(result){JS_Free(js.task,result);result=NULL;}
    if(firstRC==JSERR_OK)
        missingRC=OOGFOSScriptInvokeEventValue(&js,&dispatch,
                         "script","notImplemented",0,NULL,&result,&missingCalled);
    if(result){JS_Free(js.task,result);result=NULL;}
    if(missingRC==JSERR_OK)
        throwSetRC=OOGFOSScriptInvoke(&js,&dispatch,setupScript,&inject,&ignored);
    if(throwSetRC==JSERR_OK)
        throwRC=OOGFOSScriptInvokeEventValue(&js,&dispatch,
                         "script","onCargo",1,&arg,&result,&throwCalled);
    if(result){JS_Free(js.task,result);result=NULL;}
    if(throwRC==JSERR_EXCEPTION)
        recoverSetRC=OOGFOSScriptInvoke(&js,&dispatch,setupScript,&recover,&ignored);
    if(recoverSetRC==JSERR_OK)
        recoverRC=OOGFOSScriptInvokeEventValue(&js,&dispatch,
                         "script","onCargo",1,&arg,&result,&recoverCalled);
    if(recoverRC==JSERR_OK && result){
        readResult(js.task,result,"sum",&sumAfter);
        readResult(js.task,result,"valid",&validAfter);
    }
    if(result)JS_Free(js.task,result);
    if(arg)JS_Free(js.task,arg);
    ok=registered&&setupRC==JSERR_OK&&throwSetRC==JSERR_OK&&
       recoverSetRC==JSERR_OK&&firstRC==JSERR_OK&&missingRC==JSERR_OK&&
       throwRC==JSERR_EXCEPTION&&recoverRC==JSERR_OK&&
       firstCalled==1&&missingCalled==0&&throwCalled==0&&recoverCalled==1&&
       sum==44&&valid==1&&sumAfter==44&&validAfter==1&&
       dispatch.requests==4&&dispatch.completions==7;
    OOGFOSShipInvalidate(&handle);OOGFOSJSClose(&js);
    OOGFOSShipEntityUnregister(&registry,token);
    closed=OOGFOSGameDispatchTryClose(&dispatch);
    OOGFOSTestObjCShipFree(ship);
    printf("OOLITE_SCRIPT_RETURN setup=%ld,%ld,%ld events=%ld,%ld,%ld,%ld called=%ld,%ld,%ld,%ld sum=%ld,%ld valid=%ld,%ld rpc=%lu done=%lu closed=%d status=%s\n",
        (long)setupRC,(long)throwSetRC,(long)recoverSetRC,
        (long)firstRC,(long)missingRC,(long)throwRC,(long)recoverRC,
        (long)firstCalled,(long)missingCalled,(long)throwCalled,(long)recoverCalled,
        (long)sum,(long)sumAfter,(long)valid,(long)validAfter,
        (unsigned long)dispatch.requests,(unsigned long)dispatch.completions,
        closed,ok&&closed?"PASS":"FAIL");
    return ok&&closed?0:2;
}
