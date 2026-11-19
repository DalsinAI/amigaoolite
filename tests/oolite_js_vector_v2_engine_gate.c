/* Canonical v2 JavaScriptEngine feature gate for original Oolite Vector3D.
 * Requires the true JSHostClass prototype callback, not the v1 fallback.
 * GPL-2.0-or-later, 2026 AmiCradle Team.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSJSVector.h"
#include <proto/javascript.h>
#include <stdio.h>

struct GateResult {LONG v2,install,eval,answer;};
static LONG perform(JSTask task,APTR data)
{
    struct GateResult *r=(struct GateResult*)data;
    JSValue v=NULL;
    r->v2=OOGFOSJSVectorNativePrototypeAvailable(task);
    if(!r->v2)return JSERR_NOENGINE;
    r->install=OOGFOSJSVectorInstall(task);
    if(r->install!=JSERR_OK)return r->install;
    r->eval=JS_Evaluate(task,
        "(function(){"
        "var a=new Vector3D(3,4,0), b=Vector3D([1,2,3]);"
        "return a instanceof Vector3D && "
        "Object.getPrototypeOf(a)===Vector3D.prototype && "
        "Object.getPrototypeOf(b)===Vector3D.prototype && "
        "Vector3D.prototype.constructor===Vector3D && "
        "b.x===1 && b.y===2 && b.z===3 && "
        "a.magnitude()===5 && a.add([1,2,3]).y===6 && "
        "typeof a.toSource()==='string' && "
        "Vector3D.interpolate(a,b,0.5).x===2 "
        "? 42 : 0;"
        "})()", -1, "oolite-vector-native-v2.js", &v);
    if(r->eval==JSERR_OK && v)
        r->eval=JS_ToLong(task,v,&r->answer);
    if(v)JS_Free(task,v);
    return r->eval;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct GateResult r={0,JSERR_BADARGS,JSERR_BADARGS,-1};
    LONG post=JSERR_NOENGINE;
    int ok;
    if(!OOGFOSJSOpen(&js)){
        puts("OOLITE_VECTOR_V2 status=UNAVAILABLE");
        return 77;
    }
    post=JS_Post(js.task,perform,&r,1);
    ok=post==JSERR_OK && r.v2==1 && r.install==JSERR_OK &&
       r.eval==JSERR_OK && r.answer==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_VECTOR_V2 v2=%ld install=%ld eval=%ld answer=%ld post=%ld status=%s\n",
        (long)r.v2,(long)r.install,(long)r.eval,(long)r.answer,
        (long)post,ok?"PASS":!r.v2?"REQUIRES_MATCHED_V2_ENGINE":"FAIL");
    return ok?0:!r.v2?77:2;
}
