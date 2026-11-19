/* Original Oolite default ship script: synchronous events to ObjC game task.
 * Includes one-shot absent event, deliberate JS exception, and recovery.
 * Native AC090 68040 fixture test; GPL-2.0-or-later.
 */
#include "OOGFOSScriptEventInvoke.h"
#include "OOGFOSScriptObjects.h"
#include "OOGFOSShipEntityBackend.h"
#include "OOGFOSShipEvents.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <stdlib.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
extern int OOGFOSTestObjCRecordLegacy(void *ship,int count);
extern int OOGFOSTestObjCGetLegacyCounts(void *ship,long *calls,long *actions);
struct Setup {const UWORD *source;LONG units;LONG rc;};
static LONG setupOnJS(JSTask task,APTR data)
{
    struct Setup *s=(struct Setup *)data;
    s->rc=OOGFOSScriptObjectCreate(task,"script","ship");
    if(s->rc==JSERR_OK)s->rc=JS_Evaluate(task,
      "this.system={mainStation:{}};script.legacy_setupActions=['initial','spawn'];"
      "script.legacy_launchActions=['launch'];script.legacy_deathActions=['death'];",
      -1,"oolite-event-setup.js",NULL);
    if(s->rc==JSERR_OK)s->rc=OOGFOSScriptObjectLoadUTF16(task,"script",
            s->source,s->units,"oolite-default-ship-script.js");
    return s->rc;
}
static LONG injectThrow(JSTask task,APTR data)
{
    (void)data;
    return JS_Evaluate(task,
      "script.shipDied=function(){throw new Error('expected');};",
      -1,"oolite-test-throw.js",NULL);
}
static LONG injectRecovery(JSTask task,APTR data)
{
    (void)data;
    return JS_Evaluate(task,
      "script.shipDied=function(){this.ship.__runLegacyScriptActions(this.ship,['recover']);};",
      -1,"oolite-test-recover.js",NULL);
}
static int onGameLegacy(void *owner,ULONG token,unsigned action,
                        LONG count,LONG *result)
{
    struct OOGFOSShipEntityRegistry *reg=(struct OOGFOSShipEntityRegistry *)owner;
    void *ship=OOGFOSShipTokensLookup(&reg->tokens,token);
    if(!ship || action!=OOGFOS_SHIP_ACTION_LEGACY || !result || count<0)return 0;
    if(!OOGFOSTestObjCRecordLegacy(ship,(int)count))return 0;
    *result=count;return 1;
}
static UWORD *loadSource(LONG *units)
{
    const char *path="SYS:Oolite/Scripts/oolite-default-ship-script.utf16be";
    FILE *f=fopen(path,"rb");
    long bytes;UWORD *data;
    if(!f)return NULL;
    if(fseek(f,0,SEEK_END)!=0 || (bytes=ftell(f))<2 ||
       bytes%2!=0 || bytes>524288 || fseek(f,0,SEEK_SET)!=0){fclose(f);return NULL;}
    data=(UWORD *)malloc((size_t)bytes);
    if(!data || fread(data,1,(size_t)bytes,f)!=(size_t)bytes){
        free(data);fclose(f);return NULL;
    }
    fclose(f);*units=(LONG)(bytes/2);return data;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct OOGFOSShipEntityRegistry registry;
    struct OOGFOSGameDispatch dispatcher;
    struct OOGFOSGameShipProxy proxy;
    struct OOGFOSShipHandle handle;
    struct Setup setup;
    void *ship;
    UWORD *source;
    ULONG token;
    LONG units=0,calledA=0,calledB=0,calledMissing=-1,calledThrow=-1,calledRecover=-1;
    LONG a=-1,b=-1,c=-1,d=-1,e=-1,setupPost=-1,thrownPost=-1,recoverPost=-1;
    long calls=-1,actions=-1;
    int bound=0,closed=0,ok=0;
    setvbuf(stdout,NULL,_IONBF,0);
    puts("OOLITE_EVENT_STAGE source");
    source=loadSource(&units);
    if(!source){puts("OOLITE_JS_EVENT_INVOKE asset=UNAVAILABLE");return 77;}
    OOGFOSShipEntityRegistryInit(&registry);
    ship=OOGFOSTestObjCShipNew();
    if(!ship){free(source);return 75;}
    token=OOGFOSShipEntityRegister(&registry,ship);
    if(!token || !OOGFOSGameDispatchInitRouted(&dispatcher,&registry,
          OOGFOSShipEntityGameBackend()))return 75;
    if(!OOGFOSGameDispatchSetAction(&dispatcher,onGameLegacy))return 75;
    if(!OOGFOSJSOpen(&js))return 77;
    proxy.dispatch=&dispatcher;proxy.token=token;
    handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
    bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClassWithEvents(),&handle,"ship");
    setup=(struct Setup){source,units,-1};
    if(bound)setupPost=JS_Post(js.task,setupOnJS,&setup,1);
    printf("OOLITE_EVENT_STAGE setup post=%ld rc=%ld\\n",(long)setupPost,(long)setup.rc);
    if(setupPost==JSERR_OK && setup.rc==JSERR_OK){
        printf("OOLITE_EVENT_STAGE spawn_begin\\n");
        a=OOGFOSScriptInvokeEvent(&js,&dispatcher,"script","shipSpawned",0,NULL,&calledA);
        printf("OOLITE_EVENT_STAGE spawn_end code=%ld\\n",(long)a);
        printf("OOLITE_EVENT_STAGE death_begin\\n");
        b=OOGFOSScriptInvokeEvent(&js,&dispatcher,"script","shipDied",0,NULL,&calledB);
        printf("OOLITE_EVENT_STAGE death_end code=%ld\\n",(long)b);
        printf("OOLITE_EVENT_STAGE missing_begin\\n");
        c=OOGFOSScriptInvokeEvent(&js,&dispatcher,"script","shipSpawned",0,NULL,&calledMissing);
        printf("OOLITE_EVENT_STAGE missing_end code=%ld\\n",(long)c);
        thrownPost=JS_Post(js.task,injectThrow,NULL,1);
        if(thrownPost==JSERR_OK)
            printf("OOLITE_EVENT_STAGE exception_begin\\n");
        d=OOGFOSScriptInvokeEvent(&js,&dispatcher,"script","shipDied",0,NULL,&calledThrow);
        printf("OOLITE_EVENT_STAGE exception_end code=%ld\\n",(long)d);
        recoverPost=JS_Post(js.task,injectRecovery,NULL,1);
        if(recoverPost==JSERR_OK)
            printf("OOLITE_EVENT_STAGE recovery_begin\\n");
        e=OOGFOSScriptInvokeEvent(&js,&dispatcher,"script","shipDied",0,NULL,&calledRecover);
        printf("OOLITE_EVENT_STAGE recovery_end code=%ld\\n",(long)e);
    }
    OOGFOSTestObjCGetLegacyCounts(ship,&calls,&actions);
    ok=bound && setupPost==JSERR_OK && setup.rc==JSERR_OK &&
       a==JSERR_OK && b==JSERR_OK && c==JSERR_OK &&
       d==JSERR_EXCEPTION && e==JSERR_OK &&
       calledA==1 && calledB==1 && calledMissing==0 &&
       calledThrow==0 && calledRecover==1 &&
       thrownPost==JSERR_OK && recoverPost==JSERR_OK &&
       calls==4 && actions==5 &&
       dispatcher.requests==5 && dispatcher.completions==5;
    OOGFOSShipInvalidate(&handle);
    OOGFOSJSClose(&js);
    OOGFOSShipEntityUnregister(&registry,token);
    closed=OOGFOSGameDispatchTryClose(&dispatcher);
    OOGFOSTestObjCShipFree(ship);free(source);
    printf("OOLITE_JS_EVENT_INVOKE units=%ld actions=%ld calls=%ld called=%ld,%ld,%ld,%ld,%ld codes=%ld,%ld,%ld,%ld,%ld rpc=%lu done=%lu closed=%d status=%s\n",
      (long)units,actions,calls,(long)calledA,(long)calledB,(long)calledMissing,
      (long)calledThrow,(long)calledRecover,(long)a,(long)b,(long)c,(long)d,(long)e,
      (unsigned long)dispatcher.requests,(unsigned long)dispatcher.completions,closed,
      ok&&closed?"PASS":"FAIL");
    return ok&&closed?0:2;
}
