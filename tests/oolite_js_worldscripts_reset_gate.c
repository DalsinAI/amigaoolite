/* Original UTF-16 Oolite ship script & worldScripts registry survive
 * aborted JavaScript task reset and rebind after a successful reset.
 * Every bootstrap is game-task routed; native AC090 68040 GPL-2.0-or-later.
 */
#include "OOGFOSScriptReset.h"
#include "OOGFOSScriptObjects.h"
#include "OOGFOSScriptEventInvoke.h"
#include "OOGFOSWorldScripts.h"
#include "OOGFOSShipEntityBackend.h"
#include "OOGFOSShipEvents.h"
#include <proto/javascript.h>
#include <stdlib.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
extern int OOGFOSTestObjCRecordLegacy(void *ship,int count);
extern int OOGFOSTestObjCGetLegacyCounts(void *ship,long *calls,long *actions);
struct State {
    struct OOGFOSShipHandle *handle;
    UWORD *source;LONG units;
    LONG prepares,fail,wills,dids;
};
static UWORD *readScript(LONG *units)
{
    FILE *f=fopen("SYS:Oolite/Scripts/oolite-default-ship-script.utf16be","rb");
    UWORD *data=NULL;long bytes=-1;
    if(!f)return NULL;
    if(fseek(f,0,SEEK_END)==0)bytes=ftell(f);
    if(bytes>0 && bytes%2==0 && bytes<524288 && fseek(f,0,SEEK_SET)==0){
        data=(UWORD *)malloc((size_t)bytes);
        if(data && fread(data,1,(size_t)bytes,f)==(size_t)bytes)
            *units=(LONG)(bytes/2);
        else{free(data);data=NULL;}
    }
    fclose(f);return data;
}
static LONG prepare(JSTask task,APTR user)
{
    struct State *s=(struct State *)user;
    JSClass cls;JSValue global=NULL,ship=NULL,out=NULL;
    LONG rc=JSERR_BADARGS,result=0;
    ++s->prepares;
    cls=JS_RegisterClass(task,OOGFOSShipClassWithEvents());
    if(!cls)return JSERR_NOMEMORY;
    ship=JS_Wrap(task,cls,s->handle);
    if(!ship)return JSERR_NOMEMORY;
    global=JS_Global(task);
    if(!global)goto cleanup;
    rc=JS_SetProp(task,global,"ship",ship);
    if(rc!=JSERR_OK)goto cleanup;
    rc=OOGFOSScriptObjectCreate(task,"script","ship");
    if(rc!=JSERR_OK)goto cleanup;
    rc=JS_Evaluate(task,
       "this.system={mainStation:{}};"
       "script.legacy_setupActions=['initial','spawn'];"
       "script.legacy_launchActions=['launch'];"
       "script.legacy_deathActions=['death'];",
       -1,"oolite-reset-setup.js",NULL);
    if(rc!=JSERR_OK)goto cleanup;
    rc=OOGFOSScriptObjectLoadUTF16(task,"script",s->source,s->units,
       "oolite-default-ship-script.js");
    if(rc!=JSERR_OK)goto cleanup;
    rc=OOGFOSWorldScriptsRegister(task,"script","oolite-default-ship-script");
    if(rc!=JSERR_OK)goto cleanup;
    rc=JS_Evaluate(task,
      "worldScripts['oolite-default-ship-script']===script && "
      "Object.getOwnPropertyDescriptor(worldScripts,"
      "'oolite-default-ship-script').writable===false && "
      "script.ship.bounty===10 ? 1 : 0",
      -1,"oolite-reset-worldscript-check.js",&out);
    if(rc==JSERR_OK)rc=JS_ToLong(task,out,&result);
    if(rc==JSERR_OK && result!=1)rc=JSERR_TYPE;
    if(rc==JSERR_OK && s->fail)rc=JSERR_TYPE;
cleanup:
    if(out)JS_Free(task,out);
    if(global)JS_Free(task,global);
    JS_Free(task,ship);
    return rc;
}
static LONG invokeSetup(JSTask task,APTR user,LONG *result)
{
    if(result)*result=0;
    return prepare(task,user);
}
static void will(APTR user){++((struct State*)user)->wills;}
static void did(APTR user){++((struct State*)user)->dids;}
static int onGameLegacy(void *owner,ULONG token,unsigned action,
                        LONG count,LONG *result)
{
    struct OOGFOSShipEntityRegistry *r=(struct OOGFOSShipEntityRegistry *)owner;
    void *ship=OOGFOSShipTokensLookup(&r->tokens,token);
    if(!ship || action!=OOGFOS_SHIP_ACTION_LEGACY || !result || count<0)return 0;
    if(!OOGFOSTestObjCRecordLegacy(ship,(int)count))return 0;
    *result=count;return 1;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct OOGFOSShipEntityRegistry registry;
    struct OOGFOSGameDispatch dispatch;
    struct OOGFOSGameShipProxy proxy;
    struct OOGFOSShipHandle handle;
    struct State state={0};
    void *ship=NULL;ULONG token=0;
    LONG ignored=0,calledFirst=0,calledDeath=0,calledAbort=0,calledReborn=0;
    LONG init=-1,spawn=-1,death=-1,failed=-1,afterAbort=-1,reset=-1,reborn=-1;
    long calls=0,actions=0;
    int ok=0,bound=0,closed=0;
    state.source=readScript(&state.units);
    if(!state.source){puts("OOLITE_JS_WORLDSCRIPTS_RESET asset=UNAVAILABLE");return 77;}
    OOGFOSShipEntityRegistryInit(&registry);
    ship=OOGFOSTestObjCShipNew();
    if(!ship)return 75;
    token=OOGFOSShipEntityRegister(&registry,ship);
    if(!token || !OOGFOSGameDispatchInitRouted(&dispatch,&registry,
           OOGFOSShipEntityGameBackend()))return 75;
    if(!OOGFOSGameDispatchSetAction(&dispatch,onGameLegacy))return 75;
    if(!OOGFOSJSOpen(&js))return 77;
    proxy.dispatch=&dispatch;proxy.token=token;
    handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
    state.handle=&handle;
    init=OOGFOSScriptInvoke(&js,&dispatch,invokeSetup,&state,&ignored);
    if(init==JSERR_OK){
        spawn=OOGFOSScriptInvokeEvent(&js,&dispatch,"script","shipSpawned",0,NULL,&calledFirst);
        death=OOGFOSScriptInvokeEvent(&js,&dispatch,"script","shipDied",0,NULL,&calledDeath);
        state.fail=1;
        failed=OOGFOSScriptResetRouted(&js,&dispatch,prepare,will,did,&state);
        afterAbort=OOGFOSScriptInvokeEvent(&js,&dispatch,"script","shipDied",0,NULL,&calledAbort);
        state.fail=0;
        reset=OOGFOSScriptResetRouted(&js,&dispatch,prepare,will,did,&state);
        if(reset==JSERR_OK)
            reborn=OOGFOSScriptInvokeEvent(&js,&dispatch,"script","shipSpawned",0,NULL,&calledReborn);
    }
    OOGFOSTestObjCGetLegacyCounts(ship,&calls,&actions);
    ok=init==JSERR_OK && spawn==JSERR_OK && death==JSERR_OK &&
       failed==JSERR_TYPE && afterAbort==JSERR_OK &&
       reset==JSERR_OK && reborn==JSERR_OK &&
       calledFirst==1 && calledDeath==1 && calledAbort==1 &&
       calledReborn==1 && state.prepares==3 &&
       state.wills==1 && state.dids==1 &&
       dispatch.completions==7 && dispatch.requests>6 &&
       calls>=7 && actions>=8;
    OOGFOSShipInvalidate(&handle);
    OOGFOSJSClose(&js);
    OOGFOSShipEntityUnregister(&registry,token);
    closed=OOGFOSGameDispatchTryClose(&dispatch);
    OOGFOSTestObjCShipFree(ship);
    free(state.source);
    printf("OOLITE_JS_WORLDSCRIPTS_RESET codes=%ld,%ld,%ld,%ld,%ld,%ld,%ld called=%ld,%ld,%ld,%ld prepared=%ld notices=%ld,%ld ship_calls=%ld actions=%ld rpc=%lu done=%lu closed=%d status=%s\n",
      (long)init,(long)spawn,(long)death,(long)failed,(long)afterAbort,(long)reset,(long)reborn,
      (long)calledFirst,(long)calledDeath,(long)calledAbort,(long)calledReborn,
      (long)state.prepares,(long)state.wills,(long)state.dids,calls,actions,
      (unsigned long)dispatch.requests,(unsigned long)dispatch.completions,
      closed,ok&&closed?"PASS":"FAIL");
    return ok&&closed?0:2;
}
