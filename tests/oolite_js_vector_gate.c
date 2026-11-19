/* Native original Oolite Vector3D core methods on canonical GFOS JS.
 * Full 68k host-class methods, no JSAPI ABI substitution or game mock.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSJSVector.h"
#include <proto/javascript.h>
#include <stdio.h>
struct Probe {LONG rc,valid,convert,invalid,arrayInvalid,objectInvalid,recovered,withNew;DOUBLE x,y,z;};
static LONG run(JSTask task,APTR user)
{
    struct Probe *p=(struct Probe *)user;
    JSValue a=NULL,global=NULL,src=NULL;
    p->rc=OOGFOSJSVectorInstall(task);
    if(p->rc!=JSERR_OK)return p->rc;
    p->rc=JS_Evaluate(task,
      "var a=Vector3D(3,4,0);"
      "var b=Vector3D(1,2,3);"
      "var eq=function(n,m){return Math.abs(n-m)<1e-9};"
      "(typeof a.x==='number' && a.x===3 && a.y===4 && a.z===0"
      " && a.magnitude()===5 && a.squaredMagnitude()===25"
      " && a.dot(b)===11 && a.cross(b).x===12"
      " && a.cross(b).y===-9 && a.cross(b).z===2"
      " && a.add(b).y===6 && a.subtract(b).z===-3"      " && a.add([1,2,3]).y===6 && a.dot([1,2,3])===11"      " && Vector3D([3,4,0]).magnitude()===5"
      " && a.multiply(2).x===6 && eq(a.distanceTo(b),Math.sqrt(17))"
      " && a.squaredDistanceTo(b)===17"
      " && eq(a.direction().magnitude(),1)"
      " && Array.isArray(a.toArray()) && a.toArray()[1]===4"
      " && Vector3D().magnitude()===0"
      " && Vector3D(a).x===3 && a instanceof Vector3D"
      " && Object.getPrototypeOf(a)===Vector3D.prototype"
      " && Vector3D.prototype.constructor===Vector3D"
      " && a.toSource()===\'Vector3D(3, 4, 0)\'"
      " && typeof a.toString()===\'string\'"
      " && eq(a.angleTo(Vector3D(0,1,0)),Math.acos(0.8))"
      " && a.tripleProduct(b,Vector3D(0,0,1))===2"
      " && Vector3D.interpolate(a,b,0.25).x===2.5"
      " && Vector3D.interpolate(a,b,0.25).z===0.75) ? 42:0",
      -1,"oolite-vector-arithmetic.js",&src);
    if(p->rc==JSERR_OK) p->rc=JS_ToLong(task,src,&p->valid);
    if(src)JS_Free(task,src);src=NULL;
    global=JS_Global(task);
    a=OOGFOSJSVectorCreate(task,1.25,2.50,3.75);
    if(!a){p->rc=JSERR_NOMEMORY;goto cleanup;}
    p->convert=OOGFOSJSVectorRead(task,a,&p->x,&p->y,&p->z);
    if(global)JS_SetProp(task,global,"externalVector",a);
    p->arrayInvalid=JS_Evaluate(task,
        "a.add([1,2]);",
        -1,"oolite-vector-short-array.js",NULL);
    p->objectInvalid=JS_Evaluate(task,
        "a.add({0:1,1:2,2:3,length:3});",
        -1,"oolite-vector-not-array.js",NULL);
    p->invalid=JS_Evaluate(task,
        "Vector3D(1,2,Infinity);",
        -1,"oolite-vector-invalid.js",NULL);
    src=NULL;
    p->rc=JS_Evaluate(task,
        "(externalVector.x===1.25 && externalVector.y===2.5"
        " && externalVector.z===3.75 && a.x===3) ? 42:0",
        -1,"oolite-vector-after-error.js",&src);
    if(p->rc==JSERR_OK) p->rc=JS_ToLong(task,src,&p->recovered);
    if(src)JS_Free(task,src);src=NULL;
    p->withNew=JS_Evaluate(task,
        "var v=new Vector3D(7,8,9);v.z===9 ? 42:0",
        -1,"oolite-vector-construct.js",&src);
    if(p->withNew==JSERR_OK){
        LONG result=0;
        p->withNew=JS_ToLong(task,src,&result)==JSERR_OK ? result : -1;
    } else p->withNew=-1;
cleanup:
    if(src)JS_Free(task,src);
    if(a)JS_Free(task,a);
    if(global)JS_Free(task,global);
    return p->rc;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct Probe p={JSERR_BADARGS,0,JSERR_BADARGS,JSERR_BADARGS,JSERR_BADARGS,JSERR_BADARGS,0,-1,0,0,0};
    LONG post;
    int ok;
    if(!OOGFOSJSOpen(&js)){puts("OOLITE_JS_VECTOR status=UNAVAILABLE");return 77;}
    post=JS_Post(js.task,run,&p,1);
    ok=post==JSERR_OK && p.rc==JSERR_OK && p.valid==42 &&
       p.convert==JSERR_OK && p.x==1.25 && p.y==2.5 && p.z==3.75 &&
       p.invalid==JSERR_EXCEPTION && p.arrayInvalid==JSERR_EXCEPTION &&
       p.objectInvalid==JSERR_EXCEPTION && p.recovered==42 && p.withNew==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_JS_VECTOR valid=%ld converted=%ld coords=%ld,%ld,%ld exception=%ld shortArray=%ld objectLike=%ld recovered=%ld newCode=%ld status=%s\n",
       (long)p.valid,(long)p.convert,(long)(p.x*100),(long)(p.y*100),
       (long)(p.z*100),(long)p.invalid,(long)p.arrayInvalid,
       (long)p.objectInvalid,(long)p.recovered,
       (long)p.withNew,ok?"PASS":"FAIL");
    return ok?0:2;
}
