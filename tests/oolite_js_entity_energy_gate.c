/* Real OOJSEntity energy/maxEnergy selectors routed to game task.
 * Pinned Entity selector semantics via ObjC fixture; no direct JS-task
 * access to ShipEntity. Native AC090 test, GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSShipEntityBackend.h"
#include <proto/javascript.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
extern int OOGFOSTestObjCShipEnergy(void *ship,float *energy,float *maxEnergy);
struct Job {
    struct OOGFOSGameDispatch *dispatcher;
    DOUBLE fraction;
    LONG rc, final, rejected, result, done;
};
static LONG testOnJSTask(JSTask task, APTR data)
{
    struct Job *j=(struct Job *)data;
    JSValue v=NULL;
    LONG n=-1;
    j->rc=JS_Evaluate(task,(CONST_STRPTR)
      "ship.energy=77.25;ship.energy;",
      -1,(CONST_STRPTR)"energy-fraction.js",&v);
    if(j->rc==JSERR_OK && v)
       j->rc=JS_ToDouble(task,v,&j->fraction);
    if(v)JS_Free(task,v);
    v=NULL;
    if(j->rc==JSERR_OK)
      j->rc=JS_Evaluate(task,(CONST_STRPTR)
       "ship.maxEnergy=100;ship.energy=150.5;ship.energy+ship.maxEnergy;",
       -1,(CONST_STRPTR)"energy-clamp.js",&v);
    if(j->rc==JSERR_OK && v)j->rc=JS_ToLong(task,v,&n);
    if(v)JS_Free(task,v);
    j->final=n;
    j->rejected=JSERR_BADARGS;
    if(j->rc==JSERR_OK)
      j->rejected=JS_Evaluate(task,(CONST_STRPTR)
          "ship.maxEnergy=0;",-1,(CONST_STRPTR)"invalid-max-energy.js",NULL);
    j->done=OOGFOSGameDispatchDone(j->dispatcher,j->rc,n);
    return j->rc;
}
static LONG join(JSTask task,APTR data){(void)task;(void)data;return JSERR_OK;}
int main(void)
{
    struct OOGFOSJS js={0};
    struct OOGFOSShipEntityRegistry reg;
    struct OOGFOSGameDispatch dispatch;
    struct OOGFOSGameShipProxy proxy;
    struct OOGFOSShipHandle handle;
    struct Job job={0};
    void *ship=NULL; ULONG token=0;
    float energy=-1,maximum=-1;
    LONG posted=-1,joined=-1;
    int bound=0,closed=0,ok=0,pumped;
    OOGFOSShipEntityRegistryInit(&reg);
    ship=OOGFOSTestObjCShipNew();
    if(!ship)return 75;
    token=OOGFOSShipEntityRegister(&reg,ship);
    if(!token || !OOGFOSGameDispatchInitRouted(&dispatch,&reg,
            OOGFOSShipEntityGameBackend()))return 75;
    if(!OOGFOSJSOpen(&js)){OOGFOSGameDispatchClose(&dispatch);return 77;}
    proxy.dispatch=&dispatch;proxy.token=token;
    handle.ship=&proxy;
    handle.backend=OOGFOSGameShipProxyBackend();
    handle.valid=1;
    bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,"ship");
    job.dispatcher=&dispatch;
    job.rc=JSERR_BADARGS;job.final=-1;job.rejected=JSERR_BADARGS;
    if(bound)posted=JS_Post(js.task,testOnJSTask,&job,0);
    if(posted==JSERR_OK){
        do{pumped=OOGFOSGameDispatchServe(&dispatch);}while(pumped==0);
        joined=JS_Post(js.task,join,NULL,1);
    }
    OOGFOSTestObjCShipEnergy(ship,&energy,&maximum);
    ok=bound && posted==JSERR_OK && joined==JSERR_OK &&
       job.rc==JSERR_OK && job.fraction==77.25 &&
       job.final==200 && job.rejected==JSERR_EXCEPTION &&
       energy==100.0f && maximum==100.0f &&
       dispatch.completions==1 && job.done==1;
    OOGFOSShipInvalidate(&handle);
    OOGFOSJSClose(&js);
    OOGFOSShipEntityUnregister(&reg,token);
    closed=OOGFOSGameDispatchTryClose(&dispatch);
    OOGFOSTestObjCShipFree(ship);
    printf("OOLITE_JS_ENTITY_ENERGY fraction=%ld final=%ld rejected=%ld energy=%ld maximum=%ld rpc=%lu closed=%d status=%s\n",
           (long)(job.fraction*100),(long)job.final,(long)job.rejected,
           (long)energy,(long)maximum,(unsigned long)dispatch.requests,
           closed,ok&&closed?"PASS":"FAIL");
    return ok&&closed?0:2;
}
