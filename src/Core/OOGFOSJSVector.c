/* Oolite original OOJSVector.m scalar and vector operations implemented
 * against the canonical GFOS javascript.library host-class ABI.
 * JS-owned vector snapshots: never dereference game-task world objects.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJSVector.h"
#include <proto/javascript.h>
#include <proto/exec.h>
#include <exec/memory.h>
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <exec/libraries.h>
typedef struct OOGFOSVector {DOUBLE x,y,z;} Vec3;
static const struct JSHostClass vectorClass;
static int valid(DOUBLE value){return isfinite(value);}
static JSValue newNumber(JSTask task,DOUBLE value)
{return JS_NewDouble(task,&value);}
static JSValue get_x(JSTask task,APTR self,APTR data)
{(void)data;return newNumber(task,((Vec3*)self)->x);}
static JSValue get_y(JSTask task,APTR self,APTR data)
{(void)data;return newNumber(task,((Vec3*)self)->y);}
static JSValue get_z(JSTask task,APTR self,APTR data)
{(void)data;return newNumber(task,((Vec3*)self)->z);}
static LONG setCoord(JSTask task,APTR self,JSValue value,unsigned axis)
{
    DOUBLE number;
    Vec3 *v=(Vec3*)self;
    if(!v || JS_ToDouble(task,value,&number)!=JSERR_OK || !valid(number)){
        JS_Throw(task,(CONST_STRPTR)"Vector3D component must be finite");
        return JSERR_EXCEPTION;
    }
    if(axis==0)v->x=number;
    else if(axis==1)v->y=number;
    else v->z=number;
    return JSERR_OK;
}
static LONG set_x(JSTask task,APTR self,JSValue value,APTR data)
{(void)data;return setCoord(task,self,value,0);}
static LONG set_y(JSTask task,APTR self,JSValue value,APTR data)
{(void)data;return setCoord(task,self,value,1);}
static LONG set_z(JSTask task,APTR self,JSValue value,APTR data)
{(void)data;return setCoord(task,self,value,2);}
static VOID releaseVector(JSTask task,APTR self,APTR data)
{(void)task;(void)data;if(self)FreeVec(self);}
static JSValue create(JSTask task,DOUBLE x,DOUBLE y,DOUBLE z)
{
    JSClass cls;
    Vec3 *v;
    JSValue wrapper;
    if(!task || !valid(x) || !valid(y) || !valid(z))return NULL;
    cls=JS_RegisterClass(task,&vectorClass);
    if(!cls)return NULL;
    v=(Vec3*)AllocVec(sizeof(*v),MEMF_ANY|MEMF_CLEAR);
    if(!v)return NULL;
    v->x=x;v->y=y;v->z=z;
    wrapper=JS_Wrap(task,cls,v);
    if(!wrapper)FreeVec(v);
    return wrapper;
}
JSValue OOGFOSJSVectorCreate(JSTask task,DOUBLE x,DOUBLE y,DOUBLE z)
{return create(task,x,y,z);}
LONG OOGFOSJSVectorRead(JSTask task,JSValue value,DOUBLE *x,DOUBLE *y,DOUBLE *z)
{
    JSClass cls;
    Vec3 *v;
    if(!task || !value || !x || !y || !z)return JSERR_BADARGS;
    cls=JS_RegisterClass(task,&vectorClass);
    if(!cls)return JSERR_NOMEMORY;
    v=(Vec3*)JS_Unwrap(task,value,cls);
    if(!v)return JSERR_TYPE;
    *x=v->x;*y=v->y;*z=v->z;
    return JSERR_OK;
}
static Vec3 *other(JSTask task,LONG argc,JSValue *argv)
{
    JSClass cls;
    if(argc<1 || !argv || !argv[0])return NULL;
    cls=JS_RegisterClass(task,&vectorClass);
    return cls ? (Vec3*)JS_Unwrap(task,argv[0],cls):NULL;
}
static JSValue badArgs(JSTask task)
{return JS_Throw(task,(CONST_STRPTR)"Vector3D: expected a Vector3D argument");}
static JSValue construct(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    DOUBLE v[3]={0.0,0.0,0.0};
    Vec3 *from;
    unsigned i;
    (void)self;(void)data;
    if(argc==1 && (from=other(task,argc,argv))!=NULL)
        return create(task,from->x,from->y,from->z);
    if(argc!=0 && argc!=3)return badArgs(task);
    for(i=0;i<(unsigned)argc;i++)
        if(JS_ToDouble(task,argv[i],&v[i])!=JSERR_OK || !valid(v[i]))
            return JS_Throw(task,(CONST_STRPTR)"Vector3D: non-finite coordinate");
    return create(task,v[0],v[1],v[2]);
}
static JSValue magnitude(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *v=(Vec3*)self;
    (void)argc;(void)argv;(void)data;
    return newNumber(task,sqrt(v->x*v->x+v->y*v->y+v->z*v->z));
}
static JSValue squaredMagnitude(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *v=(Vec3*)self;
    (void)argc;(void)argv;(void)data;
    return newNumber(task,v->x*v->x+v->y*v->y+v->z*v->z);
}
static JSValue direction(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *v=(Vec3*)self;
    DOUBLE m=sqrt(v->x*v->x+v->y*v->y+v->z*v->z);
    (void)argc;(void)argv;(void)data;
    return m==0.0 ? create(task,0.0,0.0,0.0):
                    create(task,v->x/m,v->y/m,v->z/m);
}
static JSValue add(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a=(Vec3*)self,*b=other(task,argc,argv);
    (void)data;
    return b ? create(task,a->x+b->x,a->y+b->y,a->z+b->z):badArgs(task);
}
static JSValue subtract(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a=(Vec3*)self,*b=other(task,argc,argv);
    (void)data;
    return b ? create(task,a->x-b->x,a->y-b->y,a->z-b->z):badArgs(task);
}
static JSValue dot(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a=(Vec3*)self,*b=other(task,argc,argv);
    (void)data;
    return b ? newNumber(task,a->x*b->x+a->y*b->y+a->z*b->z):badArgs(task);
}
static JSValue cross(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a=(Vec3*)self,*b=other(task,argc,argv);
    (void)data;
    return b ? create(task,a->y*b->z-a->z*b->y,
                           a->z*b->x-a->x*b->z,
                           a->x*b->y-a->y*b->x):badArgs(task);
}
static JSValue distanceTo(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a=(Vec3*)self,*b=other(task,argc,argv);
    DOUBLE x,y,z;
    (void)data;
    if(!b)return badArgs(task);
    x=a->x-b->x;y=a->y-b->y;z=a->z-b->z;
    return newNumber(task,sqrt(x*x+y*y+z*z));
}
static JSValue squaredDistanceTo(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a=(Vec3*)self,*b=other(task,argc,argv);
    DOUBLE x,y,z;
    (void)data;
    if(!b)return badArgs(task);
    x=a->x-b->x;y=a->y-b->y;z=a->z-b->z;
    return newNumber(task,x*x+y*y+z*z);
}
static JSValue multiply(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a=(Vec3*)self;DOUBLE k;
    (void)data;
    if(argc<1 || !argv || JS_ToDouble(task,argv[0],&k)!=JSERR_OK ||
       !valid(k))return JS_Throw(task,(CONST_STRPTR)"Vector3D.multiply needs finite scalar");
    return create(task,a->x*k,a->y*k,a->z*k);
}
static JSValue toArray(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *v=(Vec3*)self;
    JSValue arr=NULL,coords[3]={NULL,NULL,NULL};
    unsigned i;
    int ok=1;
    DOUBLE doubles[3]={v->x,v->y,v->z};
    (void)argc;(void)argv;(void)data;
    if(JS_Evaluate(task,(CONST_STRPTR)"[]",-1,(CONST_STRPTR)"oolite-vector-array.js",&arr)!=JSERR_OK)
        return NULL;
    for(i=0;i<3;i++){
        coords[i]=newNumber(task,doubles[i]);
        if(!coords[i] || JS_SetIndex(task,arr,i,coords[i])!=JSERR_OK){
            ok=0;break;
        }
    }
    for(i=0;i<3;i++)if(coords[i])JS_Free(task,coords[i]);
    if(!ok){JS_Free(task,arr);return NULL;}
    return arr;
}
/* Remaining pure-math methods from upstream OOJSVector.m. Quaternion
 * rotation/coordinate-system methods require the separate OOJSQuaternion
 * migration and are deliberately not represented by placeholders. */
static JSValue angleTo(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a=(Vec3*)self,*b=other(task,argc,argv);
    DOUBLE da,db,ratio;
    (void)data;
    if(!b)return badArgs(task);
    da=sqrt(a->x*a->x+a->y*a->y+a->z*a->z);
    db=sqrt(b->x*b->x+b->y*b->y+b->z*b->z);
    ratio=da>0.0 && db>0.0 ?
          (a->x*b->x+a->y*b->y+a->z*b->z)/(da*db):0.0;
    if(ratio>1.0)ratio=1.0;
    if(ratio< -1.0)ratio=-1.0;
    return newNumber(task,acos(ratio));
}
static JSValue tripleProduct(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a=(Vec3*)self,*b,*c;
    DOUBLE triple;
    (void)data;
    if(argc<2 || !argv)return badArgs(task);
    b=other(task,1,argv);c=other(task,1,argv+1);
    if(!b || !c)return badArgs(task);
    triple=a->x*(b->y*c->z-b->z*c->y)
          +a->y*(b->z*c->x-b->x*c->z)
          +a->z*(b->x*c->y-b->y*c->x);
    return newNumber(task,triple);
}
static JSValue textVector(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *v=(Vec3*)self;char output[160];
    int n;
    (void)argc;(void)argv;
    n=data ?
      snprintf(output,sizeof(output),"Vector3D(%g, %g, %g)",v->x,v->y,v->z):
      snprintf(output,sizeof(output),"(%g, %g, %g)",v->x,v->y,v->z);
    if(n<0 || n>=(int)sizeof(output))
        return JS_Throw(task,(CONST_STRPTR)"Vector3D representation too long");
    return JS_NewString(task,(CONST_STRPTR)output,(LONG)n);
}
static JSValue toSource(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    (void)data;
    return textVector(task,self,argc,argv,(APTR)1);
}
static JSValue interpolate(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Vec3 *a,*b;DOUBLE t;
    (void)self;(void)data;
    if(argc<3 || !argv)return badArgs(task);
    a=other(task,1,argv);
    b=other(task,1,argv+1);
    if(!a || !b || JS_ToDouble(task,argv[2],&t)!=JSERR_OK || !valid(t))
        return JS_Throw(task,(CONST_STRPTR)"Vector3D.interpolate expects two vectors and a number");
    return create(task,a->x+(b->x-a->x)*t,
                       a->y+(b->y-a->y)*t,
                       a->z+(b->z-a->z)*t);
}

static const struct JSHostSlot slots[]={
    {(CONST_STRPTR)"x",JSS_ACCESSOR,0,(APTR)get_x,(APTR)set_x,0,0},
    {(CONST_STRPTR)"y",JSS_ACCESSOR,0,(APTR)get_y,(APTR)set_y,0,0},
    {(CONST_STRPTR)"z",JSS_ACCESSOR,0,(APTR)get_z,(APTR)set_z,0,0},
    {(CONST_STRPTR)"toString",JSS_METHOD,0,(APTR)textVector,NULL,0,0},
    {(CONST_STRPTR)"toSource",JSS_METHOD,0,(APTR)toSource,NULL,0,0},
    {(CONST_STRPTR)"angleTo",JSS_METHOD,0,(APTR)angleTo,NULL,0,1},
    {(CONST_STRPTR)"tripleProduct",JSS_METHOD,0,(APTR)tripleProduct,NULL,0,2},
    {(CONST_STRPTR)"magnitude",JSS_METHOD,0,(APTR)magnitude,NULL,0,0},
    {(CONST_STRPTR)"squaredMagnitude",JSS_METHOD,0,(APTR)squaredMagnitude,NULL,0,0},
    {(CONST_STRPTR)"direction",JSS_METHOD,0,(APTR)direction,NULL,0,0},
    {(CONST_STRPTR)"add",JSS_METHOD,0,(APTR)add,NULL,0,1},
    {(CONST_STRPTR)"subtract",JSS_METHOD,0,(APTR)subtract,NULL,0,1},
    {(CONST_STRPTR)"dot",JSS_METHOD,0,(APTR)dot,NULL,0,1},
    {(CONST_STRPTR)"cross",JSS_METHOD,0,(APTR)cross,NULL,0,1},
    {(CONST_STRPTR)"distanceTo",JSS_METHOD,0,(APTR)distanceTo,NULL,0,1},
    {(CONST_STRPTR)"squaredDistanceTo",JSS_METHOD,0,(APTR)squaredDistanceTo,NULL,0,1},
    {(CONST_STRPTR)"multiply",JSS_METHOD,0,(APTR)multiply,NULL,0,1},
    {(CONST_STRPTR)"toArray",JSS_METHOD,0,(APTR)toArray,NULL,0,0},
    {NULL,0,0,NULL,NULL,0,0}
};
static const struct JSHostClass vectorClass={
    (CONST_STRPTR)"OoliteVector3D",NULL,slots,releaseVector,NULL
};
LONG OOGFOSJSVectorInstall(JSTask task)
{
    JSValue global=NULL,native=NULL,probe=NULL,existing=NULL,prototype=NULL;
    JSValue constructor=NULL,interpolator=NULL;
    JSClass cls;
    LONG rc=JSERR_BADARGS;
    if(!task)return JSERR_BADARGS;
    cls=JS_RegisterClass(task,&vectorClass);
    if(!cls)return JSERR_NOMEMORY;
    global=JS_Global(task);
    if(!global)return JSERR_BADARGS;
    existing=JS_GetProp(task,global,(CONST_STRPTR)"Vector3D");
    if(existing && JS_TypeOf(task,existing)!=JST_UNDEFINED){
        rc=JSERR_TYPE;goto done;
    }
    /* javascript.library's HostFunction is callable but not a JS
     * constructor. Original OOJSVector requires 'new Vector3D(...)'.
     * A genuine JS function returned by this closure is constructible;
     * its native factory returns the JS_Wrap() host object as the value. */
    native=JS_NewFunction(task,(CONST_STRPTR)"_gfosVectorNative",construct,NULL);
    if(!native){rc=JSERR_NOMEMORY;goto done;}
    rc=JS_SetProp(task,global,(CONST_STRPTR)"_gfosOoliteVectorFactory",native);
    if(rc!=JSERR_OK)goto done;
    rc=JS_Evaluate(task,
        "(function(g){g.Vector3D=(function(nativeCreate){"
        "return function Vector3D(){return nativeCreate.apply(null,arguments);};"
        "})(g._gfosOoliteVectorFactory);"
        "delete g._gfosOoliteVectorFactory;})(this);",
        -1,(CONST_STRPTR)"oolite-vector-constructor.js",NULL);
    if(rc!=JSERR_OK)goto done;
    constructor=JS_GetProp(task,global,(CONST_STRPTR)"Vector3D");
    if(!constructor){rc=JSERR_BADARGS;goto done;}
#if defined(JAVASCRIPT_CLASS_PROTOTYPE_VERSION) && JAVASCRIPT_CLASS_PROTOTYPE_VERSION >= 2
    /* Native v2: true class prototype, without any placeholder wrapper.
     * JS_ClassPrototype safely returns NULL if the separately deployed
     * JavaScriptEngine still has the older v1 callback table. */
    if(JavaScriptBase &&
       JavaScriptBase->lib_Version>=JAVASCRIPT_CLASS_PROTOTYPE_VERSION)
        prototype=JS_ClassPrototype(task,cls);
#endif
    if(prototype){
        rc=JS_SetProp(task,constructor,(CONST_STRPTR)"prototype",prototype);
        if(rc!=JSERR_OK)goto done;
    }else{
        /* Older library/engine fallback: this is a JS-owned pure value,
         * never a fake ShipEntity or other game-owned object. */
        probe=create(task,0.0,0.0,0.0);
        if(!probe){rc=JSERR_NOMEMORY;goto done;}
        rc=JS_SetProp(task,global,(CONST_STRPTR)"_gfosOoliteVectorProbe",probe);
        if(rc!=JSERR_OK)goto done;
        rc=JS_Evaluate(task,
            "Vector3D.prototype=Object.getPrototypeOf(_gfosOoliteVectorProbe);"
            "delete this._gfosOoliteVectorProbe;",
            -1,(CONST_STRPTR)"oolite-vector-v1-prototype.js",NULL);
        if(rc!=JSERR_OK)goto done;
    }
    rc=JS_Evaluate(task,
        "Object.defineProperty(Vector3D.prototype,'constructor',"
        "{value:Vector3D,enumerable:false,writable:true,configurable:true});",
        -1,(CONST_STRPTR)"oolite-vector-prototype.js",NULL);
    if(rc!=JSERR_OK)goto done;
    interpolator=JS_NewFunction(task,(CONST_STRPTR)"interpolate",interpolate,NULL);
    if(!interpolator){rc=JSERR_NOMEMORY;goto done;}
    rc=JS_SetProp(task,constructor,(CONST_STRPTR)"interpolate",interpolator);
done:
    if(interpolator)JS_Free(task,interpolator);
    if(constructor)JS_Free(task,constructor);
    if(existing)JS_Free(task,existing);
    if(prototype)JS_Free(task,prototype);
    if(probe)JS_Free(task,probe);
    if(native)JS_Free(task,native);
    JS_Free(task,global);
    return rc;
}
