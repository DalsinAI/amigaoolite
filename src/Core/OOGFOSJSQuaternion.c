/* Original OOJSQuaternion: scalar native maths, constructors, prototype
 * identity and task-bound ownership over canonical javascript.library.
 * GPL-2.0-or-later. Oolite full game and JSAPI source port not claimed.
 */
#include "OOGFOSJSQuaternion.h"
#include "OOGFOSJSVector.h"
#include <proto/javascript.h>
#include <proto/exec.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
typedef struct {DOUBLE w,x,y,z;} Quat;
static const struct JSHostClass quatClass;
static int qFinite(DOUBLE x){return isfinite(x);}
static JSValue number(JSTask t,DOUBLE n){return JS_NewDouble(t,&n);}
static JSValue get_w(JSTask t,APTR s,APTR d){(void)d;return number(t,((Quat*)s)->w);}
static JSValue get_x(JSTask t,APTR s,APTR d){(void)d;return number(t,((Quat*)s)->x);}
static JSValue get_y(JSTask t,APTR s,APTR d){(void)d;return number(t,((Quat*)s)->y);}
static JSValue get_z(JSTask t,APTR s,APTR d){(void)d;return number(t,((Quat*)s)->z);}
static LONG setComponent(JSTask t,APTR self,JSValue value,unsigned idx)
{
    DOUBLE x;
    Quat *q=(Quat*)self;
    if(!q||JS_ToDouble(t,value,&x)!=JSERR_OK||!qFinite(x)){
        JS_Throw(t,(CONST_STRPTR)"Quaternion component must be finite");
        return JSERR_EXCEPTION;
    }
    switch(idx){case 0:q->w=x;break;case 1:q->x=x;break;
                case 2:q->y=x;break;default:q->z=x;break;}
    return JSERR_OK;
}
static LONG set_w(JSTask t,APTR s,JSValue v,APTR d)
{(void)d;return setComponent(t,s,v,0);}
static LONG set_x(JSTask t,APTR s,JSValue v,APTR d)
{(void)d;return setComponent(t,s,v,1);}
static LONG set_y(JSTask t,APTR s,JSValue v,APTR d)
{(void)d;return setComponent(t,s,v,2);}
static LONG set_z(JSTask t,APTR s,JSValue v,APTR d)
{(void)d;return setComponent(t,s,v,3);}
static void release(JSTask t,APTR obj,APTR d)
{(void)t;(void)d;if(obj)FreeVec(obj);}
static JSValue create(JSTask t,Quat q)
{
    JSClass cls;
    Quat *owned;
    JSValue wrapper;
    if(!t||!qFinite(q.w)||!qFinite(q.x)||!qFinite(q.y)||!qFinite(q.z))return NULL;
    cls=JS_RegisterClass(t,&quatClass);
    if(!cls)return NULL;
    owned=(Quat*)AllocVec(sizeof(*owned),MEMF_ANY);
    if(!owned)return NULL;
    *owned=q;
    wrapper=JS_Wrap(t,cls,owned);
    if(!wrapper)FreeVec(owned);
    return wrapper;
}
JSValue OOGFOSJSQuaternionCreate(JSTask t,DOUBLE w,DOUBLE x,DOUBLE y,DOUBLE z)
{
    Quat q={w,x,y,z};
    return create(t,q);
}
LONG OOGFOSJSQuaternionRead(JSTask t,JSValue value,
                           DOUBLE *w,DOUBLE *x,DOUBLE *y,DOUBLE *z)
{
    JSClass cls;
    Quat *q;
    if(!t||!value||!w||!x||!y||!z)return JSERR_BADARGS;
    cls=JS_RegisterClass(t,&quatClass);
    q=cls?(Quat*)JS_Unwrap(t,value,cls):NULL;
    if(!q)return JSERR_TYPE;
    *w=q->w;*x=q->x;*y=q->y;*z=q->z;
    return JSERR_OK;
}
static Quat *argument(JSTask t,LONG argc,JSValue *argv)
{
    JSClass cls;
    if(argc<1||!argv||!argv[0])return NULL;
    cls=JS_RegisterClass(t,&quatClass);
    return cls?(Quat*)JS_Unwrap(t,argv[0],cls):NULL;
}
static JSValue bad(JSTask t)
{return JS_Throw(t,(CONST_STRPTR)"Quaternion expects a Quaternion");}
static JSValue construct(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Quat q={1.0,0.0,0.0,0.0},*copy;
    DOUBLE *coord=&q.w;
    unsigned i;
    (void)self;(void)data;
    if(argc==1&&(copy=argument(t,argc,argv))!=NULL)return create(t,*copy);
    if(argc!=0&&argc!=4)return bad(t);
    for(i=0;i<(unsigned)argc;i++)
        if(!argv||JS_ToDouble(t,argv[i],&coord[i])!=JSERR_OK||
           !qFinite(coord[i]))
           return JS_Throw(t,(CONST_STRPTR)"Quaternion: non-finite component");
    return create(t,q);
}
static Quat multiplyQuats(Quat a,Quat b)
{
    Quat q={
        a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z,
        a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y,
        a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,
        a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w
    };
    return q;
}
static JSValue multiply(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Quat *rhs=argument(t,argc,argv);
    (void)data;
    return rhs?create(t,multiplyQuats(*(Quat*)self,*rhs)):bad(t);
}
static JSValue dot(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Quat *a=(Quat*)self,*b=argument(t,argc,argv);
    (void)data;
    return b?number(t,a->w*b->w+a->x*b->x+a->y*b->y+a->z*b->z):bad(t);
}
static JSValue conjugate(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Quat q=*(Quat*)self;
    (void)argc;(void)argv;(void)data;
    q.x=-q.x;q.y=-q.y;q.z=-q.z;
    return create(t,q);
}
static JSValue normalized(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Quat q=*(Quat*)self;
    DOUBLE m=hypot(hypot(q.w,q.x),hypot(q.y,q.z));
    (void)argc;(void)argv;(void)data;
    if(m==0.0)return create(t,(Quat){1,0,0,0});
    q.w/=m;q.x/=m;q.y/=m;q.z/=m;
    return create(t,q);
}
static JSValue textQuat(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Quat *q=(Quat*)self;
    char buffer[192];
    int n;
    (void)argc;(void)argv;
    if(data)
       n=snprintf(buffer,sizeof(buffer),"Quaternion(%g, %g, %g, %g)",
                  q->w,q->x,q->y,q->z);
    else
       n=snprintf(buffer,sizeof(buffer),"(%g, %g, %g, %g)",
                  q->w,q->x,q->y,q->z);
    if(n<0||n>=(int)sizeof(buffer))
        return JS_Throw(t,(CONST_STRPTR)"Quaternion text overflow");
    return JS_NewString(t,(CONST_STRPTR)buffer,n);
}
static JSValue toSource(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{(void)data;return textQuat(t,self,argc,argv,(APTR)1);}
static JSValue toArray(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Quat *q=(Quat*)self;
    JSValue array=NULL,n[4]={NULL,NULL,NULL,NULL};
    DOUBLE values[4]={q->w,q->x,q->y,q->z};
    unsigned i;
    int good=1;
    (void)argc;(void)argv;(void)data;
    if(JS_Evaluate(t,(CONST_STRPTR)"[]",-1,
                   (CONST_STRPTR)"oolite-quaternion-array.js",&array)!=JSERR_OK)
        return NULL;
    for(i=0;i<4;i++){
        n[i]=number(t,values[i]);
        if(!n[i]||JS_SetIndex(t,array,i,n[i])!=JSERR_OK){good=0;break;}
    }
    for(i=0;i<4;i++)if(n[i])JS_Free(t,n[i]);
    if(!good){if(array)JS_Free(t,array);return NULL;}
    return array;
}
/* Rotate a pure vector using normalized q*v*conjugate(q). */
static void rotateVector(Quat q,DOUBLE *x,DOUBLE *y,DOUBLE *z)
{
    DOUBLE mag=hypot(hypot(q.w,q.x),hypot(q.y,q.z));
    DOUBLE vx=*x,vy=*y,vz=*z,tx,ty,tz;
    if(mag==0.0)return;
    q.w/=mag;q.x/=mag;q.y/=mag;q.z/=mag;
    tx=2.0*(q.y*vz-q.z*vy);
    ty=2.0*(q.z*vx-q.x*vz);
    tz=2.0*(q.x*vy-q.y*vx);
    *x=vx+q.w*tx+q.y*tz-q.z*ty;
    *y=vy+q.w*ty+q.z*tx-q.x*tz;
    *z=vz+q.w*tz+q.x*ty-q.y*tx;
}
static JSValue vectorDirection(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{
    Quat q=*(Quat*)self;
    DOUBLE x=0,y=0,z=0;
    long axis=(long)data;
    (void)argc;(void)argv;
    if(axis==0)z=1; else if(axis==1)x=1;else y=1;
    /* Oolite forward is -Z; right is +X and up is +Y. */
    if(axis==0)z=-1;
    rotateVector(q,&x,&y,&z);
    return OOGFOSJSVectorCreate(t,x,y,z);
}
static JSValue rotateAxis(JSTask t,APTR self,LONG argc,JSValue *argv,APTR data)
{
    DOUBLE angle,half,s,c;
    Quat r={0,0,0,0};
    long axis=(long)data;
    if(argc<1||!argv||JS_ToDouble(t,argv[0],&angle)!=JSERR_OK||!qFinite(angle))
        return JS_Throw(t,(CONST_STRPTR)"Quaternion rotation needs finite angle");
    half=angle*0.5;s=sin(half);c=cos(half);
    r.w=c;
    if(axis==0)r.x=s;else if(axis==1)r.y=s;else r.z=s;
    return create(t,multiplyQuats(*(Quat*)self,r));
}
static JSValue rotatedX(JSTask t,APTR s,LONG n,JSValue *v,APTR d)
{(void)d;return rotateAxis(t,s,n,v,(APTR)0);}
static JSValue rotatedY(JSTask t,APTR s,LONG n,JSValue *v,APTR d)
{(void)d;return rotateAxis(t,s,n,v,(APTR)1);}
static JSValue rotatedZ(JSTask t,APTR s,LONG n,JSValue *v,APTR d)
{(void)d;return rotateAxis(t,s,n,v,(APTR)2);}
static JSValue directedForward(JSTask t,APTR s,LONG n,JSValue *v,APTR d)
{(void)d;return vectorDirection(t,s,n,v,(APTR)0);}
static JSValue directedRight(JSTask t,APTR s,LONG n,JSValue *v,APTR d)
{(void)d;return vectorDirection(t,s,n,v,(APTR)1);}
static JSValue directedUp(JSTask t,APTR s,LONG n,JSValue *v,APTR d)
{(void)d;return vectorDirection(t,s,n,v,(APTR)2);}
static const struct JSHostSlot slots[]={
    {(CONST_STRPTR)"w",JSS_ACCESSOR,0,(APTR)get_w,(APTR)set_w,0,0},
    {(CONST_STRPTR)"x",JSS_ACCESSOR,0,(APTR)get_x,(APTR)set_x,0,0},
    {(CONST_STRPTR)"y",JSS_ACCESSOR,0,(APTR)get_y,(APTR)set_y,0,0},
    {(CONST_STRPTR)"z",JSS_ACCESSOR,0,(APTR)get_z,(APTR)set_z,0,0},
    {(CONST_STRPTR)"multiply",JSS_METHOD,0,(APTR)multiply,NULL,0,1},
    {(CONST_STRPTR)"dot",JSS_METHOD,0,(APTR)dot,NULL,0,1},
    {(CONST_STRPTR)"normalize",JSS_METHOD,0,(APTR)normalized,NULL,0,0},
    {(CONST_STRPTR)"conjugate",JSS_METHOD,0,(APTR)conjugate,NULL,0,0},
    {(CONST_STRPTR)"toString",JSS_METHOD,0,(APTR)textQuat,NULL,0,0},
    {(CONST_STRPTR)"toSource",JSS_METHOD,0,(APTR)toSource,NULL,0,0},
    {(CONST_STRPTR)"toArray",JSS_METHOD,0,(APTR)toArray,NULL,0,0},
    {(CONST_STRPTR)"rotateX",JSS_METHOD,0,(APTR)rotatedX,NULL,0,1},
    {(CONST_STRPTR)"rotateY",JSS_METHOD,0,(APTR)rotatedY,NULL,0,1},
    {(CONST_STRPTR)"rotateZ",JSS_METHOD,0,(APTR)rotatedZ,NULL,0,1},
    {(CONST_STRPTR)"vectorForward",JSS_METHOD,0,(APTR)directedForward,NULL,0,0},
    {(CONST_STRPTR)"vectorRight",JSS_METHOD,0,(APTR)directedRight,NULL,0,0},
    {(CONST_STRPTR)"vectorUp",JSS_METHOD,0,(APTR)directedUp,NULL,0,0},
    {NULL,0,0,NULL,NULL,0,0}
};
static const struct JSHostClass quatClass={
    (CONST_STRPTR)"OoliteQuaternion",NULL,slots,release,NULL
};
/* The original JSC host function is not constructible: the JS constructor
 * wrapper is required for accurate instanceof and prototype semantics. */
LONG OOGFOSJSQuaternionInstall(JSTask t)
{
    JSValue global=NULL,existing=NULL,ctor=NULL,native=NULL,proto=NULL,probe=NULL;
    JSClass cls;
    LONG rc=JSERR_BADARGS;
    if(!t)return rc;
    cls=JS_RegisterClass(t,&quatClass);
    if(!cls)return JSERR_NOMEMORY;
    global=JS_Global(t);
    if(!global)return rc;
    existing=JS_GetProp(t,global,(CONST_STRPTR)"Quaternion");
    if(existing&&JS_TypeOf(t,existing)!=JST_UNDEFINED){rc=JSERR_TYPE;goto done;}
    native=JS_NewFunction(t,(CONST_STRPTR)"_gfosQuatNative",construct,NULL);
    if(!native){rc=JSERR_NOMEMORY;goto done;}
    rc=JS_SetProp(t,global,(CONST_STRPTR)"_gfosOoliteQuatFactory",native);
    if(rc!=JSERR_OK)goto done;
    rc=JS_Evaluate(t,(CONST_STRPTR)
        "(function(g){g.Quaternion=(function(nativeCreate){"
        "return function Quaternion(){return nativeCreate.apply(null,arguments);};"
        "})(g._gfosOoliteQuatFactory);delete g._gfosOoliteQuatFactory;})(this);",
        -1,(CONST_STRPTR)"oolite-quaternion-constructor.js",NULL);
    if(rc!=JSERR_OK)goto done;
    ctor=JS_GetProp(t,global,(CONST_STRPTR)"Quaternion");
    if(!ctor){rc=JSERR_BADARGS;goto done;}
#if defined(JAVASCRIPT_CLASS_PROTOTYPE_VERSION) && JAVASCRIPT_CLASS_PROTOTYPE_VERSION >= 2
    if(JavaScriptBase&&
       JavaScriptBase->lib_Version>=JAVASCRIPT_CLASS_PROTOTYPE_VERSION)
        proto=JS_ClassPrototype(t,cls);
#endif
    if(!proto){
        probe=create(t,(Quat){1.0,0.0,0.0,0.0});
        if(!probe){rc=JSERR_NOMEMORY;goto done;}
        rc=JS_SetProp(t,global,(CONST_STRPTR)"_gfosOoliteQuatProbe",probe);
        if(rc!=JSERR_OK)goto done;
        rc=JS_Evaluate(t,(CONST_STRPTR)
            "Quaternion.prototype=Object.getPrototypeOf(_gfosOoliteQuatProbe);"
            "delete this._gfosOoliteQuatProbe;",
            -1,(CONST_STRPTR)"oolite-quaternion-v1-prototype.js",NULL);
        if(rc!=JSERR_OK)goto done;
    }else{
        rc=JS_SetProp(t,ctor,(CONST_STRPTR)"prototype",proto);
        if(rc!=JSERR_OK)goto done;
    }
    rc=JS_Evaluate(t,(CONST_STRPTR)
        "Object.defineProperty(Quaternion.prototype,'constructor',"
        "{value:Quaternion,enumerable:false,writable:true,configurable:true});",
        -1,(CONST_STRPTR)"oolite-quaternion-prototype.js",NULL);
done:
    if(probe)JS_Free(t,probe);
    if(proto)JS_Free(t,proto);
    if(native)JS_Free(t,native);
    if(ctor)JS_Free(t,ctor);
    if(existing)JS_Free(t,existing);
    if(global)JS_Free(t,global);
    return rc;
}
