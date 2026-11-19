/* Synchronous original Oolite game-script calls pump Exec game RPC while
 * javascript.library executes on its task. Native 68040 integration test.
 * GPL-2.0-or-later.
 */
#include "OOGFOSScriptInvoke.h"
#include "OOGFOSShipEntityBackend.h"
#include <proto/javascript.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
extern int OOGFOSTestObjCShipValues(void *ship,int *b,int *h,int *e,int *r);
struct Case {CONST_STRPTR source;};
static LONG eval(JSTask task,APTR data,LONG *result)
{
    struct Case *c=(struct Case *)data;
    JSValue value=NULL;
    LONG rc;
    if(!result)return JSERR_BADARGS;
    rc=JS_Evaluate(task,c->source,-1,"oolite-synchronous-script.js",&value);
    if(rc==JSERR_OK && value) rc=JS_ToLong(task,value,result);
    if(value)JS_Free(task,value);
    return rc;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct OOGFOSShipEntityRegistry registry;
    struct OOGFOSGameDispatch dispatcher;
    struct OOGFOSGameShipProxy proxy;
    struct OOGFOSShipHandle handle;
    struct Case first={"ship.bounty+ship.homeSystem+ship.maxEscorts"},
                second={"ship.bounty=25;ship.bounty"},
                throws={"throw new Error('expected oolite sync error')"},
                after={"ship.bounty*2"};
    void *ship=NULL;ULONG token=0;
    LONG a=-1,b=-1,c=-1,d=-1,r1=-1,r2=-1,r3=-1,r4=-1;
    int bound=0,closed=0,ok=0,bounty=0,home=0,escorts=0,reason=0;
    OOGFOSShipEntityRegistryInit(&registry);
    ship=OOGFOSTestObjCShipNew();if(!ship)return 75;
    token=OOGFOSShipEntityRegister(&registry,ship);
    if(!token || !OOGFOSGameDispatchInitRouted(&dispatcher,&registry,
               OOGFOSShipEntityGameBackend()))return 75;
    if(!OOGFOSJSOpen(&js))return 77;
    proxy.dispatch=&dispatcher;proxy.token=token;
    handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
    bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,"ship");
    if(bound) {
        r1=OOGFOSScriptInvoke(&js,&dispatcher,eval,&first,&a);
        r2=OOGFOSScriptInvoke(&js,&dispatcher,eval,&second,&b);
        r3=OOGFOSScriptInvoke(&js,&dispatcher,eval,&throws,&c);
        r4=OOGFOSScriptInvoke(&js,&dispatcher,eval,&after,&d);
    }
    OOGFOSTestObjCShipValues(ship,&bounty,&home,&escorts,&reason);
    ok=bound && r1==JSERR_OK && a==19 &&
       r2==JSERR_OK && b==25 &&
       r3==JSERR_EXCEPTION && c==0 &&
       r4==JSERR_OK && d==50 &&
       bounty==25 && home==7 && escorts==2 &&
       dispatcher.requests==6 && dispatcher.completions==4;
    OOGFOSShipInvalidate(&handle);
    OOGFOSJSClose(&js);
    OOGFOSShipEntityUnregister(&registry,token);
    closed=OOGFOSGameDispatchTryClose(&dispatcher);
    OOGFOSTestObjCShipFree(ship);
    printf("OOLITE_JS_SYNC_CALL values=%ld,%ld,%ld,%ld codes=%ld,%ld,%ld,%ld rpc=%lu done=%lu closed=%d status=%s\n",
           (long)a,(long)b,(long)c,(long)d,
           (long)r1,(long)r2,(long)r3,(long)r4,
           (unsigned long)dispatcher.requests,
           (unsigned long)dispatcher.completions,closed,
           ok&&closed?"PASS":"FAIL");
    return ok&&closed?0:2;
}
