/* Oolite original Vector3D/Quaternion native rotation API qualification.
 * All objects are JS-owned snapshots in the canonical javascript.library.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSJSVector.h"
#include "OOGFOSJSQuaternion.h"
#include <proto/javascript.h>
#include <stdio.h>
struct Probe {LONG vecInstall,quatInstall,code,answer,invalid,recovered,nativeRead;DOUBLE w,x,y,z;};
static LONG run(JSTask task,APTR data)
{
    struct Probe *p=(struct Probe*)data;
    JSValue r=NULL,native=NULL,global=NULL;
    p->vecInstall=OOGFOSJSVectorInstall(task);
    p->quatInstall=OOGFOSJSQuaternionInstall(task);
    if(p->vecInstall!=JSERR_OK||p->quatInstall!=JSERR_OK)return JSERR_BADARGS;
    p->code=JS_Evaluate(task,(CONST_STRPTR)
       "(function(){"
       "var eq=function(a,b){return Math.abs(a-b)<1e-9};"
       "var v=new Vector3D(1,0,0);"
       "var id=new Quaternion();"
       "var q=v.rotationTo(new Vector3D(0,1,0));"
       "var rotated=v.rotateBy(q);"
       "return id.w===1 && id.x===0 && id.y===0 && id.z===0 && "
       "id instanceof Quaternion && q instanceof Quaternion && "
       "Object.getPrototypeOf(q)===Quaternion.prototype && "
       "Quaternion.prototype.constructor===Quaternion && "
       "eq(rotated.x,0)&&eq(rotated.y,1)&&eq(rotated.z,0) && "
       "eq(v.rotateBy(new Quaternion(1,0,0,0)).x,1) && "
       "eq(v.rotateBy(new Quaternion(1,0,0,0).rotateZ(Math.PI/2)).y,1) && "
       "eq(id.normalize().w,1)&&eq(q.conjugate().z,-q.z) && "
       "eq(q.dot(q),1)&&q.toArray().length===4 && "
       "eq(id.multiply(q).z,q.z) && "
       "eq(q.vectorRight().y,1) && "
       "eq(new Vector3D(0,0,0).rotationTo(v).w,1) && "
       "eq(v.rotationTo(new Vector3D(-1,0,0)).w,0) && "
       "eq(v.rotateBy(v.rotationTo(new Vector3D(0,1,0),Math.PI/4)).y,Math.SQRT1_2) && "
       "q.toSource().indexOf('Quaternion(')===0 ? 42 : 0;"
       "})()", -1,(CONST_STRPTR)"oolite-vector-quaternion.js",&r);
    if(p->code==JSERR_OK&&r)p->code=JS_ToLong(task,r,&p->answer);
    if(r)JS_Free(task,r);
    native=OOGFOSJSQuaternionCreate(task,0.5,0.25,0.125,0.0625);
    p->nativeRead=OOGFOSJSQuaternionRead(task,native,&p->w,&p->x,&p->y,&p->z);
    if(native)JS_Free(task,native);
    p->invalid=JS_Evaluate(task,(CONST_STRPTR)
       "new Quaternion(Infinity,0,0,0);",-1,
       (CONST_STRPTR)"oolite-quaternion-invalid.js",NULL);
    p->recovered=0;
    global=JS_Global(task);
    if(global)JS_Free(task,global);
    r=NULL;
    if(JS_Evaluate(task,(CONST_STRPTR)"6*7",-1,
       (CONST_STRPTR)"oolite-quaternion-recovery.js",&r)==JSERR_OK && r)
       JS_ToLong(task,r,&p->recovered);
    if(r)JS_Free(task,r);
    return p->code;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct Probe p={JSERR_BADARGS,JSERR_BADARGS,JSERR_BADARGS,0,0,0,JSERR_BADARGS,0,0,0,0};
    LONG rc;
    int ok;
    if(!OOGFOSJSOpen(&js)){
        puts("OOLITE_VECTOR_QUATERNION status=UNAVAILABLE");return 77;
    }
    rc=JS_Post(js.task,run,&p,1);
    ok=rc==JSERR_OK&&p.vecInstall==0&&p.quatInstall==0&&p.code==0&&
       p.answer==42&&p.nativeRead==0&&p.w==0.5&&p.x==0.25&&
       p.y==0.125&&p.z==0.0625&&p.invalid==JSERR_EXCEPTION&&p.recovered==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_VECTOR_QUATERNION vector=%ld quat=%ld code=%ld answer=%ld read=%ld invalid=%ld recovered=%ld status=%s\n",
       (long)p.vecInstall,(long)p.quatInstall,(long)p.code,
       (long)p.answer,(long)p.nativeRead,(long)p.invalid,
       (long)p.recovered,ok?"PASS":"FAIL");
    return ok?0:2;
}
