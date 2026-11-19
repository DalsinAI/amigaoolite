/* Unmodified Oolite default ship JavaScript -> Exec game task -> real ObjC
 * method invocation via original OOJSScript UTF-16/JS object semantics.\n * Test fixture, not original full ShipEntity/Universe. GPL-2.0-or-later. */
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
struct ScriptJob {struct OOGFOSGameDispatch *d;const UWORD *source;LONG units,rc,result,finished;};
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
 j->rc=OOGFOSScriptObjectCreate(task,(CONST_STRPTR)"script",
                               (CONST_STRPTR)"ship");
 if(j->rc==JSERR_OK)j->rc=JS_Evaluate(task,(CONST_STRPTR)
   "this.system={mainStation:{}};script.legacy_setupActions=['initial','spawn'];script.legacy_launchActions=['launch'];script.legacy_deathActions=['death'];",
   -1,(CONST_STRPTR)"oolite-event-setup.js",NULL);
 if(j->rc==JSERR_OK)j->rc=OOGFOSScriptObjectLoadUTF16(task,
   (CONST_STRPTR)"script",j->source,j->units,
   (CONST_STRPTR)"oolite-default-ship-script.js");
 if(j->rc==JSERR_OK)j->rc=JS_Evaluate(task,(CONST_STRPTR)
   "script.shipSpawned();script.shipDied();typeof script.shipSpawned === 'undefined' && script.ship.bounty===10 ? 1 : 0",
   -1,(CONST_STRPTR)"oolite-event-trigger.js",&out);
 if(j->rc==JSERR_OK)j->rc=JS_ToLong(task,out,&value);
 if(out)JS_Free(task,out);
 j->result=value;
 j->finished=OOGFOSGameDispatchDone(j->d,j->rc,value);
 return j->rc;
}
static LONG joinJob(JSTask task,APTR data){(void)task;(void)data;return JSERR_OK;}
static UWORD *readOriginal(const char *path,long *length)
{
 FILE *f=fopen(path,"rb");UWORD *buf=NULL;long n=-1;
 if(!f)return NULL;
 if(fseek(f,0,SEEK_END)==0)n=ftell(f);
 if(n>0 && n%2==0 && n<524288 && fseek(f,0,SEEK_SET)==0){
    buf=(UWORD *)malloc((size_t)n);
    if(!buf || fread(buf,1,(size_t)n,f)!=(size_t)n){free(buf);buf=NULL;}
    else {*length=n;}
 }
 fclose(f);return buf;
}
int main(void)
{
 const char *path="SYS:Oolite/Scripts/oolite-default-ship-script.utf16be";
 struct OOGFOSShipEntityRegistry registry;
 struct OOGFOSGameDispatch dispatcher;
 struct OOGFOSGameShipProxy proxy;
 struct OOGFOSShipHandle handle;
 struct OOGFOSJS js;
 struct ScriptJob job;
 void *ship;ULONG token;long bytes=0,calls=-1,actions=-1;
 UWORD *script=readOriginal(path,&bytes);
 LONG post=-1,joined=-1;int bound=0,ok=0,ret,closed=0;
 if(!script){puts("OOLITE_ORIGINAL_UTF16_OBJC_EVENT asset=UNAVAILABLE");return 77;}
 OOGFOSShipEntityRegistryInit(&registry);
 ship=OOGFOSTestObjCShipNew();if(!ship){free(script);return 75;}
 token=OOGFOSShipEntityRegister(&registry,ship);
 if(!token || !OOGFOSGameDispatchInitRouted(&dispatcher,&registry,OOGFOSShipEntityGameBackend()))return 75;
 if(!OOGFOSGameDispatchSetAction(&dispatcher,onGameLegacy))return 75;
 if(!OOGFOSJSOpen(&js)){OOGFOSGameDispatchClose(&dispatcher);return 77;}
 proxy.dispatch=&dispatcher;proxy.token=token;
 handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
 bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClassWithEvents(),&handle,(CONST_STRPTR)"ship");
 job.d=&dispatcher;job.source=script;job.units=bytes/2;job.rc=-1;job.result=-1;job.finished=-1;
 if(bound)post=JS_Post(js.task,executeScript,&job,0);
 if(post==JSERR_OK){
   do{ret=OOGFOSGameDispatchServe(&dispatcher);}while(ret==0);
   joined=JS_Post(js.task,joinJob,NULL,1);
 }
 if(job.rc!=JSERR_OK){char err[256]={0};OOGFOSJSErrorText(&js,(STRPTR)err,sizeof(err));printf("OOLITE_ORIGINAL_OBJC_ERROR %s\n",err);}
 OOGFOSTestObjCGetLegacyCounts(ship,&calls,&actions);
 ok=bound&&post==JSERR_OK&&joined==JSERR_OK&&job.rc==JSERR_OK&&job.result==1&&job.finished==1&&
    calls==3&&actions==4&&dispatcher.requests==4&&dispatcher.completions==1;
 OOGFOSShipInvalidate(&handle);OOGFOSJSClose(&js);OOGFOSShipEntityUnregister(&registry,token);
 closed=OOGFOSGameDispatchTryClose(&dispatcher);
 printf("OOLITE_ORIGINAL_UTF16_OBJC_EVENT bytes=%ld bound=%d methods=%ld actions=%ld rpc=%lu completed=%lu result=%ld closed=%d status=%s\n",
   bytes,bound,calls,actions,(unsigned long)dispatcher.requests,(unsigned long)dispatcher.completions,
   (long)job.result,closed,ok&&closed?"PASS":"FAIL");fflush(stdout);
 OOGFOSTestObjCShipFree(ship);free(script);
 return ok&&closed?0:2;
}
