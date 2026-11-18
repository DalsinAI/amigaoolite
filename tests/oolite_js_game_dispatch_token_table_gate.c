/* Per-ship tokens resolve on Oolite's game task, never in JS task. */
#include "OOGFOSJavaScript.h"
#include "OOGFOSGameDispatch.h"
#include "OOGFOSShipTokens.h"
#include <proto/exec.h>
#include <proto/javascript.h>
#include <stdio.h>
struct Ship { LONG bounty,home,escorts; };
struct Registry { struct OOGFOSShipTokenTable tokens; struct Ship ships[3]; struct Task *gameTask; LONG wrongTask,writes,misses; };
static struct Ship *lookup(struct Registry *r,ULONG token) {
 struct Ship *s; if(FindTask(NULL)!=r->gameTask){r->wrongTask++;return NULL;}
 s=(struct Ship *)OOGFOSShipTokensLookup(&r->tokens,token);
 if(s)return s;
 r->misses++;return NULL;
}
static int readShip(void *context,ULONG token,unsigned key,LONG *value){
 struct Ship *s=lookup((struct Registry *)context,token);
 if(!s||!value)return 0;
 switch(key){case OOGFOS_SHIP_BOUNTY:*value=s->bounty;return 1;
 case OOGFOS_SHIP_HOME_SYSTEM:*value=s->home;return 1;
 case OOGFOS_SHIP_MAX_ESCORTS:*value=s->escorts;return 1;
 default:return 0;}
}
static int writeShip(void *context,ULONG token,unsigned key,LONG value){
 struct Registry *r=(struct Registry *)context;
 struct Ship *s=lookup(r,token);
 if(!s)return 0;
 switch(key){case OOGFOS_SHIP_BOUNTY:if(value<0||value>100000)return 0;s->bounty=value;break;
 case OOGFOS_SHIP_HOME_SYSTEM:if(value<0||value>255)return 0;s->home=value;break;
 case OOGFOS_SHIP_MAX_ESCORTS:if(value<0||value>16)return 0;s->escorts=value;break;
 default:return 0;}r->writes++;return 1;
}
struct Job { struct OOGFOSGameDispatch *dispatch;const char *source; LONG jobReturn;};
static LONG scriptJob(JSTask task,APTR data){
 struct Job *j=(struct Job *)data;JSValue result=NULL;LONG out=-1;
 LONG rc=JS_Evaluate(task,(CONST_STRPTR)j->source,-1,(CONST_STRPTR)"oolite-route.js",&result);
 if(rc==JSERR_OK)rc=JS_ToLong(task,result,&out);
 if(result)JS_Free(task,result);
 j->jobReturn=OOGFOSGameDispatchDone(j->dispatch,rc,out);
 return rc;
}
static LONG joinJob(JSTask task,APTR data){(void)task;(void)data;return JSERR_OK;}
static int pumpUntilDone(struct OOGFOSGameDispatch *d,ULONG expected){
 while(d->completions<expected){if(OOGFOSGameDispatchServe(d)<0)return 0;}return 1;
}
int main(void){
 static const struct OOGFOSGameRouteBackend backend={readShip,writeShip};
 struct Registry r={0};
 struct OOGFOSGameDispatch d;
 struct OOGFOSGameShipProxy a,b,c;
 struct OOGFOSShipHandle ha,hb,hc;
 struct OOGFOSJS js;
 struct Job job;
 LONG post1=-1,post2=-1,first=-1,second=-1,j1=-1,j2=-1;
 int readyA=0,readyB=0,readyC=0,removed=0,done1=0,done2=0,pass=0,closed=0;
 ULONG oldToken=0,newToken=0;
 r.gameTask=FindTask(NULL);
 OOGFOSShipTokensInit(&r.tokens);
 r.ships[0]=(struct Ship){10,7,2};
 r.ships[1]=(struct Ship){20,8,3};
 r.ships[2]=(struct Ship){30,9,1};
 if(!OOGFOSGameDispatchInitRouted(&d,&r,&backend))return 75;
 if(!OOGFOSJSOpen(&js)){OOGFOSGameDispatchClose(&d);return 77;}
 a.dispatch=&d;a.token=OOGFOSShipTokensAdd(&r.tokens,&r.ships[0]);
 b.dispatch=&d;b.token=OOGFOSShipTokensAdd(&r.tokens,&r.ships[1]);
 oldToken=a.token;
 ha.ship=&a;ha.backend=OOGFOSGameShipProxyBackend();ha.valid=1;
 hb.ship=&b;hb.backend=OOGFOSGameShipProxyBackend();hb.valid=1;
 readyA=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&ha,(CONST_STRPTR)"shipA");
 readyB=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&hb,(CONST_STRPTR)"shipB");
 job.dispatch=&d;job.source="shipA.bounty=44;shipB.homeSystem=42;shipA.bounty+shipB.homeSystem+shipA.maxEscorts+shipB.maxEscorts;";job.jobReturn=-1;
 if(readyA&&readyB){post1=JS_Post(js.task,scriptJob,&job,0);if(post1==JSERR_OK)done1=pumpUntilDone(&d,1);}
 if(done1){j1=JS_Post(js.task,joinJob,NULL,1);first=d.result;}
 /* Invalidate the first ship *on the game owner*, keeping its proxy/token
  * exposed in JS to prove stale identity is refused by the owner backend. */
 removed=OOGFOSShipTokensRemove(&r.tokens,oldToken);
 r.tokens.nextSlot=0; /* deliberately reuse vacated slot to prove generations */
 c.dispatch=&d;c.token=OOGFOSShipTokensAdd(&r.tokens,&r.ships[2]);
 newToken=c.token;
 hc.ship=&c;hc.backend=OOGFOSGameShipProxyBackend();hc.valid=1;
 readyC=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&hc,(CONST_STRPTR)"shipC");
 job.source="(function(){var x=0;try{x=shipA.bounty;}catch(e){x=1;}return x+shipB.homeSystem+shipC.bounty;})()";
 job.jobReturn=-1;
 if(j1==JSERR_OK){post2=JS_Post(js.task,scriptJob,&job,0);if(post2==JSERR_OK)done2=pumpUntilDone(&d,2);}
 if(done2){j2=JS_Post(js.task,joinJob,NULL,1);second=d.result;}
 pass=readyA&&readyB&&readyC&&removed&&oldToken&&newToken>oldToken&&done1&&done2&&post1==JSERR_OK&&post2==JSERR_OK&&
      j1==JSERR_OK&&j2==JSERR_OK&&first==91&&second==73&&
      d.requests==9&&d.completions==2&&r.ships[0].bounty==44&&
      r.ships[1].home==42&&r.writes==2&&r.wrongTask==0&&r.misses==1;
 closed=OOGFOSGameDispatchTryClose(&d);
 pass=pass&&closed&&d.inFlight==0;
 printf("OOLITE_GAME_SHIP_TOKENS bound=%d,%d,%d jobs=%ld,%ld complete=%lu requests=%lu writes=%ld stale=%ld wrong_task=%ld first=%ld second=%ld closed=%d old=%lu new=%lu status=%s\n",readyA,readyB,readyC,(long)post1,(long)post2,(unsigned long)d.completions,(unsigned long)d.requests,(long)r.writes,(long)r.misses,(long)r.wrongTask,(long)first,(long)second,closed,(unsigned long)oldToken,(unsigned long)newToken,pass?"PASS":"FAIL");fflush(stdout);
 OOGFOSShipInvalidate(&ha);OOGFOSShipInvalidate(&hb);OOGFOSShipInvalidate(&hc);OOGFOSJSClose(&js);OOGFOSGameDispatchClose(&d);
 return pass?0:2;
}
