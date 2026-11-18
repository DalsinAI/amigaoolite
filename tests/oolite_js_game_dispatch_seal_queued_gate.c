/* Queue before seal; dispatch Serve must reject rather than executing it. */
#include "OOGFOSJavaScript.h"
#include "OOGFOSGameDispatch.h"
#include <proto/javascript.h>
#include <proto/exec.h>
#include <stdio.h>
struct TestShip { LONG value,reads,writes,wrong;struct Task *owner; };
static int readShip(void *ptr,unsigned p,LONG *v){struct TestShip *s=(struct TestShip*)ptr;
 if(FindTask(NULL)!=s->owner){s->wrong++;return 0;}s->reads++;if(p!=OOGFOS_SHIP_BOUNTY)return 0;*v=s->value;return 1;}
static int writeShip(void *ptr,unsigned p,LONG v){struct TestShip *s=(struct TestShip*)ptr;
 if(FindTask(NULL)!=s->owner){s->wrong++;return 0;}s->writes++;if(p!=OOGFOS_SHIP_BOUNTY)return 0;s->value=v;return 1;}
struct Job{struct OOGFOSGameDispatch *d;LONG callbackDone;};
static LONG doJS(JSTask task,APTR ptr){struct Job *job=(struct Job*)ptr;JSValue r=NULL;LONG value=-1;
 LONG rc=JS_Evaluate(task,(CONST_STRPTR)"(function(){try {return ship.bounty;}catch(e){return 1;}})()",-1,(CONST_STRPTR)"oolite-sealed-queued.js",&r);
 if(rc==JSERR_OK) rc=JS_ToLong(task,r,&value);
 if(r) JS_Free(task,r);
 job->callbackDone=OOGFOSGameDispatchDone(job->d,rc,value);return rc;}
static LONG barrier(JSTask t,APTR ptr){(void)t;(void)ptr;return 0;}
int main(void){static const struct OOGFOSShipBackend api={readShip,writeShip};
 struct TestShip ship={10,0,0,0,NULL};struct OOGFOSGameDispatch d;
 struct OOGFOSShipHandle handle;struct OOGFOSJS js;struct Job job;
 LONG posted=-1,joined=-1;int bound=0,sealed=0,first=-1,second=-1,closed=0,passed=0;
 ship.owner=FindTask(NULL);
 if(!OOGFOSGameDispatchInit(&d,&ship,&api))return 75;
 if(!OOGFOSJSOpen(&js)){OOGFOSGameDispatchClose(&d);return 77;}
 handle.ship=&d;handle.backend=OOGFOSGameDispatchBackend();handle.valid=1;
 bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,(CONST_STRPTR)"ship");
 job.d=&d;job.callbackDone=-1;
 if(bound)posted=JS_Post(js.task,doJS,&job,0);
 if(posted==JSERR_OK){
   /* Guarantee an RPC is queued, then seal before the owner serves it. */
   WaitPort(d.gamePort);
   sealed=OOGFOSGameDispatchSeal(&d);
   first=OOGFOSGameDispatchServe(&d);
   second=OOGFOSGameDispatchServe(&d);
   joined=JS_Post(js.task,barrier,NULL,1);
 }
 passed=bound&&posted==JSERR_OK&&sealed&&first==0&&second==1&&joined==JSERR_OK&&
        job.callbackDone==1&&d.completions==1&&d.resultCode==JSERR_OK&&d.result==1&&
        d.cancelled==1&&d.requests==0&&ship.reads==0&&ship.writes==0&&ship.value==10&&ship.wrong==0;
 closed=OOGFOSGameDispatchTryClose(&d);
 passed=passed&&closed&&d.inFlight==0;
 printf("OOLITE_SEAL_QUEUED sealed=%d rejected=%lu requests=%lu owner_reads=%ld writes=%ld result=%ld joined=%ld close=%d status=%s\n",
 sealed,(unsigned long)d.cancelled,(unsigned long)d.requests,(long)ship.reads,(long)ship.writes,
 (long)d.result,(long)joined,closed,passed?"PASS":"FAIL");fflush(stdout);
 OOGFOSShipInvalidate(&handle);OOGFOSJSClose(&js);OOGFOSGameDispatchClose(&d);
 return passed?0:2;
}
