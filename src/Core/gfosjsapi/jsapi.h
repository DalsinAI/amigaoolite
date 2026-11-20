/*
 * gfosjsapi: the SpiderMonkey 1.8.5 API, as Oolite uses it, implemented on
 * GFOS javascript.library (JavaScriptCore). No SpiderMonkey code: only the
 * names and the calling conventions of its public API, so Oolite's scripting
 * sources compile unchanged (Dale, 19 Nov 2026: "API names may stay").
 *
 * Everything runs on the JSTask's own thread (the game is posted there), so
 * every call is direct. Values:
 *   - jsval is 64 bits: int32, double, boolean, null and undefined are held
 *     inline; strings and objects hold a javascript.library JSValue handle.
 *   - JSObject* and JSString* are those handles (JavaScriptCore cell
 *     pointers: one pointer per object, so identity compares work).
 *   - Handles made while native code runs live in the current arena and are
 *     let go when it ends; locations given to JS_Add*Root keep theirs.
 *
 * Team 5, 19 Nov 2026. MIT.
 */
#ifndef GFOSJSAPI_JSAPI_H
#define GFOSJSAPI_JSAPI_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GFOSJSAPI 1
#define JS_VERSION 185
#define JS_THREADSAFE 1
#define JS_PUBLIC_API(t) t
#define JS_FRIEND_API(t) t
#define JS_BEGIN_EXTERN_C
#define JS_END_EXTERN_C

/* ---- basic types (as SpiderMonkey 1.8.5) --------------------------------- */
typedef int JSBool;
#define JS_TRUE  1
#define JS_FALSE 0
typedef int intN;
typedef unsigned int uintN;
typedef int8_t int8;   typedef uint8_t uint8;
typedef int16_t int16; typedef uint16_t uint16;
typedef int32_t int32; typedef uint32_t uint32;
typedef int64_t int64; typedef uint64_t uint64;
typedef double jsdouble;
typedef int32_t jsint;
typedef uint32_t jsuint;
typedef uint16_t jschar;
typedef int32_t jsrefcount;
typedef uint8_t jsbytecode;
typedef uint32_t jsword;
typedef int JSVersion;
#define JSVERSION_ECMA_5 185
#define JSVERSION_1_8 180
#define JSVERSION_DEFAULT 0
#define JSVERSION_LATEST JSVERSION_ECMA_5

typedef struct JSContext JSContext;
typedef struct JSContext JSRuntime;          /* one runtime per context here */
typedef struct JSObject JSObject;            /* a javascript.library JSValue (never defined) */
typedef struct JSString JSString;
typedef struct JSObject JSFunction;          /* functions are objects */
typedef struct OOJSScriptRec JSScript;
typedef struct JSPrincipals JSPrincipals;
typedef struct JSStackFrame JSStackFrame;
typedef struct JSXDRState JSXDRState;
typedef struct JSExceptionState JSExceptionState;
typedef struct JSPropertyDescArray { uint32 length; void *array; } JSPropertyDescArray;
typedef enum JSXDRMode { JSXDR_ENCODE, JSXDR_DECODE, JSXDR_FREE } JSXDRMode;
typedef enum JSGCRootType { JS_GC_ROOT_VALUE_PTR, JS_GC_ROOT_GCTHING_PTR } JSGCRootType;
typedef enum JSGCParamKey { JSGC_MAX_BYTES = 0, JSGC_MAX_MALLOC_BYTES = 1, JSGC_STACKPOOL_LIFESPAN = 2,
    JSGC_TRIGGER_FACTOR = 3, JSGC_BYTES = 4, JSGC_NUMBER = 5, JSGC_MAX_CODE_CACHE_BYTES = 6, JSGC_MODE = 7 } JSGCParamKey;
typedef enum JSType { JSTYPE_VOID, JSTYPE_OBJECT, JSTYPE_FUNCTION, JSTYPE_STRING, JSTYPE_NUMBER,
    JSTYPE_BOOLEAN, JSTYPE_NULL, JSTYPE_XML, JSTYPE_LIMIT } JSType;
typedef enum JSIterateOp { JSENUMERATE_INIT, JSENUMERATE_INIT_ALL, JSENUMERATE_NEXT, JSENUMERATE_DESTROY } JSIterateOp;
typedef enum JSWhyMagic { JS_ARRAY_HOLE, JS_ARGS_HOLE, JS_NATIVE_ENUMERATE, JS_NO_ITER_VALUE,
    JS_GENERATOR_CLOSING, JS_NO_CONSTANT, JS_THIS_POISON, JS_ARG_POISON, JS_SERIALIZE_NO_NODE,
    JS_IS_CONSTRUCTING, JS_GENERIC_MAGIC } JSWhyMagic;

/* ---- jsval: 64 bits, NaN-boxed ------------------------------------------- */
typedef uint64_t jsval;

#define OOJSV_TAG_SHIFT     32
#define OOJSV_TAG_INT32     0xFFF90000u
#define OOJSV_TAG_UNDEFINED 0xFFF90001u
#define OOJSV_TAG_NULL      0xFFF90002u
#define OOJSV_TAG_BOOLEAN   0xFFF90003u
#define OOJSV_TAG_MAGIC     0xFFF90004u
#define OOJSV_TAG_STRING    0xFFF90005u
#define OOJSV_TAG_OBJECT    0xFFF90006u
#define OOJSV_TAG_PRIVATE   0xFFF90007u
#define OOJSV_TAG(v)        ((uint32_t)((v) >> OOJSV_TAG_SHIFT))
#define OOJSV_PAYLOAD(v)    ((uint32_t)(v))
#define OOJSV_MAKE(t, p)    (((jsval)(uint32_t)(t) << OOJSV_TAG_SHIFT) | (jsval)(uint32_t)(p))

#define JSVAL_VOID   OOJSV_MAKE(OOJSV_TAG_UNDEFINED, 0)
#define JSVAL_NULL   OOJSV_MAKE(OOJSV_TAG_NULL, 0)
#define JSVAL_FALSE  OOJSV_MAKE(OOJSV_TAG_BOOLEAN, 0)
#define JSVAL_TRUE   OOJSV_MAKE(OOJSV_TAG_BOOLEAN, 1)
#define JSVAL_ZERO   OOJSV_MAKE(OOJSV_TAG_INT32, 0)
#define JSVAL_ONE    OOJSV_MAKE(OOJSV_TAG_INT32, 1)
#define JSVAL_INT_MIN ((jsint)0x80000000)
#define JSVAL_INT_MAX ((jsint)0x7fffffff)
#define JSVAL_INT_BITS 32

static inline JSBool JSVAL_IS_DOUBLE(jsval v)   { return OOJSV_TAG(v) < OOJSV_TAG_INT32; }
static inline JSBool JSVAL_IS_INT(jsval v)      { return OOJSV_TAG(v) == OOJSV_TAG_INT32; }
static inline JSBool JSVAL_IS_NUMBER(jsval v)   { return OOJSV_TAG(v) <= OOJSV_TAG_INT32; }
static inline JSBool JSVAL_IS_VOID(jsval v)     { return OOJSV_TAG(v) == OOJSV_TAG_UNDEFINED; }
static inline JSBool JSVAL_IS_NULL(jsval v)     { return OOJSV_TAG(v) == OOJSV_TAG_NULL; }
static inline JSBool JSVAL_IS_BOOLEAN(jsval v)  { return OOJSV_TAG(v) == OOJSV_TAG_BOOLEAN; }
static inline JSBool JSVAL_IS_STRING(jsval v)   { return OOJSV_TAG(v) == OOJSV_TAG_STRING; }
/* SpiderMonkey: null is an object value */
static inline JSBool JSVAL_IS_OBJECT(jsval v)   { return OOJSV_TAG(v) == OOJSV_TAG_OBJECT || OOJSV_TAG(v) == OOJSV_TAG_NULL; }
static inline JSBool JSVAL_IS_PRIMITIVE(jsval v){ return OOJSV_TAG(v) != OOJSV_TAG_OBJECT; }
static inline JSBool JSVAL_IS_GCTHING(jsval v)  { return OOJSV_TAG(v) == OOJSV_TAG_OBJECT || OOJSV_TAG(v) == OOJSV_TAG_STRING; }
static inline JSBool JSVAL_IS_MAGIC_IMPL(jsval v){ return OOJSV_TAG(v) == OOJSV_TAG_MAGIC; }

static inline jsint     JSVAL_TO_INT(jsval v)     { return (jsint)OOJSV_PAYLOAD(v); }
static inline JSBool    JSVAL_TO_BOOLEAN(jsval v) { return (JSBool)OOJSV_PAYLOAD(v); }
static inline JSObject *JSVAL_TO_OBJECT(jsval v)  { return OOJSV_TAG(v) == OOJSV_TAG_OBJECT ? (JSObject *)(uintptr_t)OOJSV_PAYLOAD(v) : NULL; }
static inline JSString *JSVAL_TO_STRING(jsval v)  { return (JSString *)(uintptr_t)OOJSV_PAYLOAD(v); }
static inline void     *JSVAL_TO_GCTHING(jsval v) { return (void *)(uintptr_t)OOJSV_PAYLOAD(v); }
static inline void     *JSVAL_TO_PRIVATE(jsval v) { return (void *)(uintptr_t)OOJSV_PAYLOAD(v); }
static inline jsdouble  JSVAL_TO_DOUBLE(jsval v)  { union { jsval v; jsdouble d; } u; u.v = v; return u.d; }

static inline jsval INT_TO_JSVAL(int32 i)          { return OOJSV_MAKE(OOJSV_TAG_INT32, (uint32_t)i); }
static inline jsval BOOLEAN_TO_JSVAL(JSBool b)     { return OOJSV_MAKE(OOJSV_TAG_BOOLEAN, b ? 1 : 0); }
static inline jsval OBJECT_TO_JSVAL(JSObject *o)   { return o ? OOJSV_MAKE(OOJSV_TAG_OBJECT, (uintptr_t)o) : JSVAL_NULL; }
static inline jsval STRING_TO_JSVAL(JSString *s)   { return OOJSV_MAKE(OOJSV_TAG_STRING, (uintptr_t)s); }
static inline jsval PRIVATE_TO_JSVAL(void *p)      { return OOJSV_MAKE(OOJSV_TAG_PRIVATE, (uintptr_t)p); }
static inline jsval DOUBLE_TO_JSVAL(jsdouble d)
{
    union { jsval v; jsdouble d; } u;
    u.d = d;
    if (d != d || OOJSV_TAG(u.v) >= OOJSV_TAG_INT32) u.v = 0x7FF8000000000000ULL;  /* one canonical NaN */
    return u.v;
}
static inline JSBool INT_FITS_IN_JSVAL(jsint i) { (void)i; return JS_TRUE; }
#define JSVAL_TO_IMPL(v) (v)
#define IMPL_TO_JSVAL(v) (v)

/* ---- jsid: 32 bits ------------------------------------------------------- */
typedef uint32_t jsid;
/* int ids: (i << 1) | 1; string ids: an interned JSString* (aligned); specials */
#define JSID_VOID     ((jsid)0x2u)
#define JSID_EMPTY    ((jsid)0x6u)
#define JSID_BITS(id) ((uint32_t)(id))
static inline JSBool  JSID_IS_INT(jsid id)       { return (id & 1u) != 0; }
static inline jsint   JSID_TO_INT(jsid id)       { return (jsint)((int32_t)id >> 1); }
static inline jsid    INT_TO_JSID(jsint i)       { return ((jsid)i << 1) | 1u; }
static inline JSBool  JSID_IS_VOID(jsid id)      { return id == JSID_VOID; }
static inline JSBool  JSID_IS_EMPTY(jsid id)     { return id == JSID_EMPTY; }
static inline JSBool  JSID_IS_STRING(jsid id)    { return (id & 3u) == 0 && id != 0; }
static inline JSString *JSID_TO_STRING(jsid id)  { return (JSString *)(uintptr_t)id; }
static inline JSBool  JSID_IS_ZERO(jsid id)      { return id == INT_TO_JSID(0); }
static inline JSBool  JSID_IS_OBJECT(jsid id)    { (void)id; return JS_FALSE; }
static inline JSObject *JSID_TO_OBJECT(jsid id)  { (void)id; return NULL; }
static inline JSBool  JSID_IS_DEFAULT_XML_NAMESPACE(jsid id) { (void)id; return JS_FALSE; }

/* an interned JSString* is its own id */
#define INTERNED_STRING_TO_JSID(str) ((jsid)(uintptr_t)(str))

typedef struct JSIdArray { jsint length; jsid vector[1]; } JSIdArray;

/* ---- classes, specs, callbacks ------------------------------------------- */
typedef JSBool (*JSNative)(JSContext *cx, uintN argc, jsval *vp);
typedef JSBool (*JSPropertyOp)(JSContext *cx, JSObject *obj, jsid id, jsval *vp);
typedef JSBool (*JSStrictPropertyOp)(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp);
typedef JSBool (*JSEnumerateOp)(JSContext *cx, JSObject *obj);
typedef JSBool (*JSNewEnumerateOp)(JSContext *cx, JSObject *obj, JSIterateOp op, jsval *statep, jsid *idp);
typedef JSBool (*JSResolveOp)(JSContext *cx, JSObject *obj, jsid id);
typedef JSBool (*JSNewResolveOp)(JSContext *cx, JSObject *obj, jsid id, uintN flags, JSObject **objp);
typedef JSBool (*JSConvertOp)(JSContext *cx, JSObject *obj, JSType type, jsval *vp);
typedef void   (*JSFinalizeOp)(JSContext *cx, JSObject *obj);
typedef void   (*JSClassInternal)(void);
typedef JSBool (*JSHasInstanceOp)(JSContext *cx, JSObject *obj, const jsval *v, JSBool *bp);
typedef uint32 (*JSMarkOp)(JSContext *cx, JSObject *obj, void *arg);
typedef JSBool (*JSCheckAccessOp)(JSContext *cx, JSObject *obj, jsid id, int mode, jsval *vp);
typedef JSObject *(*JSXDRObjectOp)(JSXDRState *xdr, JSObject **objp);

typedef struct JSClass {
    const char         *name;
    uint32              flags;
    JSPropertyOp        addProperty;
    JSPropertyOp        delProperty;
    JSPropertyOp        getProperty;
    JSStrictPropertyOp  setProperty;
    JSEnumerateOp       enumerate;
    JSResolveOp         resolve;
    JSConvertOp         convert;
    JSFinalizeOp        finalize;
    JSClassInternal     reserved0;
    JSCheckAccessOp     checkAccess;
    JSNative            call;
    JSNative            construct;
    JSXDRObjectOp       xdrObject;
    JSHasInstanceOp     hasInstance;
    JSMarkOp            mark;
    JSClassInternal     reserved1;
    void               *reserved[19];
} JSClass;

#define JSCLASS_HAS_PRIVATE             (1u << 0)
#define JSCLASS_NEW_ENUMERATE           (1u << 1)
#define JSCLASS_NEW_RESOLVE             (1u << 2)
#define JSCLASS_PRIVATE_IS_NSISUPPORTS  (1u << 3)
#define JSCLASS_CONSTRUCT_PROTOTYPE     (1u << 6)
#define JSCLASS_GLOBAL_FLAGS            (1u << 9)
#define JSCLASS_HAS_RESERVED_SLOTS(n)   ((uint32)(n) << 10)
#define JSCLASS_NO_OPTIONAL_MEMBERS     0,0,0,0,0,0,0,0,{0}  /* reserved0..reserved1, reserved[] */
#define JSCLASS_NO_INTERNAL_MEMBERS     {0}

typedef struct JSPropertySpec {
    const char         *name;
    int8                tinyid;
    uint8               flags;
    JSPropertyOp        getter;
    JSStrictPropertyOp  setter;
} JSPropertySpec;

typedef struct JSFunctionSpec {
    const char *name;
    JSNative    call;
    uint16      nargs;
    uint16      flags;
} JSFunctionSpec;
#define JS_FS(name, call, nargs, flags) { name, call, nargs, flags }
#define JS_FN(name, call, nargs, flags) { name, call, nargs, flags }
#define JS_FS_END { NULL, NULL, 0, 0 }

#define JSPROP_ENUMERATE   0x01
#define JSPROP_READONLY    0x02
#define JSPROP_PERMANENT   0x04
#define JSPROP_GETTER      0x10
#define JSPROP_SETTER      0x20
#define JSPROP_SHARED      0x40
#define JSPROP_INDEX       0x80
#define JSFUN_LAMBDA       0x08
#define JSFUN_HEAVYWEIGHT  0x80
#define JSFUN_CONSTRUCTOR  0x200
#define JSFUN_STUB_GSOPS   0x1000
#define JSCONTEXT_NEW      0
#define JSCONTEXT_DESTROY  1
#define JSREG_FOLD         0x01
#define JSREG_GLOB         0x02
#define JSREG_MULTILINE    0x04
#define JSREG_STICKY       0x08
#define JSRESOLVE_QUALIFIED 0x01
#define JSRESOLVE_ASSIGNING 0x02

typedef struct JSErrorReport {
    const char   *filename;
    uintN         lineno;
    const char   *linebuf;
    const char   *tokenptr;
    const jschar *uclinebuf;
    const jschar *uctokenptr;
    uintN         flags;
    uintN         errorNumber;
    const jschar *ucmessage;
    const jschar **messageArgs;
} JSErrorReport;
#define JSREPORT_ERROR     0x0
#define JSREPORT_WARNING   0x1
#define JSREPORT_EXCEPTION 0x2
#define JSREPORT_STRICT    0x4
#define JSREPORT_IS_WARNING(f)   (((f) & JSREPORT_WARNING) != 0)
#define JSREPORT_IS_EXCEPTION(f) (((f) & JSREPORT_EXCEPTION) != 0)
#define JSREPORT_IS_STRICT(f)    (((f) & JSREPORT_STRICT) != 0)
typedef void (*JSErrorReporter)(JSContext *cx, const char *message, JSErrorReport *report);
typedef JSBool (*JSContextCallback)(JSContext *cx, uintN contextOp);
typedef JSBool (*JSOperationCallback)(JSContext *cx);
typedef void (*JSFunctionCallback)(const JSFunction *fun, const JSScript *scr, const JSContext *cx, JSBool entering);
typedef int (*JSDebuggerHandler)(JSContext *cx, JSScript *script, jsbytecode *pc, jsval *rval, void *closure);

#define JSOPTION_STRICT      (1u << 0)
#define JSOPTION_WERROR      (1u << 1)
#define JSOPTION_VAROBJFIX   (1u << 2)
#define JSOPTION_RELIMIT     (1u << 9)
#define JSOPTION_ANONFUNFIX  (1u << 10)
#define JSOPTION_JIT         (1u << 11)
#define JSOPTION_METHODJIT   (1u << 14)
#define JSOPTION_PROFILING   (1u << 15)

/* ---- natives' vp ------------------------------------------------------------ */
#define JS_CALLEE(cx, vp)         ((vp)[0])
#define JS_THIS(cx, vp)           ((vp)[1])
#define JS_ARGV(cx, vp)           ((vp) + 2)
#define JS_RVAL(cx, vp)           (*(vp))
#define JS_SET_RVAL(cx, vp, v)    (*(vp) = (v))
#define JS_IsConstructing(cx, vp) (OOJSV_TAG((vp)[1]) == OOJSV_TAG_MAGIC && OOJSV_PAYLOAD((vp)[1]) == JS_IS_CONSTRUCTING)
JSObject *JS_THIS_OBJECT(JSContext *cx, jsval *vp);

/* ---- the API --------------------------------------------------------------- */
JSRuntime *JS_NewRuntime(uint32 maxbytes);
void JS_DestroyRuntime(JSRuntime *rt);
JSContext *JS_NewContext(JSRuntime *rt, size_t stackChunkSize);
void JS_DestroyContext(JSContext *cx);
JSRuntime *JS_GetRuntime(JSContext *cx);
JSContextCallback JS_SetContextCallback(JSRuntime *rt, JSContextCallback cxCallback);
void JS_BeginRequest(JSContext *cx);
void JS_EndRequest(JSContext *cx);
JSBool JS_IsInRequest(JSContext *cx);
jsrefcount JS_SuspendRequest(JSContext *cx);
void JS_ResumeRequest(JSContext *cx, jsrefcount saveDepth);
JSBool JS_EnterLocalRootScope(JSContext *cx);
void JS_LeaveLocalRootScope(JSContext *cx);
void JS_LeaveLocalRootScopeWithResult(JSContext *cx, jsval rval);
uint32 JS_GetOptions(JSContext *cx);
uint32 JS_SetOptions(JSContext *cx, uint32 options);
JSVersion JS_GetVersion(JSContext *cx);
JSVersion JS_SetVersion(JSContext *cx, JSVersion version);
const char *JS_VersionToString(JSVersion version);
void JS_SetCStringsAreUTF8(void);
JSErrorReporter JS_SetErrorReporter(JSContext *cx, JSErrorReporter er);
JSOperationCallback JS_SetOperationCallback(JSContext *cx, JSOperationCallback callback);
void JS_TriggerAllOperationCallbacks(JSRuntime *rt);
void JS_SetFunctionCallback(JSContext *cx, JSFunctionCallback fcb);
void JS_SetGCZeal(JSContext *cx, uint8 zeal);
JSBool JS_SetDebuggerHandler(JSRuntime *rt, JSDebuggerHandler hook, void *closure);

JSObject *JS_NewCompartmentAndGlobalObject(JSContext *cx, JSClass *clasp, JSPrincipals *principals);
void JS_SetGlobalObject(JSContext *cx, JSObject *obj);
JSBool JS_InitStandardClasses(JSContext *cx, JSObject *obj);
void JS_GC(JSContext *cx);
void JS_MaybeGC(JSContext *cx);
uint32 JS_GetGCParameter(JSRuntime *rt, JSGCParamKey key);
int64 JS_Now(void);

JSBool JS_AddValueRoot(JSContext *cx, jsval *vp);
JSBool JS_AddStringRoot(JSContext *cx, JSString **rp);
JSBool JS_AddObjectRoot(JSContext *cx, JSObject **rp);
JSBool JS_AddGCThingRoot(JSContext *cx, void **rp);
JSBool JS_AddNamedValueRoot(JSContext *cx, jsval *vp, const char *name);
JSBool JS_AddNamedStringRoot(JSContext *cx, JSString **rp, const char *name);
JSBool JS_AddNamedObjectRoot(JSContext *cx, JSObject **rp, const char *name);
JSBool JS_AddNamedGCThingRoot(JSContext *cx, void **rp, const char *name);
JSBool JS_RemoveValueRoot(JSContext *cx, jsval *vp);
JSBool JS_RemoveStringRoot(JSContext *cx, JSString **rp);
JSBool JS_RemoveObjectRoot(JSContext *cx, JSObject **rp);
JSBool JS_RemoveGCThingRoot(JSContext *cx, void **rp);
void JS_DumpNamedRoots(JSRuntime *rt, void (*dump)(const char *name, void *rp, JSGCRootType type, void *data), void *data);
JSBool JS_DumpHeap(JSContext *cx, FILE *fp, void *startThing, uint32 startKind, void *thingToFind, size_t maxDepth, void *thingToIgnore);

JSBool JS_NewNumberValue(JSContext *cx, jsdouble d, jsval *rval);
JSBool JS_ValueToNumber(JSContext *cx, jsval v, jsdouble *dp);
JSBool JS_ValueToInt32(JSContext *cx, jsval v, int32 *ip);
JSBool JS_ValueToECMAUint32(JSContext *cx, jsval v, uint32 *ip);
JSBool JS_ValueToBoolean(JSContext *cx, jsval v, JSBool *bp);
JSBool JS_ValueToObject(JSContext *cx, jsval v, JSObject **objp);
JSString *JS_ValueToString(JSContext *cx, jsval v);
JSFunction *JS_ValueToFunction(JSContext *cx, jsval v);
JSBool JS_ValueToId(JSContext *cx, jsval v, jsid *idp);
JSBool JS_IdToValue(JSContext *cx, jsid id, jsval *vp);
JSType JS_TypeOfValue(JSContext *cx, jsval v);
jsval JS_GetEmptyStringValue(JSContext *cx);

JSString *JS_NewStringCopyZ(JSContext *cx, const char *s);
JSString *JS_NewUCStringCopyN(JSContext *cx, const jschar *s, size_t n);
JSString *JS_InternString(JSContext *cx, const char *s);
JSString *JS_InternUCStringN(JSContext *cx, const jschar *s, size_t length);
JSBool JS_StringHasBeenInterned(JSString *str);
const jschar *JS_GetInternedStringChars(JSString *str);
const jschar *JS_GetStringCharsAndLength(JSContext *cx, JSString *str, size_t *length);
size_t JS_GetStringLength(JSString *str);
JSBool JS_CompareStrings(JSContext *cx, JSString *str1, JSString *str2, int32 *result);
JSBool JS_StringEqualsAscii(JSContext *cx, JSString *str, const char *asciiBytes, JSBool *match);
/* gfosjsapi extra: UTF-8 out (malloc'd; free() it) */
char *OOJSStringToUTF8(JSContext *cx, JSString *str);

JSObject *JS_NewObject(JSContext *cx, JSClass *clasp, JSObject *proto, JSObject *parent);
JSObject *JS_DefineObject(JSContext *cx, JSObject *obj, const char *name, JSClass *clasp, JSObject *proto, uintN attrs);
JSObject *JS_NewArrayObject(JSContext *cx, jsint length, jsval *vector);
JSBool JS_IsArrayObject(JSContext *cx, JSObject *obj);
JSBool JS_ObjectIsArray(JSContext *cx, JSObject *obj);
JSBool JS_GetArrayLength(JSContext *cx, JSObject *obj, jsuint *lengthp);
JSBool JS_ObjectIsFunction(JSContext *cx, JSObject *obj);
JSBool JS_FreezeObject(JSContext *cx, JSObject *obj);
JSClass *JS_GetClass(JSContext *cx, JSObject *obj);
void *JS_GetPrivate(JSContext *cx, JSObject *obj);
JSBool JS_SetPrivate(JSContext *cx, JSObject *obj, void *data);
void *JS_GetInstancePrivate(JSContext *cx, JSObject *obj, JSClass *clasp, jsval *argv);
JSBool JS_InstanceOf(JSContext *cx, JSObject *obj, JSClass *clasp, jsval *argv);
JSObject *JS_InitClass(JSContext *cx, JSObject *obj, JSObject *parent_proto, JSClass *clasp, JSNative constructor,
    uintN nargs, JSPropertySpec *ps, JSFunctionSpec *fs, JSPropertySpec *static_ps, JSFunctionSpec *static_fs);
void JS_ClearScope(JSContext *cx, JSObject *obj);
JSIdArray *JS_Enumerate(JSContext *cx, JSObject *obj);
void JS_DestroyIdArray(JSContext *cx, JSIdArray *ida);

JSBool JS_GetProperty(JSContext *cx, JSObject *obj, const char *name, jsval *vp);
JSBool JS_SetProperty(JSContext *cx, JSObject *obj, const char *name, jsval *vp);
JSBool JS_DeleteProperty(JSContext *cx, JSObject *obj, const char *name);
JSBool JS_GetPropertyById(JSContext *cx, JSObject *obj, jsid id, jsval *vp);
JSBool JS_SetPropertyById(JSContext *cx, JSObject *obj, jsid id, jsval *vp);
JSBool JS_LookupPropertyById(JSContext *cx, JSObject *obj, jsid id, jsval *vp);
JSBool JS_GetMethodById(JSContext *cx, JSObject *obj, jsid id, JSObject **objp, jsval *vp);
JSBool JS_DefineProperty(JSContext *cx, JSObject *obj, const char *name, jsval value, JSPropertyOp getter,
    JSStrictPropertyOp setter, uintN attrs);
JSBool JS_DefinePropertyById(JSContext *cx, JSObject *obj, jsid id, jsval value, JSPropertyOp getter,
    JSStrictPropertyOp setter, uintN attrs);
JSBool JS_DefineProperties(JSContext *cx, JSObject *obj, JSPropertySpec *ps);
JSBool JS_GetElement(JSContext *cx, JSObject *obj, jsint index, jsval *vp);
JSBool JS_SetElement(JSContext *cx, JSObject *obj, jsint index, jsval *vp);
JSBool JS_LookupElement(JSContext *cx, JSObject *obj, jsint index, jsval *vp);

JSFunction *JS_DefineFunction(JSContext *cx, JSObject *obj, const char *name, JSNative call, uintN nargs, uintN attrs);
JSBool JS_DefineFunctions(JSContext *cx, JSObject *obj, JSFunctionSpec *fs);
JSObject *JS_GetFunctionObject(JSFunction *fun);
JSString *JS_GetFunctionId(JSFunction *fun);
JSNative JS_GetFunctionNative(JSContext *cx, JSFunction *fun);
JSBool JS_CallFunction(JSContext *cx, JSObject *obj, JSFunction *fun, uintN argc, jsval *argv, jsval *rval);
JSBool JS_CallFunctionValue(JSContext *cx, JSObject *obj, jsval fval, uintN argc, jsval *argv, jsval *rval);
JSFunction *JS_CompileUCFunction(JSContext *cx, JSObject *obj, const char *name, uintN nargs, const char **argnames,
    const jschar *chars, size_t length, const char *filename, uintN lineno);
JSScript *JS_CompileUCScript(JSContext *cx, JSObject *obj, const jschar *chars, size_t length, const char *filename,
    uintN lineno);
JSBool JS_ExecuteScript(JSContext *cx, JSObject *obj, JSScript *script, jsval *rval);
JSObject *JS_NewScriptObject(JSContext *cx, JSScript *script);
void JS_DestroyScript(JSContext *cx, JSScript *script);
const char *JS_GetScriptFilename(JSContext *cx, JSScript *script);
JSBool JS_BufferIsCompilableUnit(JSContext *cx, JSObject *obj, const char *bytes, size_t length);
JSObject *JS_NewUCRegExpObjectNoStatics(JSContext *cx, jschar *chars, size_t length, uintN flags);

void JS_ReportError(JSContext *cx, const char *format, ...);
JSBool JS_ReportWarning(JSContext *cx, const char *format, ...);
JSBool JS_ReportPendingException(JSContext *cx);
JSBool JS_IsExceptionPending(JSContext *cx);
JSBool JS_GetPendingException(JSContext *cx, jsval *vp);
void JS_SetPendingException(JSContext *cx, jsval v);
void JS_ClearPendingException(JSContext *cx);
JSExceptionState *JS_SaveExceptionState(JSContext *cx);
void JS_RestoreExceptionState(JSContext *cx, JSExceptionState *state);

/* Stubs for stub slots in JSClass tables */
JSBool JS_PropertyStub(JSContext *cx, JSObject *obj, jsid id, jsval *vp);
JSBool JS_StrictPropertyStub(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp);
JSBool JS_EnumerateStub(JSContext *cx, JSObject *obj);
JSBool JS_ResolveStub(JSContext *cx, JSObject *obj, jsid id);
JSBool JS_ConvertStub(JSContext *cx, JSObject *obj, JSType type, jsval *vp);
void JS_FinalizeStub(JSContext *cx, JSObject *obj);

#ifdef __cplusplus
}
#endif

#endif /* GFOSJSAPI_JSAPI_H */
