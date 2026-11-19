/* Four original OOJSEntity read-only GLfloat properties over game task
 * Exec bridge to native ObjC Entity selectors. Native AC090 68040.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSShipEntityBackend.h"
#include <proto/javascript.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
struct Job {struct OOGFOSGameDispatch *dispatch;LONG rc,writeRC,result,done;};
static LONG job(JSTask task,APTR data)
{
    struct Job *j=(struct Job*)data;
    JSValue value=NULL;LONG answer=-1;
    j->rc=JS_Evaluate(task,
      "ship.collisionRadius===28.25 && "
      "ship.distanceTravelled===1234.5 && "
      "ship.mass===90.5 && ship.spawnTime===3.75 && "
      "ship.collisionRadius+ship.distanceTravelled+ship.mass+ship.spawnTime===1357 "
      "? 42 : 0",
      -1,"oolite-entity-scalar-getters.js",&value);
    if(j->rc==JSERR_OK && value)j->rc=JS_ToLong(task,value,&answer);
    if(value)JS_Free(task,value);
    j->writeRC=JS_Evaluate(task,
      "\"use strict\"; ship.mass=999;",
      -1,"oolite-mass-read-only.js",NULL);
    j->result=answer;
    j->done=OOGFOSGameDispatchDone(j->dispatch,j->rc,answer);
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
    struct Job work;
    void *ship;ULONG token;LONG posted=-1,joined=-1;
    int bound=0,pumped,ok=0,closed=0;
    OOGFOSShipEntityRegistryInit(&reg);
    ship=OOGFOSTestObjCShipNew();if(!ship)return 75;
    token=OOGFOSShipEntityRegister(&reg,ship);
    if(!token || !OOGFOSGameDispatchInitRouted(&dispatch,&reg,
        OOGFOSShipEntityGameBackend()))return 75;
    if(!OOGFOSJSOpen(&js))return 77;
    proxy.dispatch=&dispatch;proxy.token=token;
    handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
    bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,"ship");
    work=(struct Job){&dispatch,JSERR_BADARGS,JSERR_BADARGS,-1,0};
    if(bound)posted=JS_Post(js.task,job,&work,0);
    if(posted==JSERR_OK){
        do{pumped=OOGFOSGameDispatchServe(&dispatch);}while(pumped==0);
        joined=JS_Post(js.task,join,NULL,1);
    }
    ok=bound && posted==JSERR_OK && joined==JSERR_OK &&
       work.rc==JSERR_OK && work.result==42 && work.done==1 &&
       work.writeRC==JSERR_EXCEPTION &&
       dispatch.requests==8 && dispatch.completions==1;
    OOGFOSShipInvalidate(&handle);OOGFOSJSClose(&js);
    OOGFOSShipEntityUnregister(&reg,token);
    closed=OOGFOSGameDispatchTryClose(&dispatch);
    OOGFOSTestObjCShipFree(ship);
    printf("OOLITE_ENTITY_SCALARS code=%ld write=%ld result=%ld rpc=%lu done=%lu closed=%d status=%s\n",
       (long)work.rc,(long)work.writeRC,(long)work.result,
       (unsigned long)dispatch.requests,(unsigned long)dispatch.completions,
       closed,ok&&closed?"PASS":"FAIL");
    return ok&&closed?0:2;
}
