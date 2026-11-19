/* Native OOJSEntity type/validity flags through real ObjC selector calls.
 * Script callbacks never dereference ShipEntity outside game-owner task.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSShipEntityBackend.h"
#include <proto/javascript.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
extern void OOGFOSTestObjCShipSetStatus(void *ship,int status);
struct Job {struct OOGFOSGameDispatch *dispatch;const char *source;LONG rc,result,done;};
static LONG evaluate(JSTask task,APTR data)
{
    struct Job *j=(struct Job *)data;
    JSValue result=NULL;
    LONG n=-1;
    j->rc=JS_Evaluate(task,j->source,-1,"oolite-entity-flags.js",&result);
    if(j->rc==JSERR_OK && result)j->rc=JS_ToLong(task,result,&n);
    if(result)JS_Free(task,result);
    j->result=n;
    j->done=OOGFOSGameDispatchDone(j->dispatch,j->rc,n);
    return j->rc;
}
static LONG join(JSTask task,APTR data){(void)task;(void)data;return JSERR_OK;}
static int runJob(struct OOGFOSJS *js,struct OOGFOSGameDispatch *dispatch,struct Job *job)
{
    LONG posted=JS_Post(js->task,evaluate,job,0);
    int poll;
    if(posted!=JSERR_OK)return 0;
    do{poll=OOGFOSGameDispatchServe(dispatch);}while(poll==0);
    return poll==1 && JS_Post(js->task,join,NULL,1)==JSERR_OK &&
           job->done && job->rc==JSERR_OK && job->result==42;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct OOGFOSShipEntityRegistry reg;
    struct OOGFOSGameDispatch dispatch;
    struct OOGFOSGameShipProxy proxy;
    struct OOGFOSShipHandle handle;
    struct Job alive,dead;
    void *ship;ULONG token;
    int registered=0,aliveOK=0,deadOK=0,closed=0,ok;
    OOGFOSShipEntityRegistryInit(&reg);
    ship=OOGFOSTestObjCShipNew();
    if(!ship)return 75;
    token=OOGFOSShipEntityRegister(&reg,ship);
    if(!token || !OOGFOSGameDispatchInitRouted(&dispatch,&reg,
       OOGFOSShipEntityGameBackend()))return 75;
    if(!OOGFOSJSOpen(&js))return 77;
    proxy.dispatch=&dispatch;proxy.token=token;
    handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
    registered=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,"ship");
    alive=(struct Job){&dispatch,
      "typeof ship.isValid==='boolean' && ship.isValid===true && "
      "ship.isShip===true && ship.isStation===false && "
      "ship.isPlayer===false && ship.isDock===false && "
      "ship.isInSpace===true ? 42:0",-1,-1,0};
    dead=(struct Job){&dispatch,
      "ship.isValid===false ? 42:0",-1,-1,0};
    if(registered)aliveOK=runJob(&js,&dispatch,&alive);
    /* Pinned OOEntityStatus.tbl: STATUS_DEAD=-1; game task owns the update. */
    OOGFOSTestObjCShipSetStatus(ship,-1);
    if(aliveOK)deadOK=runJob(&js,&dispatch,&dead);
    ok=registered&&aliveOK&&deadOK&&dispatch.completions==2;
    OOGFOSShipInvalidate(&handle);OOGFOSJSClose(&js);
    OOGFOSShipEntityUnregister(&reg,token);
    closed=OOGFOSGameDispatchTryClose(&dispatch);
    OOGFOSTestObjCShipFree(ship);
    printf("OOLITE_JS_ENTITY_FLAGS live=%d dead=%d rpc=%lu done=%lu closed=%d status=%s\n",
      aliveOK,deadOK,(unsigned long)dispatch.requests,
      (unsigned long)dispatch.completions,closed,
      ok&&closed?"PASS":"FAIL");
    return ok&&closed?0:2;
}
