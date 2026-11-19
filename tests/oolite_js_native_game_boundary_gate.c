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
    DOUBLE x,y,z;
};

static LONG prepareAndRun(JSTask t,APTR opaque)
{
    struct ScriptTest *s=(struct ScriptTest*)opaque;
    JSValue a=NULL,b=NULL;
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
    return ok?0:2;
}
