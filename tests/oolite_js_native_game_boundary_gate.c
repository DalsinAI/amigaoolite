/* Native Oolite game state vs OXP script runtime boundary: one shared
 * javascript.library JSTask, original Oolite worldScripts registry,
 * separately scoped scripts, native Ship host and native Vector3D/Quaternion.
 * This is a genuine scripting integration gate, not the full-game executable.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSJSVector.h"
#include "OOGFOSJSQuaternion.h"
#include "OOGFOSScriptObjects.h"
#include "OOGFOSWorldScripts.h"
#include <proto/javascript.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct NativeShip { LONG bounty; };
static const struct JSHostSlot shipSlots[] = {
    { (CONST_STRPTR)"bounty", JSS_FIELD, JSF_LONG,
      NULL,NULL,offsetof(struct NativeShip,bounty),0 },
    { NULL,0,0,NULL,NULL,0,0 }
};
static const struct JSHostClass shipClass = {
    "OoliteNativeShip", NULL, shipSlots, NULL, NULL
};

struct ScriptTest {
    LONG vector, quaternion, scripts, registry, loadA, loadB;
    LONG resultA, resultB, callsA, callsB, eventA, eventB, nativeRead;
    LONG worldCreate,worldLoad,worldRegister,worldCall,worldCalled,worldResult;
    LONG readOnlyAttr,mutableAttr,readAttr;
    LONG authorPresent;
    DOUBLE x,y,z;
};

static LONG prepareAndRun(JSTask t,APTR opaque)
{
    struct ScriptTest *s=(struct ScriptTest*)opaque;
    JSValue a=NULL,b=NULL,w=NULL,author=NULL,counter=NULL,readAuthor=NULL;
    s->vector=OOGFOSJSVectorInstall(t);
    s->quaternion=OOGFOSJSQuaternionInstall(t);
    if(s->vector||s->quaternion)return JSERR_BADARGS;
    s->registry=OOGFOSWorldScriptsInit(t);
    if(s->registry)return s->registry;
    s->scripts=OOGFOSScriptObjectCreate(t,"oxpA","ship");
    if(s->scripts)return s->scripts;
    s->scripts=OOGFOSScriptObjectCreate(t,"oxpB","ship");
    if(s->scripts)return s->scripts;
    s->registry=OOGFOSWorldScriptsRegister(t,"oxpA","oxp.a");
    if(s->registry)return s->registry;
    s->registry=OOGFOSWorldScriptsRegister(t,"oxpB","oxp-b");
    if(s->registry)return s->registry;
    /* Original OOJSScript world/global scripts do not have a ShipEntity
     * property. Reuse the shared script object lifecycle without requiring
     * a dummy ship and prove cross-script worldScripts reachability. */
    s->worldCreate=OOGFOSScriptObjectCreate(t,"worldOXP",NULL);
    if(s->worldCreate)return s->worldCreate;
    /* OOJSScript defines manifest/properties before executing source.
     * The readonly author field is not a CSS/JS source literal injection. */
    author=JS_NewString(t,(CONST_STRPTR)"AmiCradle",9);
    counter=JS_NewLong(t,7);
    if(!author || !counter){
        if(author)JS_Free(t,author);
        if(counter)JS_Free(t,counter);
        return JSERR_NOMEMORY;
    }
    s->readOnlyAttr=OOGFOSScriptObjectSetProperty(t,"worldOXP",
        "author",author,1);
    s->mutableAttr=OOGFOSScriptObjectSetProperty(t,"worldOXP",
        "counter",counter,0);
    JS_Free(t,author);JS_Free(t,counter);
    if(s->readOnlyAttr || s->mutableAttr)
        return s->readOnlyAttr?s->readOnlyAttr:s->mutableAttr;
    s->readAttr=OOGFOSScriptObjectGetProperty(t,"worldOXP",
        "author",&readAuthor);
    if(s->readAttr==JSERR_OK&&readAuthor){
        char buffer[24];
        LONG n=JS_ToString(t,readAuthor,(STRPTR)buffer,sizeof(buffer));
        s->authorPresent=n==9&&strcmp(buffer,"AmiCradle")==0;
    }
    if(readAuthor)JS_Free(t,readAuthor);
    if(!s->authorPresent)return JSERR_BADARGS;
    s->worldRegister=OOGFOSWorldScriptsRegister(t,"worldOXP","global.only");
    if(s->worldRegister)return s->worldRegister;
    static const char worldSrc[] =
        "this.name='world-oxp';"
        "this.counter=8;"
        "try{this.author='hacked';}catch(e){}"
        "this.shipSpawned=function(){"
        "var desc=Object.getOwnPropertyDescriptor(this,'author');"
        "return typeof this.ship==='undefined' &&"
        "this.author==='AmiCradle' && this.counter===8 &&"
        "desc && !desc.writable && desc.enumerable===true"
        "?42:-1;};";
    s->worldLoad=OOGFOSScriptObjectLoad(t,"worldOXP",
        (CONST_STRPTR)worldSrc,(LONG)strlen(worldSrc),
        (CONST_STRPTR)"world-oxp.js");
    if(s->worldLoad)return s->worldLoad;
    s->worldCall=OOGFOSScriptObjectEventValue(t,
        "worldOXP","shipSpawned",0,NULL,&w,&s->worldCalled);
    if(s->worldCall==JSERR_OK && w)
        s->worldCall=JS_ToLong(t,w,&s->worldResult);
    if(w)JS_Free(t,w);
    if(s->worldCall)return s->worldCall;

    /* OXP scripts define callbacks; the game retains native state.
     * OXP B reads OXP A via the canonical worldScripts registry.
     * Quaternion and Vector3D are implemented by native C math. */
    static const char srcA[] =
        "this.name='native-oxp-a';"
        "this.shipSpawned=function(){"
        " var v=new Vector3D(1,0,0);"
        " var q=new Quaternion().rotateZ(Math.PI/2);"
        " return v.rotateBy(q);"
        "};";
    s->loadA=OOGFOSScriptObjectLoad(t,"oxpA",(CONST_STRPTR)srcA,
        (LONG)strlen(srcA),(CONST_STRPTR)"oxp-a.js");
    if(s->loadA)return s->loadA;
    static const char srcB[] =
        "this.shipSpawned=function(){"
        " if(worldScripts['global.only'].shipSpawned()!==42)"
        " throw new Error('missing global-only world script');"
        " var v=worldScripts['oxp.a'].shipSpawned();"
        " return Math.round(v.y*40)+this.ship.bounty;"
        "};";
    s->loadB=OOGFOSScriptObjectLoad(t,"oxpB",(CONST_STRPTR)srcB,
        (LONG)strlen(srcB),(CONST_STRPTR)"oxp-b.js");
    if(s->loadB)return s->loadB;
    s->eventA=OOGFOSScriptObjectEventValue(t,
        "oxpA","shipSpawned",0,NULL,&a,&s->callsA);
    if(s->eventA==JSERR_OK&&a)
        s->nativeRead=OOGFOSJSVectorRead(t,a,&s->x,&s->y,&s->z);
    s->eventB=OOGFOSScriptObjectEventValue(t,
        "oxpB","shipSpawned",0,NULL,&b,&s->callsB);
    if(s->eventB==JSERR_OK&&b)
        s->eventB=JS_ToLong(t,b,&s->resultB);
    if(a)JS_Free(t,a);
    if(b)JS_Free(t,b);
    return s->eventA!=JSERR_OK?s->eventA:s->eventB;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct NativeShip ship={2};
    struct ScriptTest r={0};
    LONG rc;
    int exposed,ok;
    if(!OOGFOSJSOpen(&js))return 77;
    exposed=OOGFOSJSExposeHostClass(&js,&shipClass,&ship,"ship");
    rc=exposed?JS_Post(js.task,prepareAndRun,&r,1):JSERR_TYPE;
    ok=exposed&&rc==JSERR_OK&&r.vector==JSERR_OK&&
       r.quaternion==JSERR_OK&&r.registry==JSERR_OK&&
       r.loadA==JSERR_OK&&r.loadB==JSERR_OK&&
       r.eventA==JSERR_OK&&r.eventB==JSERR_OK&&
       r.nativeRead==JSERR_OK&&r.callsA==1&&r.callsB==1&&
       r.worldCreate==JSERR_OK&&r.worldRegister==JSERR_OK&&
       r.worldLoad==JSERR_OK&&r.worldCall==JSERR_OK&&
       r.worldCalled==1&&r.worldResult==42&&
       r.readOnlyAttr==JSERR_OK&&r.mutableAttr==JSERR_OK&&
       r.readAttr==JSERR_OK&&r.authorPresent==1&&
       r.x>-0.000001&&r.x<0.000001&&
       r.y>0.999999&&r.y<1.000001&&
       r.z==0.0&&r.resultB==42&&ship.bounty==2;
    OOGFOSJSClose(&js);
    printf("OOLITE_NATIVE_OXP_BOUNDARY exposed=%d rc=%ld vec=%ld quat=%ld registry=%ld oxp=%ld,%ld event=%ld,%ld calls=%ld,%ld native=(%.3f,%.3f,%.3f) gameBounty=%ld result=%ld status=%s\n",
           exposed,(long)rc,(long)r.vector,(long)r.quaternion,
           (long)r.registry,(long)r.loadA,(long)r.loadB,
           (long)r.eventA,(long)r.eventB,
           (long)r.callsA,(long)r.callsB,r.x,r.y,r.z,
           (long)ship.bounty,(long)r.resultB,ok?"PASS":"FAIL");
    printf("OOLITE_WORLD_SCRIPT_PROPERTIES readonly=%ld mutable=%ld get=%ld author=%ld status=%s\n",
      (long)r.readOnlyAttr,(long)r.mutableAttr,(long)r.readAttr,
      (long)r.authorPresent,ok?"PASS":"FAIL");
    printf("OOLITE_WORLD_SCRIPT_NO_SHIP create=%ld register=%ld load=%ld event=%ld called=%ld result=%ld status=%s\n",
      (long)r.worldCreate,(long)r.worldRegister,(long)r.worldLoad,
      (long)r.worldCall,(long)r.worldCalled,(long)r.worldResult,
      ok?"PASS":"FAIL");
    return ok?0:2;
}
