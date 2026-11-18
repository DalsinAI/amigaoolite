/* Original Oolite ship rules over real GFOS JS -> Exec -> ObjC selectors.
 * Controlled ObjC mock has test-only knobs for player, escort group, bounty64. */
#include "OOGFOSJavaScript.h"
#include "OOGFOSShipEntityBackend.h"
#include <proto/javascript.h>
#include <stdint.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
extern int OOGFOSTestObjCShipValues(void *ship,int *b,int *h,int *e,int *r);
extern void OOGFOSTestObjCShipSetPlayer(void *ship,int value);
extern void OOGFOSTestObjCShipSetGroupSize(void *ship,unsigned value);
extern void OOGFOSTestObjCShipSetBounty64(void *ship,uint64_t value);
struct JSJob {struct OOGFOSGameDispatch *dispatcher;const char *source;LONG rc,result,finished;};
static LONG evalJob(JSTask task,APTR data){
 struct JSJob *j=(struct JSJob *)data;JSValue result=NULL;LONG v=-1;
 j->rc=JS_Evaluate(task,(CONST_STRPTR)j->source,-1,(CONST_STRPTR)"oolite-guard.js",&result);
 if(j->rc==JSERR_OK)j->rc=JS_ToLong(task,result,&v);
 if(result)JS_Free(task,result);
 j->result=v;
 j->finished=OOGFOSGameDispatchDone(j->dispatcher,j->rc,v);
 return j->rc;
}
static LONG joinJob(JSTask task,APTR data){(void)task;(void)data;return JSERR_OK;}
static int execute(struct OOGFOSJS *js,struct OOGFOSGameDispatch *d,
                   struct JSJob *j,const char *source,LONG expected,ULONG count){
 LONG rc;int served;
 j->source=source;j->rc=-1;j->result=-1;j->finished=-1;
 rc=JS_Post(js->task,evalJob,j,0);
 if(rc!=JSERR_OK)return 0;
 while(d->completions<count){served=OOGFOSGameDispatchServe(d);if(served<0)return 0;}
 if(JS_Post(js->task,joinJob,NULL,1)!=JSERR_OK)return 0;
 return j->rc==JSERR_OK && j->result==expected && j->finished==1;
}
int main(void){
 struct OOGFOSShipEntityRegistry registry;
 struct OOGFOSGameDispatch dispatch;
 struct OOGFOSGameShipProxy proxy;
 struct OOGFOSShipHandle handle;
 struct OOGFOSJS js;
 struct JSJob work;
 ULONG token;void *ship;int bound=0,guardPlayer=0,guardGroup=0,guardLarge=0,closed=0,ok=0;
 int bounty=0,home=0,escorts=0,reason=0;
 OOGFOSShipEntityRegistryInit(&registry);
 ship=OOGFOSTestObjCShipNew();if(!ship)return 75;
 token=OOGFOSShipEntityRegister(&registry,ship);
 if(!token || !OOGFOSGameDispatchInitRouted(&dispatch,&registry,OOGFOSShipEntityGameBackend()))return 75;
 if(!OOGFOSJSOpen(&js)){OOGFOSGameDispatchClose(&dispatch);return 77;}
 proxy.dispatch=&dispatch;proxy.token=token;
 handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
 bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,(CONST_STRPTR)"ship");
 work.dispatcher=&dispatch;
 if(bound){
  OOGFOSTestObjCShipSetPlayer(ship,1);
  guardPlayer=execute(&js,&dispatch,&work,
    "(function(){try{ship.maxEscorts=6;return 0;}catch(e){return 1;}})()",1,1);
  OOGFOSTestObjCShipSetPlayer(ship,0);
  OOGFOSTestObjCShipSetGroupSize(ship,5);
  guardGroup=execute(&js,&dispatch,&work,
    "(function(){try{ship.maxEscorts=3;return 0;}catch(e){return 1;}})()",1,2);
  OOGFOSTestObjCShipSetBounty64(ship,2147483648ULL);
  guardLarge=execute(&js,&dispatch,&work,
    "(function(){try{var b=ship.bounty;return 0;}catch(e){return 1;}})()",1,3);
 }
 OOGFOSTestObjCShipValues(ship,&bounty,&home,&escorts,&reason);
 ok=bound&&guardPlayer&&guardGroup&&guardLarge&&dispatch.requests==3&&
    dispatch.completions==3&&escorts==2&&home==7;
 OOGFOSShipInvalidate(&handle);OOGFOSJSClose(&js);
 OOGFOSShipEntityUnregister(&registry,token);
 closed=OOGFOSGameDispatchTryClose(&dispatch);ok=ok&&closed;
 printf("OOLITE_JS_OBJC_GUARDS player=%d group=%d bounty64=%d escorts=%d requests=%lu complete=%lu closed=%d status=%s\n",
     guardPlayer,guardGroup,guardLarge,escorts,(unsigned long)dispatch.requests,
     (unsigned long)dispatch.completions,closed,ok?"PASS":"FAIL");fflush(stdout);
 OOGFOSTestObjCShipFree(ship);
 return ok?0:2;
}
