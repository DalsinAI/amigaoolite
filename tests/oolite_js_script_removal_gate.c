/* Unmodified Oolite default ship JavaScript -> Exec game task -> real ObjC
 * method invocation (test fixture, not original full ShipEntity/Universe). */
#include "OOGFOSJavaScript.h"
#include "OOGFOSShipEntityBackend.h"
#include "OOGFOSShipEvents.h"
#include "OOGFOSScriptObjects.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
extern int OOGFOSTestObjCRecordLegacy(void *ship,int count);
extern int OOGFOSTestObjCGetLegacyCounts(void *ship,long *calls,long *actions);
struct ScriptJob {struct OOGFOSGameDispatch *d;const char *source;LONG length,rc,result,finished,called,missing;int mode;};
static int onGameLegacy(void *owner,ULONG token,unsigned action,LONG count,LONG *result)
{
 struct OOGFOSShipEntityRegistry *registry=(struct OOGFOSShipEntityRegistry *)owner;
 void *ship=OOGFOSShipTokensLookup(&registry->tokens,token);
 if(action!=OOGFOS_SHIP_ACTION_LEGACY || !ship || !result || count<0)return 0;
 if(!OOGFOSTestObjCRecordLegacy(ship,(int)count))return 0;
 *result=count;
 return 1;
}
static LONG executeScript(JSTask task,APTR userdata)
{
 struct ScriptJob *j=(struct ScriptJob *)userdata;
 JSValue out=NULL;LONG value=-1,called=0;
 j->called=0;j->missing=-1;
 if(j->mode==2){
   j->rc=OOGFOSScriptObjectDrop(task,(CONST_STRPTR)"scriptA");
   if(j->rc==JSERR_OK)j->rc=OOGFOSScriptObjectEvent(task,
       (CONST_STRPTR)"scriptA",(CONST_STRPTR)"shipDied",0,NULL,&j->missing);
   if(j->rc==JSERR_OK)j->rc=OOGFOSScriptObjectEvent(task,
       (CONST_STRPTR)"scriptB",(CONST_STRPTR)"shipDied",0,NULL,&j->called);
   if(j->rc==JSERR_OK)j->rc=JS_Evaluate(task,(CONST_STRPTR)
       "(function(){var dropped=typeof scriptA==='undefined',stale=false;try{var b=shipA.bounty;}catch(e){stale=true;}return dropped&&stale&&scriptB.ship.bounty===10 ? 1 : 0;})()",
       -1,(CONST_STRPTR)"oolite-ship-removed-check.js",&out);
   if(j->rc==JSERR_OK)j->rc=JS_ToLong(task,out,&value);
   if(out)JS_Free(task,out);
   j->result=value;j->finished=OOGFOSGameDispatchDone(j->d,j->rc,value);
   return j->rc;
 }
 j->rc=JS_Evaluate(task,(CONST_STRPTR)
   "this.system={mainStation:{}};",
   -1,(CONST_STRPTR)"oolite-system-setup.js",NULL);
 if(j->rc==JSERR_OK)j->rc=OOGFOSScriptObjectCreate(task,
   (CONST_STRPTR)"scriptA",(CONST_STRPTR)"shipA");
 if(j->rc==JSERR_OK)j->rc=OOGFOSScriptObjectCreate(task,
   (CONST_STRPTR)"scriptB",(CONST_STRPTR)"shipB");
 if(j->rc==JSERR_OK)j->rc=JS_Evaluate(task,(CONST_STRPTR)
   "scriptA.legacy_setupActions=['initial','spawn'];scriptA.legacy_launchActions=['launch'];scriptA.legacy_deathActions=['death'];scriptB.legacy_setupActions=['initB'];scriptB.legacy_launchActions=['launchB','escortB'];scriptB.legacy_deathActions=['deathB','secondB'];",
   -1,(CONST_STRPTR)"oolite-legacy-action-data.js",NULL);
 if(j->rc==JSERR_OK)j->rc=OOGFOSScriptObjectLoad(task,
   (CONST_STRPTR)"scriptA",(CONST_STRPTR)j->source,j->length,
   (CONST_STRPTR)"oolite-default-ship-A.js");
 if(j->rc==JSERR_OK)j->rc=OOGFOSScriptObjectLoad(task,
   (CONST_STRPTR)"scriptB",(CONST_STRPTR)j->source,j->length,
   (CONST_STRPTR)"oolite-default-ship-B.js");
 if(j->rc==JSERR_OK){
    j->rc=OOGFOSScriptObjectEvent(task,(CONST_STRPTR)"scriptA",
           (CONST_STRPTR)"shipSpawned",0,NULL,&called);j->called+=called;
 }
 if(j->rc==JSERR_OK){
    j->rc=OOGFOSScriptObjectEvent(task,(CONST_STRPTR)"scriptB",
           (CONST_STRPTR)"shipSpawned",0,NULL,&called);j->called+=called;
 }
 if(j->rc==JSERR_OK){
    j->rc=OOGFOSScriptObjectEvent(task,(CONST_STRPTR)"scriptA",
           (CONST_STRPTR)"shipDied",0,NULL,&called);j->called+=called;
 }
 if(j->rc==JSERR_OK){
    j->rc=OOGFOSScriptObjectEvent(task,(CONST_STRPTR)"scriptB",
           (CONST_STRPTR)"shipDied",0,NULL,&called);j->called+=called;
 }
 if(j->rc==JSERR_OK)j->rc=OOGFOSScriptObjectEvent(task,
    (CONST_STRPTR)"scriptA",(CONST_STRPTR)"shipSpawned",0,NULL,&j->missing);
 if(j->rc==JSERR_OK)j->rc=JS_Evaluate(task,(CONST_STRPTR)
    "typeof scriptA.shipSpawned==='undefined' && typeof scriptB.shipSpawned==='undefined' && scriptA.ship.bounty===10 && scriptB.ship.bounty===10 && scriptA.name==='oolite-default-ship-script' && scriptB.name==='oolite-default-ship-script' ? 1 : 0",
    -1,(CONST_STRPTR)"oolite-shared-engine-check.js",&out);
 if(j->rc==JSERR_OK)j->rc=JS_ToLong(task,out,&value);
 if(out)JS_Free(task,out);
 j->result=value;
 j->finished=OOGFOSGameDispatchDone(j->d,j->rc,value);
 return j->rc;
}
static LONG joinJob(JSTask task,APTR data){(void)task;(void)data;return JSERR_OK;}
static char *readOriginal(const char *path,long *length)
{
 FILE *f=fopen(path,"rb");char *buf=NULL;long n=-1;
 if(!f)return NULL;
 if(fseek(f,0,SEEK_END)==0)n=ftell(f);
 if(n>0&&n<262144&&fseek(f,0,SEEK_SET)==0){
    buf=(char *)malloc((size_t)n+1);
    if(!buf || fread(buf,1,(size_t)n,f)!=(size_t)n){free(buf);buf=NULL;}
    else {buf[n]=0;*length=n;}
 }
 fclose(f);return buf;
}
int main(void)
{
 const char *path="SYS:Oolite/Scripts/oolite-default-ship-script.js";
 struct OOGFOSShipEntityRegistry registry;
 struct OOGFOSGameDispatch dispatcher;
 struct OOGFOSGameShipProxy pa,pb;
 struct OOGFOSShipHandle ha,hb;
 struct OOGFOSJS js;
 struct ScriptJob job;
 void *shipA,*shipB;
 ULONG tokenA,tokenB;long bytes=0,callsA=-1,callsB=-1,actionsA=-1,actionsB=-1;
 char *script=readOriginal(path,&bytes);
 LONG post=-1,joined=-1,secondPost=-1,secondJoin=-1,firstResult=-1;
 int droppedA=0,secondCalled=0,secondMissing=-1;
 int boundA=0,boundB=0,ok=0,closed=0,ret;
 if(!script){puts("OOLITE_SHIP_SCRIPT_REMOVAL asset=UNAVAILABLE");return 77;}
 OOGFOSShipEntityRegistryInit(&registry);
 shipA=OOGFOSTestObjCShipNew();shipB=OOGFOSTestObjCShipNew();
 if(!shipA || !shipB)return 75;
 tokenA=OOGFOSShipEntityRegister(&registry,shipA);
 tokenB=OOGFOSShipEntityRegister(&registry,shipB);
 if(!tokenA || !tokenB || tokenA==tokenB ||
    !OOGFOSGameDispatchInitRouted(&dispatcher,&registry,OOGFOSShipEntityGameBackend()))return 75;
 if(!OOGFOSGameDispatchSetAction(&dispatcher,onGameLegacy))return 75;
 if(!OOGFOSJSOpen(&js)){OOGFOSGameDispatchClose(&dispatcher);return 77;}
 pa.dispatch=&dispatcher;pa.token=tokenA;pb.dispatch=&dispatcher;pb.token=tokenB;
 ha.ship=&pa;ha.backend=OOGFOSGameShipProxyBackend();ha.valid=1;
 hb.ship=&pb;hb.backend=OOGFOSGameShipProxyBackend();hb.valid=1;
 boundA=OOGFOSJSExposeHostClass(&js,OOGFOSShipClassWithEvents(),&ha,(CONST_STRPTR)"shipA");
 boundB=OOGFOSJSExposeHostClass(&js,OOGFOSShipClassWithEvents(),&hb,(CONST_STRPTR)"shipB");
 job=(struct ScriptJob){&dispatcher,script,(LONG)bytes,-1,-1,-1,-1,-1,1};
 if(boundA&&boundB)post=JS_Post(js.task,executeScript,&job,0);
 if(post==JSERR_OK){
   do{ret=OOGFOSGameDispatchServe(&dispatcher);}while(ret==0);
   joined=JS_Post(js.task,joinJob,NULL,1);
 }
 firstResult=job.result;
 if(joined==JSERR_OK){
   droppedA=OOGFOSShipEntityUnregister(&registry,tokenA);
   job.mode=2;
   secondPost=JS_Post(js.task,executeScript,&job,0);
   if(secondPost==JSERR_OK){
      while(dispatcher.completions<2){ret=OOGFOSGameDispatchServe(&dispatcher);if(ret<0)break;}
      if(dispatcher.completions==2)secondJoin=JS_Post(js.task,joinJob,NULL,1);
   }
   secondCalled=job.called;secondMissing=(int)job.missing;
 }
 if(job.rc!=JSERR_OK){char err[256]={0};OOGFOSJSErrorText(&js,(STRPTR)err,sizeof(err));printf("OOLITE_SINGLE_TASK_ERROR %s\n",err);}
 OOGFOSTestObjCGetLegacyCounts(shipA,&callsA,&actionsA);
 OOGFOSTestObjCGetLegacyCounts(shipB,&callsB,&actionsB);
 ok=boundA&&boundB&&post==JSERR_OK&&joined==JSERR_OK&&droppedA&&
    secondPost==JSERR_OK&&secondJoin==JSERR_OK&&job.rc==JSERR_OK&&
    firstResult==1&&job.result==1&&job.finished==1&&
    dispatcher.completions==2&&dispatcher.requests==11&&
    callsA==3&&callsB==4&&actionsA==4&&actionsB==7&&
    secondCalled==1&&secondMissing==0;
 OOGFOSShipInvalidate(&ha);OOGFOSShipInvalidate(&hb);
 OOGFOSJSClose(&js);
 OOGFOSShipEntityUnregister(&registry,tokenB);
 closed=OOGFOSGameDispatchTryClose(&dispatcher);
 printf("OOLITE_SHIP_SCRIPT_REMOVAL bytes=%ld scripts=2 js_tasks=1 calls=%ld,%ld actions=%ld,%ld rpc=%lu done=%lu first=%ld second=%ld called=%d missing=%d droppedA=%d closed=%d status=%s\n",
    bytes,callsA,callsB,actionsA,actionsB,(unsigned long)dispatcher.requests,
    (unsigned long)dispatcher.completions,(long)firstResult,(long)job.result,secondCalled,secondMissing,droppedA,closed,ok&&closed?"PASS":"FAIL");fflush(stdout);
 OOGFOSTestObjCShipFree(shipB);OOGFOSTestObjCShipFree(shipA);
 free(script);
 return ok&&closed?0:2;
}
