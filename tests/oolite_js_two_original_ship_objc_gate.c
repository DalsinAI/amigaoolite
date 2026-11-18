/* Unmodified Oolite default ship JavaScript -> Exec game task -> real ObjC
 * method invocation (test fixture, not original full ShipEntity/Universe). */
#include "OOGFOSJavaScript.h"
#include "OOGFOSShipEntityBackend.h"
#include "OOGFOSShipEvents.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
extern int OOGFOSTestObjCRecordLegacy(void *ship,int count);
extern int OOGFOSTestObjCGetLegacyCounts(void *ship,long *calls,long *actions);
struct ScriptJob {struct OOGFOSGameDispatch *d;const char *source,*setup;LONG rc,result,finished;};
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
 struct ScriptJob *j=(struct ScriptJob *)userdata;JSValue out=NULL;LONG value=-1;
 j->rc=JS_Evaluate(task,(CONST_STRPTR)
   j->setup,-1,(CONST_STRPTR)"oolite-event-setup.js",NULL);
 if(j->rc==JSERR_OK)j->rc=JS_Evaluate(task,(CONST_STRPTR)j->source,-1,
   (CONST_STRPTR)"oolite-default-ship-script.js",NULL);
 if(j->rc==JSERR_OK)j->rc=JS_Evaluate(task,(CONST_STRPTR)
   "this.shipSpawned();this.shipDied();typeof this.shipSpawned === 'undefined' && ship.bounty===10 ? 1 : 0",
   -1,(CONST_STRPTR)"oolite-event-trigger.js",&out);
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
 const char *setupA="this.system={mainStation:{}};this.legacy_setupActions=['initial','spawn'];this.legacy_launchActions=['launch'];this.legacy_deathActions=['death'];";
 const char *setupB="this.system={mainStation:{}};this.legacy_setupActions=['initB'];this.legacy_launchActions=['launchB','escortB'];this.legacy_deathActions=['deathB','secondB'];";
 struct OOGFOSShipEntityRegistry registry;
 struct OOGFOSGameDispatch dispatcher;
 struct OOGFOSGameShipProxy pa,pb;
 struct OOGFOSShipHandle ha,hb;
 struct OOGFOSJS ja,jb;
 struct ScriptJob jobA,jobB;
 void *shipA,*shipB;
 ULONG tokenA,tokenB;long bytes=0,ca=-1,cb=-1,aa=-1,ab=-1;
 char *script=readOriginal(path,&bytes);
 LONG postA=-1,postB=-1,joinedA=-1,joinedB=-1;
 int wrappedA=0,wrappedB=0,ok=0,closed=0,ret;
 if(!script){puts("OOLITE_TWO_ORIGINAL_JS asset=UNAVAILABLE");return 77;}
 OOGFOSShipEntityRegistryInit(&registry);
 shipA=OOGFOSTestObjCShipNew();shipB=OOGFOSTestObjCShipNew();
 if(!shipA || !shipB){free(script);return 75;}
 tokenA=OOGFOSShipEntityRegister(&registry,shipA);
 tokenB=OOGFOSShipEntityRegister(&registry,shipB);
 if(!tokenA || !tokenB || tokenA==tokenB ||
    !OOGFOSGameDispatchInitRouted(&dispatcher,&registry,OOGFOSShipEntityGameBackend()))return 75;
 if(!OOGFOSGameDispatchSetAction(&dispatcher,onGameLegacy))return 75;
 if(!OOGFOSJSOpen(&ja)){OOGFOSGameDispatchClose(&dispatcher);return 77;}
 if(!OOGFOSJSOpen(&jb)){OOGFOSJSClose(&ja);OOGFOSGameDispatchClose(&dispatcher);return 77;}
 pa.dispatch=&dispatcher;pa.token=tokenA;
 pb.dispatch=&dispatcher;pb.token=tokenB;
 ha.ship=&pa;ha.backend=OOGFOSGameShipProxyBackend();ha.valid=1;
 hb.ship=&pb;hb.backend=OOGFOSGameShipProxyBackend();hb.valid=1;
 wrappedA=OOGFOSJSExposeHostClass(&ja,OOGFOSShipClassWithEvents(),&ha,(CONST_STRPTR)"ship");
 wrappedB=OOGFOSJSExposeHostClass(&jb,OOGFOSShipClassWithEvents(),&hb,(CONST_STRPTR)"ship");
 jobA=(struct ScriptJob){&dispatcher,script,setupA,-1,-1,-1};
 jobB=(struct ScriptJob){&dispatcher,script,setupB,-1,-1,-1};
 if(wrappedA&&wrappedB){
   postA=JS_Post(ja.task,executeScript,&jobA,0);
   postB=JS_Post(jb.task,executeScript,&jobB,0);
 }
 if(postA==JSERR_OK && postB==JSERR_OK){
   while(dispatcher.completions<2){ret=OOGFOSGameDispatchServe(&dispatcher);if(ret<0)break;}
   if(dispatcher.completions==2){
     joinedA=JS_Post(ja.task,joinJob,NULL,1);
     joinedB=JS_Post(jb.task,joinJob,NULL,1);
   }
 }
 OOGFOSTestObjCGetLegacyCounts(shipA,&ca,&aa);
 OOGFOSTestObjCGetLegacyCounts(shipB,&cb,&ab);
 ok=wrappedA&&wrappedB&&postA==JSERR_OK&&postB==JSERR_OK&&
    joinedA==JSERR_OK&&joinedB==JSERR_OK&&jobA.rc==JSERR_OK&&jobB.rc==JSERR_OK&&
    jobA.result==1&&jobB.result==1&&jobA.finished==1&&jobB.finished==1&&
    ca==3&&aa==4&&cb==3&&ab==5&&dispatcher.completions==2&&dispatcher.requests==8;
 OOGFOSShipInvalidate(&ha);OOGFOSShipInvalidate(&hb);
 OOGFOSJSClose(&jb);OOGFOSJSClose(&ja);
 OOGFOSShipEntityUnregister(&registry,tokenB);
 OOGFOSShipEntityUnregister(&registry,tokenA);
 closed=OOGFOSGameDispatchTryClose(&dispatcher);
 printf("OOLITE_TWO_ORIGINAL_JS bytes=%ld jobs=%ld,%ld done=%lu rpc=%lu calls=%ld,%ld actions=%ld,%ld results=%ld,%ld closed=%d status=%s\n",
    bytes,(long)postA,(long)postB,(unsigned long)dispatcher.completions,(unsigned long)dispatcher.requests,
    ca,cb,aa,ab,(long)jobA.result,(long)jobB.result,closed,ok&&closed?"PASS":"FAIL");fflush(stdout);
 OOGFOSTestObjCShipFree(shipB);OOGFOSTestObjCShipFree(shipA);free(script);
 return ok&&closed?0:2;
}
