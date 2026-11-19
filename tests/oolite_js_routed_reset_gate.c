/* Original Oolite game-state JS reset: prepare replacement through Exec
 * while game owner pumps real ObjC ship selector RPCs; failure must leave
 * old scripts intact, successful reset replaces them. GPL-2.0-or-later.
 */
#include "OOGFOSScriptReset.h"
#include "OOGFOSShipEntityBackend.h"
#include <proto/javascript.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
struct Data {
    struct OOGFOSShipHandle *handle;
    LONG fail,prepares,wills,dids;
};
static LONG prepare(JSTask task,APTR user)
{
    struct Data *d=(struct Data *)user;
    JSClass cls;
    JSValue global=NULL,wrapper=NULL,readBack=NULL;
    LONG rc=JSERR_BADARGS,n=-1;
    ++d->prepares;
    cls=JS_RegisterClass(task,OOGFOSShipClass());
    if(!cls)return JSERR_NOMEMORY;
    wrapper=JS_Wrap(task,cls,d->handle);
    if(!wrapper)return JSERR_NOMEMORY;
    global=JS_Global(task);
    if(!global)goto done;
    rc=JS_SetProp(task,global,"ship",wrapper);
    if(rc==JSERR_OK)rc=JS_Evaluate(task,
       "ship.bounty + ship.homeSystem + ship.maxEscorts",
       -1,"oolite-reset-bootstrap.js",&readBack);
    if(rc==JSERR_OK)rc=JS_ToLong(task,readBack,&n);
    if(rc==JSERR_OK && n!=19)rc=JSERR_TYPE;
    if(rc==JSERR_OK && d->fail)rc=JSERR_TYPE; /* simulated init failure */
done:
    if(readBack)JS_Free(task,readBack);
    if(global)JS_Free(task,global);
    JS_Free(task,wrapper);
    return rc;
}
static void will(APTR user){++((struct Data*)user)->wills;}
static void did(APTR user){++((struct Data*)user)->dids;}
struct Script {CONST_STRPTR source;};
static LONG execScript(JSTask task,APTR user,LONG *result)
{
    JSValue out=NULL;
    struct Script *s=(struct Script *)user;
    LONG rc=JS_Evaluate(task,s->source,-1,"oolite-reset-verify.js",&out);
    if(rc==JSERR_OK)rc=JS_ToLong(task,out,result);
    if(out)JS_Free(task,out);
    return rc;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct OOGFOSShipEntityRegistry registry;
    struct OOGFOSGameDispatch dispatch;
    struct OOGFOSGameShipProxy proxy;
    struct OOGFOSShipHandle handle;
    struct Data data={0};
    struct Script init={"var persistent=23; ship.bounty + ship.homeSystem + ship.maxEscorts"},
                  preserved={"persistent===23 ? ship.bounty + ship.homeSystem + ship.maxEscorts : 0"},
                  changed={"typeof persistent==='undefined' ? ship.bounty + ship.homeSystem + ship.maxEscorts : 0"};
    void *ship=NULL;ULONG token=0;
    LONG first=-1,abort=-1,after=-1;
    LONG rc1=-1,rc2=-1,rc3=-1,rc4=-1,rc5=-1;
    int bound=0,ok=0,closed=0;
    OOGFOSShipEntityRegistryInit(&registry);
    ship=OOGFOSTestObjCShipNew();
    if(!ship)return 75;
    token=OOGFOSShipEntityRegister(&registry,ship);
    if(!token || !OOGFOSGameDispatchInitRouted(&dispatch,&registry,
       OOGFOSShipEntityGameBackend()))return 75;
    if(!OOGFOSJSOpen(&js))return 77;
    proxy.dispatch=&dispatch;proxy.token=token;
    handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
    data.handle=&handle;
    bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,"ship");
    if(bound){
        rc1=OOGFOSScriptInvoke(&js,&dispatch,execScript,&init,&first);
        data.fail=1;
        rc2=OOGFOSScriptResetRouted(&js,&dispatch,prepare,will,did,&data);
        rc3=OOGFOSScriptInvoke(&js,&dispatch,execScript,&preserved,&abort);
        data.fail=0;
        rc4=OOGFOSScriptResetRouted(&js,&dispatch,prepare,will,did,&data);
        rc5=OOGFOSScriptInvoke(&js,&dispatch,execScript,&changed,&after);
    }
    ok=bound && rc1==JSERR_OK && first==19 &&
       rc2==JSERR_TYPE && rc3==JSERR_OK && abort==19 &&
       rc4==JSERR_OK && rc5==JSERR_OK && after==19 &&
       data.prepares==2 && data.wills==1 && data.dids==1 &&
       dispatch.requests==5 && dispatch.completions==5;
    OOGFOSShipInvalidate(&handle);
    OOGFOSJSClose(&js);
    OOGFOSShipEntityUnregister(&registry,token);
    closed=OOGFOSGameDispatchTryClose(&dispatch);
    OOGFOSTestObjCShipFree(ship);
    printf("OOLITE_JS_ROUTED_RESET values=%ld,%ld,%ld codes=%ld,%ld,%ld,%ld,%ld prepares=%ld notices=%ld,%ld rpc=%lu done=%lu closed=%d status=%s\n",
      (long)first,(long)abort,(long)after,
      (long)rc1,(long)rc2,(long)rc3,(long)rc4,(long)rc5,
      (long)data.prepares,(long)data.wills,(long)data.dids,
      (unsigned long)dispatch.requests,(unsigned long)dispatch.completions,
      closed,ok&&closed?"PASS":"FAIL");
    return ok&&closed?0:2;
}
