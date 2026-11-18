/* Two independent GFOS JavaScript tasks issue concurrent Exec MsgPort requests
 * to one game owner. Qualified against canonical javascript.library on AC090. */
#include "OOGFOSJavaScript.h"
#include "OOGFOSGameDispatch.h"
#include <proto/javascript.h>
#include <proto/exec.h>
#include <stdio.h>
struct Ship { LONG bounty,home,escorts,writes,wrongTask; struct Task *gameTask; };
static int getShip(void *p,unsigned k,LONG *v) {
 struct Ship *s=(struct Ship *)p;
 if(FindTask(NULL)!=s->gameTask){s->wrongTask++;return 0;}
 switch(k){case OOGFOS_SHIP_BOUNTY:*v=s->bounty;return 1;
 case OOGFOS_SHIP_HOME_SYSTEM:*v=s->home;return 1;
 case OOGFOS_SHIP_MAX_ESCORTS:*v=s->escorts;return 1;default:return 0;}
}
static int setShip(void *p,unsigned k,LONG v) {
 struct Ship *s=(struct Ship *)p;
 if(FindTask(NULL)!=s->gameTask){s->wrongTask++;return 0;}
 switch(k){case OOGFOS_SHIP_BOUNTY:if(v<0||v>100000)return 0;s->bounty=v;break;
 case OOGFOS_SHIP_HOME_SYSTEM:if(v<0||v>255)return 0;s->home=v;break;
 case OOGFOS_SHIP_MAX_ESCORTS:if(v<0||v>16)return 0;s->escorts=v;break;
 default:return 0;}s->writes++;return 1;
}
struct JSJob {struct OOGFOSGameDispatch *dispatch;const char *source;struct Task *gameTask;LONG result,rc,sameTask,doneOK;};
static LONG doJob(JSTask task,APTR data){
 struct JSJob *j=(struct JSJob *)data;JSValue r=NULL;LONG v=-1;
 j->sameTask=FindTask(NULL)==j->gameTask;
 j->rc=JS_Evaluate(task,(CONST_STRPTR)j->source,-1,(CONST_STRPTR)"oolite-two-task.js",&r);
 if(j->rc==JSERR_OK) j->rc=JS_ToLong(task,r,&v);
 if(r)JS_Free(task,r);
 j->result=v;
 j->doneOK=OOGFOSGameDispatchDone(j->dispatch,j->rc,v);
 return j->rc;
}
static LONG barrierJob(JSTask task,APTR data){(void)task;(void)data;return 0;}
int main(void){
 static const struct OOGFOSShipBackend native={getShip,setShip};
 struct Ship ship={10,7,2,0,0,NULL};
 struct OOGFOSGameDispatch d;
 struct OOGFOSShipHandle handle={NULL,NULL,1};
 struct OOGFOSJS a,b;
 struct JSJob jobA,jobB;
 LONG pa=-1,pb=-1;int readyA,readyB,served,pass=0,cycles=0;
 LONG joinedA=-1,joinedB=-1;
 ship.gameTask=FindTask(NULL);
 if(!OOGFOSGameDispatchInit(&d,&ship,&native))return 75;
 if(!OOGFOSJSOpen(&a)){OOGFOSGameDispatchClose(&d);return 77;}
 if(!OOGFOSJSOpen(&b)){OOGFOSJSClose(&a);OOGFOSGameDispatchClose(&d);return 77;}
 handle.ship=&d;handle.backend=OOGFOSGameDispatchBackend();
 readyA=OOGFOSJSExposeHostClass(&a,OOGFOSShipClass(),&handle,(CONST_STRPTR)"ship");
 readyB=OOGFOSJSExposeHostClass(&b,OOGFOSShipClass(),&handle,(CONST_STRPTR)"ship");
 jobA=(struct JSJob){&d,"ship.bounty=44;ship.bounty;",ship.gameTask,-1,-1,-1,-1};
 jobB=(struct JSJob){&d,"ship.homeSystem=42;ship.homeSystem;",ship.gameTask,-1,-1,-1,-1};
 if(readyA&&readyB){
  pa=JS_Post(a.task,doJob,&jobA,0);
  pb=JS_Post(b.task,doJob,&jobB,0);
 }
 if(pa==JSERR_OK&&pb==JSERR_OK) {
   while(d.completions<2 && cycles<20){served=OOGFOSGameDispatchServe(&d);if(served<0)break;cycles++;}
 }
 if(d.completions==2){ joinedA=JS_Post(a.task,barrierJob,NULL,1); joinedB=JS_Post(b.task,barrierJob,NULL,1); }
 pass=joinedA==JSERR_OK&&joinedB==JSERR_OK&&readyA&&readyB&&pa==JSERR_OK&&pb==JSERR_OK&&d.completions==2&&
      d.requests==4&&ship.writes==2&&ship.bounty==44&&ship.home==42&&
      ship.wrongTask==0&&jobA.sameTask==0&&jobB.sameTask==0&&
      jobA.rc==JSERR_OK&&jobB.rc==JSERR_OK&&jobA.result==44&&jobB.result==42&&
      jobA.doneOK==1&&jobB.doneOK==1;
 printf("OOLITE_GAME_TWO_JS_TASKS bound=%d,%d posts=%ld,%ld complete=%lu rpc=%lu writes=%ld wrong_task=%ld same_task=%ld,%ld values=%ld,%ld joined=%ld,%ld done=%ld,%ld status=%s\n",readyA,readyB,(long)pa,(long)pb,(unsigned long)d.completions,(unsigned long)d.requests,(long)ship.writes,(long)ship.wrongTask,(long)jobA.sameTask,(long)jobB.sameTask,(long)jobA.result,(long)jobB.result,(long)joinedA,(long)joinedB,(long)jobA.doneOK,(long)jobB.doneOK,pass?"PASS":"FAIL");fflush(stdout);
 OOGFOSShipInvalidate(&handle);OOGFOSJSClose(&b);OOGFOSJSClose(&a);OOGFOSGameDispatchClose(&d);
 return pass?0:2;
}
