/* Native AC090 JS_Post to Oolite game-task Exec MsgPort qualification.
 * JS script executes on engine task, while game owner services property RPC.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSGameDispatch.h"
#include <proto/javascript.h>
#include <proto/exec.h>
#include <stdio.h>
struct TestShip { LONG bounty, home, escorts; LONG writes; struct Task *ownerTask; LONG wrongTask; };
static int getShip(void *ptr, unsigned key, LONG *value) {
    struct TestShip *s=(struct TestShip *)ptr;
    if (FindTask(NULL)!=s->ownerTask) { s->wrongTask++; return 0; }
    switch(key) {
    case OOGFOS_SHIP_BOUNTY: *value=s->bounty;return 1;
    case OOGFOS_SHIP_HOME_SYSTEM: *value=s->home;return 1;
    case OOGFOS_SHIP_MAX_ESCORTS: *value=s->escorts;return 1;
    default:return 0;
    }
}
static int setShip(void *ptr, unsigned key, LONG value) {
    struct TestShip *s=(struct TestShip *)ptr;
    if (FindTask(NULL)!=s->ownerTask) { s->wrongTask++; return 0; }
    switch(key) {
    case OOGFOS_SHIP_BOUNTY: if(value<0||value>100000)return 0;s->bounty=value;break;
    case OOGFOS_SHIP_HOME_SYSTEM: if(value<0||value>255)return 0;s->home=value;break;
    case OOGFOS_SHIP_MAX_ESCORTS: if(value<0||value>16)return 0;s->escorts=value;break;
    default:return 0;
    }
    s->writes++;return 1;
}
struct Job { struct OOGFOSGameDispatch *dispatch; struct Task *gameTask; LONG callbacksOnOwner; };
static LONG scriptJob(JSTask task, APTR userdata) {
    struct Job *job=(struct Job *)userdata;
    JSValue result=NULL;
    LONG value=-1;
    LONG rc=JS_Evaluate(task,
        (CONST_STRPTR)"ship.bounty=44;ship.homeSystem=42;ship.maxEscorts=4;ship.bounty+ship.homeSystem+ship.maxEscorts;",
        -1,(CONST_STRPTR)"oolite-game-dispatch.js", &result);
    if(rc==JSERR_OK) rc=JS_ToLong(task,result,&value);
    if(result) JS_Free(task,result);
    job->callbacksOnOwner = FindTask(NULL)==job->gameTask;
    OOGFOSGameDispatchDone(job->dispatch,rc,value);
    return rc;
}
int main(void) {
    static const struct OOGFOSShipBackend ownerBackend={getShip,setShip};
    struct TestShip ship={10,7,2,0,NULL,0};
    struct OOGFOSGameDispatch dispatch;
    struct OOGFOSShipHandle handle;
    struct OOGFOSJS js;
    struct Job job;
    LONG post=-1;
    int bound=0,ok=0,ret,empty=-1,processed=0;
 LONG emptyBudget=-2,invalidBudget=-2;
    ship.ownerTask=FindTask(NULL);
    if(!OOGFOSGameDispatchInit(&dispatch,&ship,&ownerBackend))return 75;
    empty=OOGFOSGameDispatchPoll(&dispatch);
    emptyBudget=OOGFOSGameDispatchPumpFrame(&dispatch,1);
    invalidBudget=OOGFOSGameDispatchPumpFrame(&dispatch,0);
    if(!OOGFOSJSOpen(&js)) {OOGFOSGameDispatchClose(&dispatch);return 77;}
    handle.ship=&dispatch;
    handle.backend=OOGFOSGameDispatchBackend();
    handle.valid=1;
    bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,(CONST_STRPTR)"ship");
    job.dispatch=&dispatch;job.gameTask=ship.ownerTask;job.callbacksOnOwner=-1;
    if(bound) post=JS_Post(js.task,scriptJob,&job,0);
    if(bound && post==JSERR_OK) {
        do {
            WaitPort(dispatch.gamePort);
            ret=(int)OOGFOSGameDispatchPumpFrame(&dispatch,1);
            if(ret==1) processed++;
        } while (dispatch.completions==0 && ret==1);
    }
    ok=bound && post==JSERR_OK && empty==2 && emptyBudget==0 && invalidBudget==-1 && processed==7 && dispatch.completions==1 &&
        dispatch.resultCode==JSERR_OK && dispatch.result==90 &&
        ship.writes==3 && ship.bounty==44 && ship.home==42 && ship.escorts==4 &&
        ship.wrongTask==0 && job.callbacksOnOwner==0 && dispatch.requests==6;
    printf("OOLITE_JS_FRAME_BUDGET bound=%d post=%ld completed=%lu requests=%lu writes=%ld wrong_task=%ld on_owner=%ld sum=%ld idle=%d empty_budget=%ld zero_budget=%ld processed=%d status=%s\n",
        bound,(long)post,(unsigned long)dispatch.completions,(unsigned long)dispatch.requests,
        (long)ship.writes,(long)ship.wrongTask,(long)job.callbacksOnOwner,
        (long)dispatch.result,empty,(long)emptyBudget,(long)invalidBudget,processed,ok?"PASS":"FAIL");fflush(stdout);
    OOGFOSShipInvalidate(&handle);
    OOGFOSJSClose(&js);
    OOGFOSGameDispatchClose(&dispatch);
    return ok?0:2;
}
