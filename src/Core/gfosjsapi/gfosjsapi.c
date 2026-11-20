/*
 * gfosjsapi: SpiderMonkey 1.8.5's API (as Oolite uses it) on javascript.library.
 * See jsapi.h for the value model. Everything runs on the JSTask's own thread.
 *
 * Lifetimes: every library handle this file makes or receives goes into the
 * arena and is let go when the outermost request (JS_BeginRequest/JS_EndRequest,
 * or a native call) ends. Before that, each location given to JS_Add*Root gets
 * its own hold on whatever handle it now names, so rooted values survive.
 *
 * Team 5, 19 Nov 2026. MIT.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "jsapi.h"
#include "jsxdrapi.h"
#include "oojsl.h"

/* ==== the context ========================================================== */

typedef struct OOBox {
    JSClass *clasp;
    void    *priv;
} OOBox;

typedef struct OORoot { void *loc; int kind; ooLValue pinned; const char *name; } OORoot;
enum { ROOT_VALUE, ROOT_THING };

typedef struct OOClassRec { struct OOClassRec *next; JSClass *clasp; ooLValue proto; ooLValue ctor; } OOClassRec;

typedef struct OONativeRec { struct OONativeRec *next; JSNative fn; uint16 nargs; ooLValue callee; const char *name; } OONativeRec;

typedef struct OOAccessorRec {
    JSPropertyOp       getter;     /* NULL: the object's class getProperty */
    JSStrictPropertyOp setter;     /* NULL: the object's class setProperty */
    jsid               id;
    uint8              flags;
} OOAccessorRec;

struct JSContext {
    int                 ready;
    JSErrorReporter     reporter;
    JSContextCallback   cxCallback;
    JSOperationCallback opCallback;
    uint32              options;
    JSVersion           version;
    int                 nest;          /* requests + native calls in progress */
    int                 nativeDepth;
    int                 suspended;
    int                 hasPending;
    jsval               pending;
    ooLValue            pendingPin;
    JSObject           *global;
    JSClass            *globalClass;
    OOBox              *finalizing;    /* the box whose finalizer runs now */
    /* arena */
    ooLValue           *arena; int arenaCount, arenaCap;
    void              **mallocs; int mallocCount, mallocCap;
    /* roots */
    OORoot             *roots; int rootCount, rootCap;
    OOClassRec         *classes;
    OONativeRec        *natives;
    /* glue (JavaScript helpers) and the exception sentinel */
    ooLValue gSent, gMakeNative, gDefineAccessor, gDefineValue, gSetProto, gIsArray, gFreeze, gKeys, gDel,
             gNewError, gToStr, gToObj, gNewArray, gRegExp, gObjectProto, gEmpty, gExecIn, gClearScope;
};

static struct JSContext sContext;
#define CX (&sContext)
#define FINALIZING_OBJ ((JSObject *)(uintptr_t)0x0F1A1150u)

static JSClass sObjectClass   = { "Object",   0, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
                                  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS };
static JSClass sFunctionClass = { "Function", 0, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
                                  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS };
static JSClass sArrayClass    = { "Array",    0, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
                                  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS };

static void *xgrow(void *p, int *cap, int need, size_t size)
{
    if (need <= *cap) return p;
    int n = *cap ? *cap * 2 : 64;
    while (n < need) n *= 2;
    void *q = realloc(p, (size_t)n * size);
    if (!q) { fprintf(stderr, "gfosjsapi: out of memory\n"); abort(); }
    *cap = n;
    return q;
}

static ooLValue keep(ooLValue v)            /* into the arena */
{
    if (!v) return NULL;
    CX->arena = xgrow(CX->arena, &CX->arenaCap, CX->arenaCount + 1, sizeof *CX->arena);
    CX->arena[CX->arenaCount++] = v;
    return v;
}

static void *keepMalloc(void *p)
{
    if (!p) return NULL;
    CX->mallocs = xgrow(CX->mallocs, &CX->mallocCap, CX->mallocCount + 1, sizeof *CX->mallocs);
    CX->mallocs[CX->mallocCount++] = p;
    return p;
}

static ooLValue handleOfJsval(jsval v)
{
    return JSVAL_IS_GCTHING(v) ? (ooLValue)JSVAL_TO_GCTHING(v) : NULL;
}

static ooLValue rootHandle(const OORoot *r)
{
    return r->kind == ROOT_VALUE ? handleOfJsval(*(jsval *)r->loc) : *(ooLValue *)r->loc;
}

/* Rooted locations take their own hold on what they name now; then the
 * arena lets go. Runs when the outermost request or native call ends. */
static void flush(void)
{
    int i;
    for (i = 0; i < CX->rootCount; i++) {
        OORoot *r = &CX->roots[i];
        ooLValue h = rootHandle(r);
        if (h != r->pinned) {
            if (h) ooL_Dup(h);
            if (r->pinned) ooL_Free(r->pinned);
            r->pinned = h;
        }
    }
    for (i = 0; i < CX->arenaCount; i++) ooL_Free(CX->arena[i]);
    CX->arenaCount = 0;
    for (i = 0; i < CX->mallocCount; i++) free(CX->mallocs[i]);
    CX->mallocCount = 0;
}

static void enter(void) { CX->nest++; }
static void leave(void) { if (--CX->nest <= 0) { CX->nest = 0; flush(); } }

/* ---- values ------------------------------------------------------------------ */

static jsval numberJsval(double d)
{
    if (d == (double)(int32_t)d && !(d == 0 && signbit(d))) return INT_TO_JSVAL((int32_t)d);
    return DOUBLE_TO_JSVAL(d);
}

/* A library value we own -> jsval (gc things go into the arena). */
static jsval fromOwned(ooLValue v)
{
    double d;
    jsval r;
    switch (ooL_TypeOf(v)) {
    case OOL_UNDEFINED: ooL_Free(v); return JSVAL_VOID;
    case OOL_NULL:      ooL_Free(v); return JSVAL_NULL;
    case OOL_BOOLEAN:   r = BOOLEAN_TO_JSVAL(ooL_ToBool(v)); ooL_Free(v); return r;
    case OOL_NUMBER:    ooL_ToDouble(v, &d); ooL_Free(v); return numberJsval(d);
    case OOL_STRING:    return STRING_TO_JSVAL((JSString *)keep(v));
    default:            return OBJECT_TO_JSVAL((JSObject *)keep(v));
    }
}

/* A library value lent to us (arguments): take our own hold, then as owned. */
static jsval fromBorrowed(ooLValue v)
{
    return v ? fromOwned(ooL_Dup(v)) : JSVAL_VOID;
}

/* jsval -> a library value valid until the arena flushes (borrowed). */
static ooLValue toL(jsval v)
{
    switch (OOJSV_TAG(v)) {
    case OOJSV_TAG_INT32:     return keep(ooL_NewLong(JSVAL_TO_INT(v)));
    case OOJSV_TAG_UNDEFINED: return NULL;
    case OOJSV_TAG_NULL:      return keep(ooL_NewNull());
    case OOJSV_TAG_BOOLEAN:   return keep(ooL_NewBool(JSVAL_TO_BOOLEAN(v)));
    case OOJSV_TAG_STRING:
    case OOJSV_TAG_OBJECT:    return (ooLValue)JSVAL_TO_GCTHING(v);
    case OOJSV_TAG_MAGIC:
    case OOJSV_TAG_PRIVATE:   return NULL;
    default:                  return keep(ooL_NewDouble(JSVAL_TO_DOUBLE(v)));
    }
}

/* jsval -> a new library value the caller owns (handed to the library). */
static ooLValue toLNew(jsval v)
{
    switch (OOJSV_TAG(v)) {
    case OOJSV_TAG_INT32:     return ooL_NewLong(JSVAL_TO_INT(v));
    case OOJSV_TAG_UNDEFINED: return NULL;
    case OOJSV_TAG_NULL:      return ooL_NewNull();
    case OOJSV_TAG_BOOLEAN:   return ooL_NewBool(JSVAL_TO_BOOLEAN(v));
    case OOJSV_TAG_STRING:
    case OOJSV_TAG_OBJECT:    return ooL_Dup((ooLValue)JSVAL_TO_GCTHING(v));
    case OOJSV_TAG_MAGIC:
    case OOJSV_TAG_PRIVATE:   return NULL;
    default:                  return ooL_NewDouble(JSVAL_TO_DOUBLE(v));
    }
}

/* ---- glue: JavaScript helpers made once ---------------------------------------- */

static const char sGlue[] =
"(function(){\n"
"  var SENT = {}; var take = null;\n"
"  /* SpiderMonkey's JS_DefineProperty replaces a property, even a permanent one; JavaScriptCore\n"
"     throws for a non-configurable one. Oolite's Ship lists homeSystem twice: keep the first. */\n"
"  /* JS_ClearScope on the global (Oolite's JavaScript reset) must leave it as it began, as\n"
"     SpiderMonkey's new global does: what is defined on it later is kept configurable, so it\n"
"     can go, and the built-ins it began with stay. */\n"
"  var G = (0, eval)('this');\n"
"  var builtIn = {}; Object.getOwnPropertyNames(G).forEach(function(n) { builtIn[n] = true; });\n"
"  var realDefine = Object.defineProperty;\n"
"  Object.defineProperty = function defineProperty(o, n, d) {\n"
"    if (o === G && d && typeof d === 'object' && !builtIn[n] && d.configurable !== true) d = Object.assign({}, d, { configurable: true });\n"
"    return realDefine(o, n, d);\n"
"  };\n"
"  var define = function(o, n, d) {\n"
"    var had = Object.getOwnPropertyDescriptor(o, n);\n"
"    if (had && !had.configurable) { try { Object.defineProperty(o, n, d); } catch (e) {} return; }\n"
"    Object.defineProperty(o, n, d);\n"
"  };\n"
"  return {\n"
"    sent: SENT,\n"
"    setTake: function(t) { take = t; },\n"
"    makeNative: function(f, name, nargs) {\n"
"      var w = function() { var r = f(this, new.target !== undefined, ...arguments); if (r === SENT) throw take(); return r; };\n"
"      try { Object.defineProperty(w, 'name', { value: name }); Object.defineProperty(w, 'length', { value: nargs }); } catch (e) {}\n"
"      return w;\n"
"    },\n"
"    defineAccessor: function(o, n, g, s, e, c) {\n"
"      var d = { enumerable: e, configurable: c };\n"
"      d.get = g ? function() { var r = g(this); if (r === SENT) throw take(); return r; } : undefined;\n"
"      d.set = s ? function(v) { var r = s(this, v); if (r === SENT) throw take(); } : undefined;\n"
"      define(o, n, d);\n"
"    },\n"
"    defineValue: function(o, n, v, e, w, c) { define(o, n, { value: v, enumerable: e, writable: w, configurable: c }); },\n"
"    setProto: function(o, p) { Object.setPrototypeOf(o, p); return o; },\n"
"    isArray: function(o) { return Array.isArray(o); },\n"
"    freeze: function(o) { Object.freeze(o); },\n"
"    keys: function(o) { var k = []; for (var p in o) k.push(p); return k; },\n"
"    del: function(o, n) { return delete o[n]; },\n"
"    newError: function(m) { return new Error(m); },\n"
"    toStr: function(v) { return String(v); },\n"
"    toObj: function(v) { return Object(v); },\n"
"    newArray: function(n) { return new Array(n); },\n"
"    regexp: function(s, f) { return new RegExp(s, f); },\n"
"    objectProto: Object.prototype,\n"
"    clearScope: function(o) {\n"
"      Object.getOwnPropertyNames(o).forEach(function(n) { if (o !== G || !builtIn[n]) { try { delete o[n]; } catch (e) {} } });\n"
"    },\n"
"    execIn: function(__gfosjsapiSource) { with (this) { return eval(__gfosjsapiSource); } }\n"
"  };\n"
"})()";

static ooLValue glueFn(ooLValue glue, const char *name)
{
    ooLValue f = ooL_GetProp(glue, name);
    if (!f) fprintf(stderr, "gfosjsapi: glue %s missing\n", name);
    return f;                         /* kept for the program's life */
}

static void pendingFromFailure(ooLValue thrown);

static ooLValue callGlue(ooLValue fn, long argc, ooLValue *argv)
{
    ooLValue r = NULL;
    if (ooL_Call(fn, NULL, argc, argv, &r) != OOL_OK) {
        pendingFromFailure(r);        /* surface it to the caller */
        return NULL;
    }
    return keep(r);
}

static ooLValue takePending(long argc, ooLValue *argv, void *data);
static void boxReleased(void *box);

static int ensureReady(void)
{
    ooLValue glue = NULL, take;
    if (CX->ready) return 1;
    if (!ooL_Task()) { fprintf(stderr, "gfosjsapi: no JSTask (call ooL_SetTask first)\n"); return 0; }
    if (!ooL_Init(boxReleased)) { fprintf(stderr, "gfosjsapi: box class not registered\n"); return 0; }
    if (ooL_Evaluate(sGlue, -1, "gfosjsapi-glue.js", &glue) != OOL_OK || !glue) {
        char err[256] = "";
        ooL_ErrorText(err, sizeof err);
        fprintf(stderr, "gfosjsapi: glue failed: %s\n", err);
        return 0;
    }
    CX->gSent = glueFn(glue, "sent");
    CX->gMakeNative = glueFn(glue, "makeNative");
    CX->gDefineAccessor = glueFn(glue, "defineAccessor");
    CX->gDefineValue = glueFn(glue, "defineValue");
    CX->gSetProto = glueFn(glue, "setProto");
    CX->gIsArray = glueFn(glue, "isArray");
    CX->gFreeze = glueFn(glue, "freeze");
    CX->gKeys = glueFn(glue, "keys");
    CX->gDel = glueFn(glue, "del");
    CX->gNewError = glueFn(glue, "newError");
    CX->gToStr = glueFn(glue, "toStr");
    CX->gToObj = glueFn(glue, "toObj");
    CX->gNewArray = glueFn(glue, "newArray");
    CX->gRegExp = glueFn(glue, "regexp");
    CX->gObjectProto = glueFn(glue, "objectProto");
    CX->gExecIn = glueFn(glue, "execIn");
    CX->gClearScope = glueFn(glue, "clearScope");
    take = ooL_NewFunction("__gfosjsapiTake", takePending, NULL);
    {
        ooLValue setTake = glueFn(glue, "setTake"), r = NULL;
        ooL_Call(setTake, NULL, 1, &take, &r);
        ooL_Free(r);
        ooL_Free(setTake);
    }
    ooL_Free(take);
    ooL_Free(glue);
    CX->gEmpty = ooL_NewString("", 0);
    CX->global = (JSObject *)ooL_Global();
    CX->version = JSVERSION_ECMA_5;
    CX->ready = 1;
    return 1;
}

/* ==== exceptions =============================================================== */

/* SetEnv GFOSJSAPI_TRACE 1: every exception made pending is printed, to stdout (where Oolite's
   log goes, so Run >file catches it: this Shell has no *> for stderr), with its JS stack. */
static char *propText(ooLValue o, const char *name);

static int traceOn(void)
{
    static int on = -1;
    if (on < 0) {
        const char *e = getenv("GFOSJSAPI_TRACE");
        FILE *f = (e && *e) ? NULL : fopen("ENV:GFOSJSAPI_TRACE", "r");   /* getenv may not see ENV: from the script task */
        char buf[8] = "";
        if (f) { if (fgets(buf, sizeof buf, f)) e = buf; fclose(f); }
        on = e && *e && *e != '0';
    }
    return on;
}

static void setPendingOwned(ooLValue v)       /* takes ownership of v */
{
    if (traceOn()) {
        static unsigned long count;
        char text[300] = "";
        const char *stack = NULL;
        ooL_ToString(v, text, sizeof text);
        if (ooL_TypeOf(v) == OOL_OBJECT) stack = propText(v, "stack");
        FILE *f = fopen("T:gfosjsapi-trace.txt", "a");   /* the game runs on the script task: a file is sure */
        printf("gfosjsapi: pending exception %lu: %s\n", ++count, text);
        if (stack && *stack) printf("gfosjsapi: pending exception %lu stack: %.600s\n", count, stack);
        fflush(stdout);
        if (f) {
            fprintf(f, "gfosjsapi: pending exception %lu: %s\n", count, text);
            if (stack && *stack) fprintf(f, "gfosjsapi: pending exception %lu stack: %.600s\n", count, stack);
            fclose(f);
        }
    }
    if (CX->pendingPin) ooL_Free(CX->pendingPin);
    CX->pendingPin = v;
    CX->hasPending = 1;
    switch (ooL_TypeOf(v)) {
    case OOL_UNDEFINED: CX->pending = JSVAL_VOID; break;
    case OOL_NULL:      CX->pending = JSVAL_NULL; break;
    case OOL_STRING:    CX->pending = STRING_TO_JSVAL((JSString *)v); break;
    case OOL_OBJECT: case OOL_FUNCTION: CX->pending = OBJECT_TO_JSVAL((JSObject *)v); break;
    default: { double d; ooL_ToDouble(v, &d); CX->pending = numberJsval(d); break; }
    }
}

static void clearPending(void)
{
    if (CX->pendingPin) ooL_Free(CX->pendingPin);
    CX->pendingPin = NULL;
    CX->hasPending = 0;
    CX->pending = JSVAL_VOID;
}

/* Called from the glue when a native returned the sentinel: the exception. */
static ooLValue takePending(long argc, ooLValue *argv, void *data)
{
    ooLValue v;
    (void)argc; (void)argv; (void)data;
    if (!CX->hasPending) return ooL_Dup(keep(ooL_NewString("gfosjsapi: native failed without an exception", -1)));
    v = toLNew(CX->pending);
    clearPending();
    return v;
}

/* ==== strings =================================================================== */

static size_t utf16ToUtf8(const jschar *s, size_t n, char *out)
{
    size_t i, o = 0;
    for (i = 0; i < n; i++) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c < 0xDC00 && i + 1 < n && s[i + 1] >= 0xDC00 && s[i + 1] < 0xE000) {
            c = 0x10000 + ((c - 0xD800) << 10) + (s[i + 1] - 0xDC00);
            i++;
        }
        if (c < 0x80) { if (out) out[o] = (char)c; o += 1; }
        else if (c < 0x800) { if (out) { out[o] = (char)(0xC0 | (c >> 6)); out[o+1] = (char)(0x80 | (c & 0x3F)); } o += 2; }
        else if (c < 0x10000) { if (out) { out[o] = (char)(0xE0 | (c >> 12)); out[o+1] = (char)(0x80 | ((c >> 6) & 0x3F));
                                 out[o+2] = (char)(0x80 | (c & 0x3F)); } o += 3; }
        else { if (out) { out[o] = (char)(0xF0 | (c >> 18)); out[o+1] = (char)(0x80 | ((c >> 12) & 0x3F));
                          out[o+2] = (char)(0x80 | ((c >> 6) & 0x3F)); out[o+3] = (char)(0x80 | (c & 0x3F)); } o += 4; }
    }
    if (out) out[o] = 0;
    return o;
}

static char *utf16Dup8(const jschar *s, size_t n)
{
    size_t len = utf16ToUtf8(s, n, NULL);
    char *r = (char *)malloc(len + 1);
    if (r) utf16ToUtf8(s, n, r);
    return r;
}

static jschar *utf8Dup16(const char *s, size_t *lengthOut)
{
    size_t n = strlen(s), i = 0, o = 0;
    jschar *r = (jschar *)malloc((n + 1) * sizeof(jschar));
    if (!r) return NULL;
    while (i < n) {
        uint32_t c = (unsigned char)s[i];
        int extra = c < 0x80 ? 0 : c < 0xE0 ? 1 : c < 0xF0 ? 2 : 3;
        if (extra) c &= (0x3F >> extra);
        i++;
        while (extra-- > 0 && i < n) c = (c << 6) | ((unsigned char)s[i++] & 0x3F);
        if (c >= 0x10000) { c -= 0x10000; r[o++] = (jschar)(0xD800 + (c >> 10)); r[o++] = (jschar)(0xDC00 + (c & 0x3FF)); }
        else r[o++] = (jschar)c;
    }
    r[o] = 0;
    if (lengthOut) *lengthOut = o;
    return r;
}

/* The UTF-8 of a string handle: malloc'd, caller frees. */
static char *handleUtf8(ooLValue h)
{
    char small[128];
    long n = ooL_ToString(h, small, sizeof small);
    char *r;
    if (n < 0) return NULL;
    r = (char *)malloc((size_t)n + 1);
    if (!r) return NULL;
    if (n < (long)sizeof small) memcpy(r, small, (size_t)n + 1);
    else ooL_ToString(h, r, n + 1);
    return r;
}

char *OOJSStringToUTF8(JSContext *cx, JSString *str)
{
    (void)cx;
    return str ? handleUtf8((ooLValue)str) : NULL;
}

/* ---- the intern table: content -> one pinned handle (string ids) -------------- */

typedef struct OOIntern { struct OOIntern *nextByName, *nextByHandle; char *utf8; jschar *u16; size_t len16; ooLValue h; } OOIntern;
#define INTERN_BUCKETS 1024
static OOIntern *sByName[INTERN_BUCKETS], *sByHandle[INTERN_BUCKETS];

static unsigned hashName(const char *s) { unsigned h = 5381; while (*s) h = h * 33 + (unsigned char)*s++; return h & (INTERN_BUCKETS - 1); }
static unsigned hashHandle(ooLValue h) { return (unsigned)(((uintptr_t)h >> 3) & (INTERN_BUCKETS - 1)); }

static OOIntern *internLookupHandle(ooLValue h)
{
    OOIntern *i;
    for (i = sByHandle[hashHandle(h)]; i; i = i->nextByHandle) if (i->h == h) return i;
    return NULL;
}

static OOIntern *intern(const char *utf8)
{
    unsigned b = hashName(utf8), hb;
    OOIntern *i;
    for (i = sByName[b]; i; i = i->nextByName) if (strcmp(i->utf8, utf8) == 0) return i;
    i = (OOIntern *)calloc(1, sizeof *i);
    if (!i) return NULL;
    i->utf8 = strdup(utf8);
    i->u16 = utf8Dup16(utf8, &i->len16);
    i->h = ooL_NewString(utf8, -1);           /* pinned for the program's life */
    i->nextByName = sByName[b]; sByName[b] = i;
    hb = hashHandle(i->h);
    i->nextByHandle = sByHandle[hb]; sByHandle[hb] = i;
    return i;
}

/* ---- ids ------------------------------------------------------------------------ */

static int isIndex(const char *s, int32_t *out)
{
    long v = 0;
    const char *p = s;
    if (!*p || (*p == '0' && p[1])) return 0;
    while (*p) { if (*p < '0' || *p > '9' || v > 0x7FFFFFF) return 0; v = v * 10 + (*p++ - '0'); }
    *out = (int32_t)v;
    return 1;
}

static jsid idForName(const char *utf8)
{
    int32_t i;
    OOIntern *in;
    if (isIndex(utf8, &i)) return INT_TO_JSID(i);
    in = intern(utf8);
    return in ? (jsid)(uintptr_t)in->h : JSID_VOID;
}

/* The name of a string id, or the decimal of an int id, in buf. */
static const char *idName(jsid id, char *buf, size_t size)
{
    if (JSID_IS_INT(id)) { snprintf(buf, size, "%ld", (long)JSID_TO_INT(id)); return buf; }
    if (JSID_IS_STRING(id)) {
        OOIntern *in = internLookupHandle((ooLValue)JSID_TO_STRING(id));
        if (in) return in->utf8;
        {   /* not interned by us (shouldn't happen): read it */
            char *s = handleUtf8((ooLValue)JSID_TO_STRING(id));
            if (s) { snprintf(buf, size, "%s", s); free(s); return buf; }
        }
    }
    return NULL;
}

JSString *JS_NewStringCopyZ(JSContext *cx, const char *s)
{
    (void)cx;
    return (JSString *)keep(ooL_NewString(s ? s : "", -1));
}

JSString *JS_NewUCStringCopyN(JSContext *cx, const jschar *s, size_t n)
{
    char *u = utf16Dup8(s, n);
    JSString *r;
    (void)cx;
    if (!u) return NULL;
    r = (JSString *)keep(ooL_NewString(u, -1));
    free(u);
    return r;
}

JSString *JS_InternString(JSContext *cx, const char *s)
{
    OOIntern *in;
    (void)cx;
    in = intern(s ? s : "");
    return in ? (JSString *)in->h : NULL;
}

JSString *JS_InternUCStringN(JSContext *cx, const jschar *s, size_t length)
{
    char *u = utf16Dup8(s, length);
    JSString *r;
    if (!u) return NULL;
    r = JS_InternString(cx, u);
    free(u);
    return r;
}

JSBool JS_StringHasBeenInterned(JSString *str) { return internLookupHandle((ooLValue)str) != NULL; }

const jschar *JS_GetInternedStringChars(JSString *str)
{
    OOIntern *in = internLookupHandle((ooLValue)str);
    return in ? in->u16 : NULL;
}

const jschar *JS_GetStringCharsAndLength(JSContext *cx, JSString *str, size_t *length)
{
    OOIntern *in = internLookupHandle((ooLValue)str);
    char *u;
    jschar *w;
    (void)cx;
    if (in) { if (length) *length = in->len16; return in->u16; }
    u = handleUtf8((ooLValue)str);
    if (!u) return NULL;
    w = utf8Dup16(u, length);
    free(u);
    return (const jschar *)keepMalloc(w);      /* valid until the arena flushes */
}

size_t JS_GetStringLength(JSString *str)
{
    size_t n = 0;
    JS_GetStringCharsAndLength(CX, str, &n);
    return n;
}

JSBool JS_CompareStrings(JSContext *cx, JSString *a, JSString *b, int32 *result)
{
    char *x = OOJSStringToUTF8(cx, a), *y = OOJSStringToUTF8(cx, b);
    int c = strcmp(x ? x : "", y ? y : "");
    free(x); free(y);
    *result = c < 0 ? -1 : c > 0 ? 1 : 0;
    return JS_TRUE;
}

JSBool JS_StringEqualsAscii(JSContext *cx, JSString *str, const char *ascii, JSBool *match)
{
    char *x = OOJSStringToUTF8(cx, str);
    *match = x && strcmp(x, ascii) == 0;
    free(x);
    return JS_TRUE;
}

jsval JS_GetEmptyStringValue(JSContext *cx)
{
    (void)cx;
    return STRING_TO_JSVAL((JSString *)CX->gEmpty);
}

JSBool JS_ValueToId(JSContext *cx, jsval v, jsid *idp)
{
    char *s;
    if (JSVAL_IS_INT(v)) { *idp = INT_TO_JSID(JSVAL_TO_INT(v)); return JS_TRUE; }
    if (JSVAL_IS_STRING(v)) s = handleUtf8((ooLValue)JSVAL_TO_STRING(v));
    else {
        JSString *str = JS_ValueToString(cx, v);
        s = str ? handleUtf8((ooLValue)str) : NULL;
    }
    if (!s) return JS_FALSE;
    *idp = idForName(s);
    free(s);
    return JS_TRUE;
}

JSBool JS_IdToValue(JSContext *cx, jsid id, jsval *vp)
{
    (void)cx;
    if (JSID_IS_INT(id)) *vp = INT_TO_JSVAL(JSID_TO_INT(id));
    else if (JSID_IS_STRING(id)) *vp = STRING_TO_JSVAL(JSID_TO_STRING(id));
    else *vp = JSVAL_VOID;
    return JS_TRUE;
}

/* ==== conversions ================================================================ */

JSBool JS_NewNumberValue(JSContext *cx, jsdouble d, jsval *rval) { (void)cx; *rval = numberJsval(d); return JS_TRUE; }

JSBool JS_ValueToNumber(JSContext *cx, jsval v, jsdouble *dp)
{
    (void)cx;
    switch (OOJSV_TAG(v)) {
    case OOJSV_TAG_INT32:     *dp = JSVAL_TO_INT(v); return JS_TRUE;
    case OOJSV_TAG_BOOLEAN:   *dp = JSVAL_TO_BOOLEAN(v) ? 1 : 0; return JS_TRUE;
    case OOJSV_TAG_NULL:      *dp = 0; return JS_TRUE;
    case OOJSV_TAG_UNDEFINED: *dp = NAN; return JS_TRUE;
    case OOJSV_TAG_STRING:
    case OOJSV_TAG_OBJECT:    { double d = NAN; ooL_ToDouble((ooLValue)JSVAL_TO_GCTHING(v), &d); *dp = d; return JS_TRUE; }
    case OOJSV_TAG_MAGIC:
    case OOJSV_TAG_PRIVATE:   *dp = NAN; return JS_TRUE;
    default:                  *dp = JSVAL_TO_DOUBLE(v); return JS_TRUE;
    }
}

static int32 toInt32(double d)
{
    double two32 = 4294967296.0;
    if (d != d || isinf(d)) return 0;
    d = trunc(d);
    d = fmod(d, two32);
    if (d < 0) d += two32;
    return (int32)(uint32)d;
}

JSBool JS_ValueToInt32(JSContext *cx, jsval v, int32 *ip)
{
    double d;
    if (JSVAL_IS_INT(v)) { *ip = JSVAL_TO_INT(v); return JS_TRUE; }
    JS_ValueToNumber(cx, v, &d);
    *ip = toInt32(d < 0 ? ceil(d - 0.5) : floor(d + 0.5));   /* SpiderMonkey rounds here */
    return JS_TRUE;
}

JSBool JS_ValueToECMAUint32(JSContext *cx, jsval v, uint32 *ip)
{
    double d;
    JS_ValueToNumber(cx, v, &d);
    *ip = (uint32)toInt32(d);
    return JS_TRUE;
}

JSBool JS_ValueToBoolean(JSContext *cx, jsval v, JSBool *bp)
{
    (void)cx;
    switch (OOJSV_TAG(v)) {
    case OOJSV_TAG_INT32:   *bp = JSVAL_TO_INT(v) != 0; break;
    case OOJSV_TAG_BOOLEAN: *bp = JSVAL_TO_BOOLEAN(v); break;
    case OOJSV_TAG_NULL: case OOJSV_TAG_UNDEFINED: case OOJSV_TAG_MAGIC: case OOJSV_TAG_PRIVATE: *bp = JS_FALSE; break;
    case OOJSV_TAG_STRING:  *bp = ooL_ToBool((ooLValue)JSVAL_TO_STRING(v)); break;
    case OOJSV_TAG_OBJECT:  *bp = JS_TRUE; break;
    default: { double d = JSVAL_TO_DOUBLE(v); *bp = d == d && d != 0; break; }
    }
    return JS_TRUE;
}

JSType JS_TypeOfValue(JSContext *cx, jsval v)
{
    (void)cx;
    switch (OOJSV_TAG(v)) {
    case OOJSV_TAG_UNDEFINED: return JSTYPE_VOID;
    case OOJSV_TAG_NULL:      return JSTYPE_OBJECT;
    case OOJSV_TAG_BOOLEAN:   return JSTYPE_BOOLEAN;
    case OOJSV_TAG_STRING:    return JSTYPE_STRING;
    case OOJSV_TAG_OBJECT:    return ooL_TypeOf((ooLValue)JSVAL_TO_OBJECT(v)) == OOL_FUNCTION ? JSTYPE_FUNCTION : JSTYPE_OBJECT;
    default:                  return JSTYPE_NUMBER;
    }
}

JSString *JS_ValueToString(JSContext *cx, jsval v)
{
    ooLValue a, r;
    (void)cx;
    if (JSVAL_IS_STRING(v)) return JSVAL_TO_STRING(v);
    a = toL(v);
    r = callGlue(CX->gToStr, 1, &a);
    return (JSString *)r;
}

JSBool JS_ValueToObject(JSContext *cx, jsval v, JSObject **objp)
{
    ooLValue a, r;
    (void)cx;
    if (JSVAL_IS_OBJECT(v)) { *objp = JSVAL_TO_OBJECT(v); return JS_TRUE; }   /* null -> NULL */
    if (JSVAL_IS_VOID(v)) { *objp = NULL; return JS_TRUE; }
    a = toL(v);
    r = callGlue(CX->gToObj, 1, &a);
    *objp = (JSObject *)r;
    return r != NULL;
}

JSObject *JS_THIS_OBJECT(JSContext *cx, jsval *vp)
{
    jsval t = vp[1];
    JSObject *o = NULL;
    if (JSVAL_IS_MAGIC_IMPL(t)) return NULL;
    if (JSVAL_IS_NULL(t) || JSVAL_IS_VOID(t)) return CX->global;
    JS_ValueToObject(cx, t, &o);
    return o;
}

/* ==== natives ======================================================================= */

static ooLValue nativeEntry(long argc, ooLValue *argv, void *data)
{
    OONativeRec *n = (OONativeRec *)data;
    long nArgs = argc > 2 ? argc - 2 : 0, slots = nArgs > n->nargs ? nArgs : n->nargs, i;
    jsval small[2 + 12], *vp = slots <= 12 ? small : (jsval *)malloc(sizeof(jsval) * (size_t)(2 + slots));
    JSBool ok;
    ooLValue ret;
    if (!vp) return ooL_NewString("gfosjsapi: out of memory", -1);
    enter();
    CX->nativeDepth++;
    if (argc > 1 && ooL_ToBool(argv[1])) vp[1] = OOJSV_MAKE(OOJSV_TAG_MAGIC, JS_IS_CONSTRUCTING);
    else vp[1] = argc > 0 ? fromBorrowed(argv[0]) : JSVAL_VOID;
    for (i = 0; i < slots; i++) vp[2 + i] = i < nArgs ? fromBorrowed(argv[2 + i]) : JSVAL_VOID;
    /* SpiderMonkey: the rval slot starts as the callee; natives overwrite it */
    vp[0] = n->callee ? OBJECT_TO_JSVAL((JSObject *)n->callee) : JSVAL_VOID;
    ok = n->fn(CX, (uintN)nArgs, vp);
    if (ok) {
        jsval r = vp[0];
        /* a native that never set its result leaves the callee there: undefined */
        if (n->callee && OOJSV_TAG(r) == OOJSV_TAG_OBJECT && JSVAL_TO_OBJECT(r) == (JSObject *)n->callee) r = JSVAL_VOID;
        ret = toLNew(r);
    } else {
        ret = ooL_Dup(CX->gSent);
    }
    CX->nativeDepth--;
    leave();
    if (vp != small) free(vp);
    return ret;
}

static ooLValue makeNative(JSNative fn, const char *name, uintN nargs)
{
    OONativeRec *n = (OONativeRec *)calloc(1, sizeof *n);
    ooLValue f, args[3], w;
    if (!n) return NULL;
    n->fn = fn;
    n->nargs = (uint16)nargs;
    n->name = name;
    f = keep(ooL_NewFunction(name ? name : "native", nativeEntry, n));
    args[0] = f;
    args[1] = keep(ooL_NewString(name ? name : "", -1));
    args[2] = keep(ooL_NewLong((long)nargs));
    w = callGlue(CX->gMakeNative, 3, args);
    if (!w) return NULL;
    n->callee = ooL_Dup(w);            /* functions live as long as the program */
    n->next = CX->natives;
    CX->natives = n;
    return w;
}

/* ==== objects ======================================================================= */

static OOClassRec *classRec(JSClass *clasp)
{
    OOClassRec *r;
    for (r = CX->classes; r; r = r->next) if (r->clasp == clasp) return r;
    return NULL;
}

static OOBox *boxOf(JSObject *obj)
{
    if (obj == FINALIZING_OBJ) return CX->finalizing;
    return obj ? (OOBox *)ooL_UnwrapBox((ooLValue)obj) : NULL;
}

static void boxReleased(void *p)
{
    OOBox *box = (OOBox *)p;
    if (box->clasp && box->clasp->finalize && box->clasp->finalize != JS_FinalizeStub) {
        OOBox *was = CX->finalizing;
        CX->finalizing = box;
        box->clasp->finalize(CX, FINALIZING_OBJ);
        CX->finalizing = was;
    }
    free(box);
}

static void setProto(ooLValue obj, ooLValue proto)
{
    ooLValue args[2];
    args[0] = obj;
    args[1] = proto ? proto : keep(ooL_NewNull());
    callGlue(CX->gSetProto, 2, args);
}

JSObject *JS_NewObject(JSContext *cx, JSClass *clasp, JSObject *proto, JSObject *parent)
{
    ooLValue o;
    (void)cx; (void)parent;
    if (!clasp || clasp == &sObjectClass) {
        o = keep(ooL_NewObject());
        if (proto) setProto(o, (ooLValue)proto);
        return (JSObject *)o;
    }
    {
        OOBox *box = (OOBox *)calloc(1, sizeof *box);
        OOClassRec *rec;
        if (!box) return NULL;
        box->clasp = clasp;
        o = keep(ooL_WrapBox(box));
        if (!o) { free(box); return NULL; }
        if (!proto && (rec = classRec(clasp)) != NULL) proto = (JSObject *)rec->proto;
        setProto(o, proto ? (ooLValue)proto : CX->gObjectProto);
    }
    return (JSObject *)o;
}

JSClass *JS_GetClass(JSContext *cx, JSObject *obj)
{
    OOBox *box;
    (void)cx;
    if (!obj) return NULL;
    if (obj == CX->global && CX->globalClass) return CX->globalClass;
    if ((box = boxOf(obj)) != NULL) return box->clasp;
    if (ooL_TypeOf((ooLValue)obj) == OOL_FUNCTION) return &sFunctionClass;
    {
        ooLValue a = (ooLValue)obj, r = callGlue(CX->gIsArray, 1, &a);
        if (r && ooL_ToBool(r)) return &sArrayClass;
    }
    return &sObjectClass;
}

void *JS_GetPrivate(JSContext *cx, JSObject *obj) { OOBox *b = boxOf(obj); (void)cx; return b ? b->priv : NULL; }

JSBool JS_SetPrivate(JSContext *cx, JSObject *obj, void *data)
{
    OOBox *b = boxOf(obj);
    (void)cx;
    if (!b) return JS_FALSE;
    b->priv = data;
    return JS_TRUE;
}

JSBool JS_InstanceOf(JSContext *cx, JSObject *obj, JSClass *clasp, jsval *argv)
{
    (void)argv;
    return obj && JS_GetClass(cx, obj) == clasp;
}

void *JS_GetInstancePrivate(JSContext *cx, JSObject *obj, JSClass *clasp, jsval *argv)
{
    OOBox *b = boxOf(obj);
    (void)cx; (void)argv;
    return b && b->clasp == clasp ? b->priv : NULL;
}

JSBool JS_ObjectIsFunction(JSContext *cx, JSObject *obj) { (void)cx; return obj && ooL_TypeOf((ooLValue)obj) == OOL_FUNCTION; }

JSBool JS_IsArrayObject(JSContext *cx, JSObject *obj)
{
    ooLValue a = (ooLValue)obj, r;
    (void)cx;
    if (!obj) return JS_FALSE;
    r = callGlue(CX->gIsArray, 1, &a);
    return r && ooL_ToBool(r);
}
JSBool JS_ObjectIsArray(JSContext *cx, JSObject *obj) { return JS_IsArrayObject(cx, obj); }

JSBool JS_GetArrayLength(JSContext *cx, JSObject *obj, jsuint *lengthp)
{
    ooLValue l = keep(ooL_GetProp((ooLValue)obj, "length"));
    long n = 0;
    (void)cx;
    if (!l) return JS_FALSE;
    ooL_ToLong(l, &n);
    *lengthp = (jsuint)n;
    return JS_TRUE;
}

JSObject *JS_NewArrayObject(JSContext *cx, jsint length, jsval *vector)
{
    ooLValue n = keep(ooL_NewLong(vector ? 0 : length)), a = callGlue(CX->gNewArray, 1, &n);
    jsint i;
    (void)cx;
    if (!a) return NULL;
    if (vector) for (i = 0; i < length; i++) ooL_SetIndex(a, (unsigned long)i, toL(vector[i]));
    return (JSObject *)a;
}

JSBool JS_FreezeObject(JSContext *cx, JSObject *obj)
{
    ooLValue a = (ooLValue)obj;
    (void)cx;
    return callGlue(CX->gFreeze, 1, &a) != NULL || !CX->hasPending;
}

/* Oolite's JavaScript reset clears the global and builds it again; SpiderMonkey makes the global
   new, JavaScriptCore's stays: the glue deletes what was added to it (made configurable for this). */
void JS_ClearScope(JSContext *cx, JSObject *obj)
{
    ooLValue a, r;
    (void)cx;
    if (!obj || !ensureReady() || !CX->gClearScope) return;
    a = (ooLValue)obj;
    r = callGlue(CX->gClearScope, 1, &a);
    (void)r;
}

/* ---- properties ----------------------------------------------------------------------- */

static JSBool defineValue(JSObject *obj, const char *name, jsval value, uintN attrs)
{
    ooLValue a[6];
    a[0] = (ooLValue)obj;
    a[1] = keep(ooL_NewString(name, -1));
    a[2] = toL(value);
    a[3] = keep(ooL_NewBool((attrs & JSPROP_ENUMERATE) != 0));
    a[4] = keep(ooL_NewBool((attrs & JSPROP_READONLY) == 0));
    a[5] = keep(ooL_NewBool((attrs & JSPROP_PERMANENT) == 0));
    return callGlue(CX->gDefineValue, 6, a) != NULL || !CX->hasPending;
}

/* Accessor halves: argv[0] is `this`, argv[1] the value set. */
static ooLValue accessorGet(long argc, ooLValue *argv, void *data)
{
    OOAccessorRec *a = (OOAccessorRec *)data;
    JSObject *self;
    JSPropertyOp op;
    jsval v = JSVAL_VOID, t;
    JSBool ok = JS_TRUE;
    ooLValue ret;
    enter();
    CX->nativeDepth++;
    t = argc > 0 ? fromBorrowed(argv[0]) : JSVAL_VOID;
    self = JSVAL_IS_OBJECT(t) ? JSVAL_TO_OBJECT(t) : CX->global;
    op = a->getter;
    if (!op) { JSClass *c = JS_GetClass(CX, self); op = c ? c->getProperty : NULL; }
    if (op && op != JS_PropertyStub) ok = op(CX, self, a->id, &v);
    ret = ok ? toLNew(v) : ooL_Dup(CX->gSent);
    CX->nativeDepth--;
    leave();
    return ret;
}

static ooLValue accessorSet(long argc, ooLValue *argv, void *data)
{
    OOAccessorRec *a = (OOAccessorRec *)data;
    JSObject *self;
    JSStrictPropertyOp op;
    jsval v, t;
    JSBool ok = JS_TRUE;
    ooLValue ret;
    enter();
    CX->nativeDepth++;
    t = argc > 0 ? fromBorrowed(argv[0]) : JSVAL_VOID;
    v = argc > 1 ? fromBorrowed(argv[1]) : JSVAL_VOID;
    self = JSVAL_IS_OBJECT(t) ? JSVAL_TO_OBJECT(t) : CX->global;
    op = a->setter;
    if (!op) { JSClass *c = JS_GetClass(CX, self); op = c ? c->setProperty : NULL; }
    if (op && op != JS_StrictPropertyStub) ok = op(CX, self, a->id, JS_FALSE, &v);
    ret = ok ? NULL : ooL_Dup(CX->gSent);
    CX->nativeDepth--;
    leave();
    return ret;
}

static JSBool defineAccessor(JSObject *obj, const char *name, jsid id, JSPropertyOp getter,
                             JSStrictPropertyOp setter, uintN attrs, int classHooks)
{
    OOAccessorRec *a = (OOAccessorRec *)calloc(1, sizeof *a);
    ooLValue args[6];
    if (!a) return JS_FALSE;
    a->getter = getter && getter != JS_PropertyStub ? getter : NULL;
    a->setter = setter && setter != JS_StrictPropertyStub ? setter : NULL;
    a->id = id;
    a->flags = (uint8)attrs;
    if (!classHooks && !a->getter && !a->setter) { free(a); return defineValue(obj, name, JSVAL_VOID, attrs); }
    args[0] = (ooLValue)obj;
    args[1] = keep(ooL_NewString(name, -1));
    args[2] = keep(ooL_NewFunction(name, accessorGet, a));
    args[3] = (attrs & JSPROP_READONLY) ? NULL : keep(ooL_NewFunction(name, accessorSet, a));
    args[4] = keep(ooL_NewBool((attrs & JSPROP_ENUMERATE) != 0));
    args[5] = keep(ooL_NewBool((attrs & JSPROP_PERMANENT) == 0));
    return callGlue(CX->gDefineAccessor, 6, args) != NULL || !CX->hasPending;
}

JSBool JS_DefineProperty(JSContext *cx, JSObject *obj, const char *name, jsval value, JSPropertyOp getter,
                         JSStrictPropertyOp setter, uintN attrs)
{
    (void)cx;
    if ((getter && getter != JS_PropertyStub) || (setter && setter != JS_StrictPropertyStub))
        return defineAccessor(obj, name, idForName(name), getter, setter, attrs, 0);
    return defineValue(obj, name, value, attrs);
}

JSBool JS_DefinePropertyById(JSContext *cx, JSObject *obj, jsid id, jsval value, JSPropertyOp getter,
                             JSStrictPropertyOp setter, uintN attrs)
{
    char buf[24];
    const char *name = idName(id, buf, sizeof buf);
    if (!name) return JS_FALSE;
    return JS_DefineProperty(cx, obj, name, value, getter, setter, attrs);
}

JSBool JS_DefineProperties(JSContext *cx, JSObject *obj, JSPropertySpec *ps)
{
    (void)cx;
    for (; ps && ps->name; ps++) {
        /* tinyid properties reach the getter (or the class's) with their int id */
        jsid id = INT_TO_JSID(ps->tinyid);
        int hooks = (ps->flags & JSPROP_SHARED) != 0;
        if (!defineAccessor(obj, ps->name, id, ps->getter, ps->setter, ps->flags, hooks)) return JS_FALSE;
    }
    return JS_TRUE;
}

JSFunction *JS_DefineFunction(JSContext *cx, JSObject *obj, const char *name, JSNative call, uintN nargs, uintN attrs)
{
    ooLValue f = makeNative(call, name, nargs);
    (void)cx;
    if (!f || !defineValue(obj, name, OBJECT_TO_JSVAL((JSObject *)f), attrs)) return NULL;
    return (JSFunction *)f;
}

JSBool JS_DefineFunctions(JSContext *cx, JSObject *obj, JSFunctionSpec *fs)
{
    for (; fs && fs->name; fs++) if (!JS_DefineFunction(cx, obj, fs->name, fs->call, fs->nargs, 0)) return JS_FALSE;
    return JS_TRUE;
}

JSObject *JS_DefineObject(JSContext *cx, JSObject *obj, const char *name, JSClass *clasp, JSObject *proto, uintN attrs)
{
    JSObject *o = JS_NewObject(cx, clasp, proto, obj);
    if (!o || !defineValue(obj, name, OBJECT_TO_JSVAL(o), attrs)) return NULL;
    return o;
}

JSObject *JS_InitClass(JSContext *cx, JSObject *obj, JSObject *parent_proto, JSClass *clasp, JSNative constructor,
                       uintN nargs, JSPropertySpec *ps, JSFunctionSpec *fs, JSPropertySpec *static_ps,
                       JSFunctionSpec *static_fs)
{
    OOClassRec *rec;
    JSObject *proto = JS_NewObject(cx, clasp, parent_proto ? parent_proto : (JSObject *)CX->gObjectProto, obj);
    ooLValue ctor;
    if (!proto) return NULL;
    if (ps && !JS_DefineProperties(cx, proto, ps)) return NULL;
    if (fs && !JS_DefineFunctions(cx, proto, fs)) return NULL;
    rec = (OOClassRec *)calloc(1, sizeof *rec);
    if (!rec) return NULL;
    rec->clasp = clasp;
    rec->proto = ooL_Dup((ooLValue)proto);
    if (constructor) {
        ctor = makeNative(constructor, clasp->name, nargs);
        if (!ctor) return NULL;
        rec->ctor = ooL_Dup(ctor);
        defineValue((JSObject *)ctor, "prototype", OBJECT_TO_JSVAL(proto), JSPROP_READONLY | JSPROP_PERMANENT);
        defineValue(proto, "constructor", OBJECT_TO_JSVAL((JSObject *)ctor), 0);
        if (static_ps) JS_DefineProperties(cx, (JSObject *)ctor, static_ps);
        if (static_fs) JS_DefineFunctions(cx, (JSObject *)ctor, static_fs);
        defineValue(obj, clasp->name, OBJECT_TO_JSVAL((JSObject *)ctor), 0);
    }
    rec->next = CX->classes;
    CX->classes = rec;
    return proto;
}

JSBool JS_GetProperty(JSContext *cx, JSObject *obj, const char *name, jsval *vp)
{
    (void)cx;
    clearPending();
    *vp = fromOwned(ooL_GetProp((ooLValue)obj, name));
    return JS_TRUE;
}

JSBool JS_SetProperty(JSContext *cx, JSObject *obj, const char *name, jsval *vp)
{
    (void)cx;
    return ooL_SetProp((ooLValue)obj, name, toL(*vp)) == OOL_OK;
}

JSBool JS_DeleteProperty(JSContext *cx, JSObject *obj, const char *name)
{
    ooLValue a[2];
    (void)cx;
    a[0] = (ooLValue)obj;
    a[1] = keep(ooL_NewString(name, -1));
    return callGlue(CX->gDel, 2, a) != NULL || !CX->hasPending;
}

JSBool JS_GetPropertyById(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
{
    char buf[24];
    const char *name;
    if (JSID_IS_INT(id) && JSID_TO_INT(id) >= 0) { *vp = fromOwned(ooL_GetIndex((ooLValue)obj, (unsigned long)JSID_TO_INT(id))); return JS_TRUE; }
    name = idName(id, buf, sizeof buf);
    return name ? JS_GetProperty(cx, obj, name, vp) : JS_FALSE;
}

JSBool JS_SetPropertyById(JSContext *cx, JSObject *obj, jsid id, jsval *vp)
{
    char buf[24];
    const char *name;
    if (JSID_IS_INT(id) && JSID_TO_INT(id) >= 0)
        return ooL_SetIndex((ooLValue)obj, (unsigned long)JSID_TO_INT(id), toL(*vp)) == OOL_OK;
    name = idName(id, buf, sizeof buf);
    return name ? JS_SetProperty(cx, obj, name, vp) : JS_FALSE;
}

JSBool JS_LookupPropertyById(JSContext *cx, JSObject *obj, jsid id, jsval *vp) { return JS_GetPropertyById(cx, obj, id, vp); }

JSBool JS_GetMethodById(JSContext *cx, JSObject *obj, jsid id, JSObject **objp, jsval *vp)
{
    if (objp) *objp = obj;
    return JS_GetPropertyById(cx, obj, id, vp);
}

JSBool JS_GetElement(JSContext *cx, JSObject *obj, jsint index, jsval *vp)
{
    (void)cx;
    *vp = fromOwned(ooL_GetIndex((ooLValue)obj, (unsigned long)index));
    return JS_TRUE;
}
JSBool JS_LookupElement(JSContext *cx, JSObject *obj, jsint index, jsval *vp) { return JS_GetElement(cx, obj, index, vp); }

JSBool JS_SetElement(JSContext *cx, JSObject *obj, jsint index, jsval *vp)
{
    (void)cx;
    return ooL_SetIndex((ooLValue)obj, (unsigned long)index, toL(*vp)) == OOL_OK;
}

JSIdArray *JS_Enumerate(JSContext *cx, JSObject *obj)
{
    ooLValue a = (ooLValue)obj, keys = callGlue(CX->gKeys, 1, &a);
    long n = 0, i;
    JSIdArray *ida;
    if (!keys) return NULL;
    ooL_ToLong(keep(ooL_GetProp(keys, "length")), &n);
    ida = (JSIdArray *)malloc(sizeof *ida + sizeof(jsid) * (size_t)(n > 0 ? n : 1));
    if (!ida) return NULL;
    ida->length = (jsint)n;
    for (i = 0; i < n; i++) {
        jsval k = fromOwned(ooL_GetIndex(keys, (unsigned long)i));
        if (!JS_ValueToId(cx, k, &ida->vector[i])) ida->vector[i] = JSID_VOID;
    }
    return ida;
}

void JS_DestroyIdArray(JSContext *cx, JSIdArray *ida) { (void)cx; free(ida); }

/* ==== calls and scripts ============================================================== */

/* After a failed library call: the thrown value (or the library's error text) becomes pending. */
static void pendingFromFailure(ooLValue thrown)
{
    if (thrown) { setPendingOwned(thrown); return; }
    {
        char err[512] = "gfosjsapi: script error";
        ooL_ErrorText(err, sizeof err);
        setPendingOwned(ooL_NewString(err, -1));
    }
}

JSBool JS_CallFunctionValue(JSContext *cx, JSObject *obj, jsval fval, uintN argc, jsval *argv, jsval *rval)
{
    ooLValue small[8], *a = argc <= 8 ? small : (ooLValue *)malloc(sizeof(ooLValue) * argc), r = NULL;
    uintN i;
    long rc;
    (void)cx;
    if (!a) return JS_FALSE;
    for (i = 0; i < argc; i++) a[i] = toL(argv[i]);
    rc = ooL_Call(toL(fval), obj ? (ooLValue)obj : NULL, (long)argc, a, &r);
    if (a != small) free(a);
    if (rc != OOL_OK) { pendingFromFailure(r); if (rval) *rval = JSVAL_VOID; return JS_FALSE; }
    if (rval) *rval = fromOwned(r); else ooL_Free(r);
    return JS_TRUE;
}

JSBool JS_CallFunction(JSContext *cx, JSObject *obj, JSFunction *fun, uintN argc, jsval *argv, jsval *rval)
{
    return JS_CallFunctionValue(cx, obj, OBJECT_TO_JSVAL((JSObject *)fun), argc, argv, rval);
}

JSFunction *JS_ValueToFunction(JSContext *cx, jsval v)
{
    JSObject *o = JSVAL_IS_OBJECT(v) ? JSVAL_TO_OBJECT(v) : NULL;
    if (o && JS_ObjectIsFunction(cx, o)) return (JSFunction *)o;
    JS_ReportError(cx, "value is not a function");
    return NULL;
}

JSObject *JS_GetFunctionObject(JSFunction *fun) { return (JSObject *)fun; }

JSString *JS_GetFunctionId(JSFunction *fun)
{
    ooLValue n = ooL_GetProp((ooLValue)fun, "name");
    if (!n || ooL_TypeOf(n) != OOL_STRING || JS_GetStringLength((JSString *)keep(n)) == 0) {
        if (n && ooL_TypeOf(n) != OOL_STRING) ooL_Free(n);
        return NULL;
    }
    return (JSString *)n;
}

JSNative JS_GetFunctionNative(JSContext *cx, JSFunction *fun)
{
    OONativeRec *n;
    (void)cx;
    for (n = CX->natives; n; n = n->next) if (n->callee == (ooLValue)fun) return n->fn;
    return NULL;
}

/* A compiled script keeps its source. Run against the global it is evaluated
 * as it is (completion value, top-level declarations global); run against an
 * object it goes through a direct eval in a glue function called with
 * this = obj, which keeps `this` and the completion value. Top-level var and
 * function declarations then stay local to that call (SpiderMonkey would put
 * them on obj): a known gap, see the handover. */
struct OOJSScriptRec { char *utf8; ooLValue src; char *filename; };

static char *joinSource(const char *head, const jschar *chars, size_t length, const char *tail)
{
    char *body = utf16Dup8(chars, length), *src;
    size_t hl = strlen(head), bl, tl = strlen(tail);
    if (!body) return NULL;
    bl = strlen(body);
    src = (char *)malloc(hl + bl + tl + 1);
    if (src) { memcpy(src, head, hl); memcpy(src + hl, body, bl); memcpy(src + hl + bl, tail, tl + 1); }
    free(body);
    return src;
}

/* Compile checks syntax now (so errors come from compiling, as Oolite expects);
 * the head stays on line 1 so reported line numbers match the file. */
JSScript *JS_CompileUCScript(JSContext *cx, JSObject *obj, const jschar *chars, size_t length, const char *filename,
                             uintN lineno)
{
    const char *name = filename ? filename : "script";
    char *check = joinSource("(function(){", chars, length, "\n})"), *tail;
    ooLValue fn = NULL;
    JSScript *s;
    size_t tl;
    (void)cx; (void)obj; (void)lineno;
    if (!check) return NULL;
    if (ooL_Evaluate(check, -1, name, &fn) != OOL_OK || !fn) {
        free(check);
        pendingFromFailure(fn);
        return NULL;
    }
    free(check);
    ooL_Free(fn);
    s = (JSScript *)calloc(1, sizeof *s);
    if (!s) return NULL;
    tl = strlen(name) + 20;
    tail = (char *)malloc(tl);
    if (tail) snprintf(tail, tl, "\n//# sourceURL=%s", name);
    s->utf8 = joinSource("", chars, length, tail ? tail : "");
    free(tail);
    s->filename = strdup(name);
    s->src = s->utf8 ? ooL_NewString(s->utf8, -1) : NULL;
    if (!s->utf8 || !s->src || !s->filename) { JS_DestroyScript(cx, s); return NULL; }
    return s;
}

JSBool JS_ExecuteScript(JSContext *cx, JSObject *obj, JSScript *script, jsval *rval)
{
    ooLValue r = NULL;
    long rc;
    (void)cx;
    if (!script) return JS_FALSE;
    if (!obj || obj == CX->global) rc = ooL_Evaluate(script->utf8, -1, script->filename, &r);
    else rc = ooL_Call(CX->gExecIn, (ooLValue)obj, 1, &script->src, &r);
    if (rc != OOL_OK) {
        pendingFromFailure(r);
        if (rval) *rval = JSVAL_VOID;
        return JS_FALSE;
    }
    if (rval) *rval = fromOwned(r); else ooL_Free(r);
    return JS_TRUE;
}

JSObject *JS_NewScriptObject(JSContext *cx, JSScript *script)
{
    (void)cx;
    return script ? (JSObject *)keep(ooL_NewObject()) : NULL;   /* a token to root; the script lives until destroyed */
}

void JS_DestroyScript(JSContext *cx, JSScript *script)
{
    (void)cx;
    if (!script) return;
    ooL_Free(script->src);
    free(script->utf8);
    free(script->filename);
    free(script);
}

const char *JS_GetScriptFilename(JSContext *cx, JSScript *script) { (void)cx; return script ? script->filename : NULL; }

JSFunction *JS_CompileUCFunction(JSContext *cx, JSObject *obj, const char *name, uintN nargs, const char **argnames,
                                 const jschar *chars, size_t length, const char *filename, uintN lineno)
{
    char head[512];
    size_t o;
    uintN i;
    char *src;
    ooLValue fn = NULL;
    (void)cx; (void)obj; (void)lineno;
    o = (size_t)snprintf(head, sizeof head, "(function %s(", name ? name : "");
    for (i = 0; i < nargs && o < sizeof head - 64; i++)
        o += (size_t)snprintf(head + o, sizeof head - o, "%s%s", i ? "," : "", argnames[i]);
    snprintf(head + o, sizeof head - o, "){");
    src = joinSource(head, chars, length, "\n})");
    if (!src) return NULL;
    if (ooL_Evaluate(src, -1, filename ? filename : "function", &fn) != OOL_OK || !fn) {
        free(src);
        pendingFromFailure(fn);
        return NULL;
    }
    free(src);
    return (JSFunction *)keep(fn);
}

JSBool JS_BufferIsCompilableUnit(JSContext *cx, JSObject *obj, const char *bytes, size_t length)
{
    (void)cx; (void)obj; (void)bytes; (void)length;
    return JS_TRUE;   /* only the debug console asks; it then reports any syntax error */
}

JSObject *JS_NewUCRegExpObjectNoStatics(JSContext *cx, jschar *chars, size_t length, uintN flags)
{
    char f[5], *s = utf16Dup8(chars, length);
    int n = 0;
    ooLValue a[2], r;
    (void)cx;
    if (!s) return NULL;
    if (flags & JSREG_FOLD) f[n++] = 'i';
    if (flags & JSREG_GLOB) f[n++] = 'g';
    if (flags & JSREG_MULTILINE) f[n++] = 'm';
    if (flags & JSREG_STICKY) f[n++] = 'y';
    f[n] = 0;
    a[0] = keep(ooL_NewString(s, -1));
    a[1] = keep(ooL_NewString(f, -1));
    free(s);
    r = callGlue(CX->gRegExp, 2, a);
    return (JSObject *)r;
}

/* ==== errors ========================================================================== */

static void report(const char *msg, uintN flags, const char *file, uintN line)
{
    JSErrorReport rep;
    size_t n;
    jschar *uc;
    if (!CX->reporter) { fprintf(stderr, "%s:%u: %s\n", file ? file : "?", line, msg); return; }
    memset(&rep, 0, sizeof rep);
    rep.filename = file;
    rep.lineno = line;
    rep.flags = flags;
    uc = utf8Dup16(msg, &n);
    rep.ucmessage = uc;
    CX->reporter(CX, msg, &rep);
    free(uc);
}

void JS_ReportError(JSContext *cx, const char *format, ...)
{
    char msg[1024];
    va_list ap;
    (void)cx;
    va_start(ap, format);
    vsnprintf(msg, sizeof msg, format, ap);
    va_end(ap);
    if (CX->nativeDepth > 0) {
        /* inside a native: becomes the exception the script sees */
        ooLValue m = keep(ooL_NewString(msg, -1)), e = callGlue(CX->gNewError, 1, &m);
        if (e) setPendingOwned(ooL_Dup(e));
        return;
    }
    report(msg, JSREPORT_ERROR, NULL, 0);
}

JSBool JS_ReportWarning(JSContext *cx, const char *format, ...)
{
    char msg[1024];
    va_list ap;
    (void)cx;
    va_start(ap, format);
    vsnprintf(msg, sizeof msg, format, ap);
    va_end(ap);
    report(msg, JSREPORT_WARNING, NULL, 0);
    return JS_TRUE;
}

static char *propText(ooLValue o, const char *name)
{
    ooLValue v = keep(ooL_GetProp(o, name));
    char *s;
    long n;
    if (!v || ooL_TypeOf(v) == OOL_UNDEFINED) return NULL;
    n = ooL_ToString(v, NULL, 0);
    if (n < 0) return NULL;
    s = (char *)keepMalloc(malloc((size_t)n + 1));
    if (s) ooL_ToString(v, s, n + 1);
    return s;
}

JSBool JS_ReportPendingException(JSContext *cx)
{
    ooLValue e;
    char *msg = NULL, *file = NULL, *line;
    uintN lineno = 0;
    (void)cx;
    if (!CX->hasPending) return JS_FALSE;
    e = toL(CX->pending);
    if (e && (ooL_TypeOf(e) == OOL_OBJECT)) {
        char *m = propText(e, "message"), *nm = propText(e, "name");
        if (m) {
            size_t l = strlen(m) + (nm ? strlen(nm) : 5) + 3;
            msg = (char *)keepMalloc(malloc(l));
            if (msg) snprintf(msg, l, "%s: %s", nm ? nm : "Error", m);
        }
        file = propText(e, "sourceURL");
        if (!file) file = propText(e, "fileName");
        line = propText(e, "line");
        if (!line) line = propText(e, "lineNumber");
        if (line) lineno = (uintN)strtoul(line, NULL, 10);
    }
    if (!msg) {
        JSString *s = JS_ValueToString(CX, CX->pending);
        msg = s ? (char *)keepMalloc(OOJSStringToUTF8(CX, s)) : NULL;
    }
    clearPending();
    report(msg ? msg : "uncaught exception", JSREPORT_ERROR | JSREPORT_EXCEPTION, file, lineno);
    return JS_TRUE;
}

JSBool JS_IsExceptionPending(JSContext *cx) { (void)cx; return CX->hasPending; }

JSBool JS_GetPendingException(JSContext *cx, jsval *vp)
{
    (void)cx;
    if (!CX->hasPending) return JS_FALSE;
    *vp = CX->pending;
    if (JSVAL_IS_GCTHING(*vp)) keep(ooL_Dup(handleOfJsval(*vp)));   /* outlives a clear in this scope */
    return JS_TRUE;
}

void JS_SetPendingException(JSContext *cx, jsval v) { (void)cx; setPendingOwned(toLNew(v)); }
void JS_ClearPendingException(JSContext *cx) { (void)cx; clearPending(); }

struct JSExceptionState { int has; ooLValue v; };

JSExceptionState *JS_SaveExceptionState(JSContext *cx)
{
    JSExceptionState *s = (JSExceptionState *)calloc(1, sizeof *s);
    (void)cx;
    if (!s) return NULL;
    s->has = CX->hasPending;
    if (s->has) s->v = toLNew(CX->pending);
    return s;
}

void JS_RestoreExceptionState(JSContext *cx, JSExceptionState *s)
{
    (void)cx;
    if (!s) return;
    if (s->has) setPendingOwned(s->v); else clearPending();
    free(s);
}

/* ==== runtime, context, requests ========================================================= */

static uint32 sOptions;

JSRuntime *JS_NewRuntime(uint32 maxbytes) { (void)maxbytes; return ensureReady() ? (JSRuntime *)CX : NULL; }
void JS_DestroyRuntime(JSRuntime *rt) { (void)rt; }

JSContext *JS_NewContext(JSRuntime *rt, size_t stackChunkSize)
{
    (void)rt; (void)stackChunkSize;
    if (!ensureReady()) return NULL;
    if (CX->cxCallback && !CX->cxCallback(CX, JSCONTEXT_NEW)) return NULL;
    return CX;
}

void JS_DestroyContext(JSContext *cx) { if (CX->cxCallback) CX->cxCallback(cx, JSCONTEXT_DESTROY); }
JSRuntime *JS_GetRuntime(JSContext *cx) { return (JSRuntime *)cx; }

JSContextCallback JS_SetContextCallback(JSRuntime *rt, JSContextCallback cb)
{
    JSContextCallback old = CX->cxCallback;
    (void)rt;
    CX->cxCallback = cb;
    return old;
}

void JS_BeginRequest(JSContext *cx) { (void)cx; enter(); }
void JS_EndRequest(JSContext *cx) { (void)cx; leave(); }
JSBool JS_IsInRequest(JSContext *cx) { (void)cx; return CX->nest > 0; }

jsrefcount JS_SuspendRequest(JSContext *cx)
{
    jsrefcount d = CX->nest;
    (void)cx;
    CX->suspended += d;
    CX->nest = 0;
    flush();
    return d;
}

void JS_ResumeRequest(JSContext *cx, jsrefcount saveDepth) { (void)cx; CX->nest += saveDepth; CX->suspended -= saveDepth; }

JSBool JS_EnterLocalRootScope(JSContext *cx) { (void)cx; enter(); return JS_TRUE; }
void JS_LeaveLocalRootScope(JSContext *cx) { (void)cx; leave(); }
void JS_LeaveLocalRootScopeWithResult(JSContext *cx, jsval rval) { (void)cx; (void)rval; leave(); }

uint32 JS_GetOptions(JSContext *cx) { (void)cx; return sOptions; }
uint32 JS_SetOptions(JSContext *cx, uint32 options) { uint32 o = sOptions; (void)cx; sOptions = options; return o; }
JSVersion JS_GetVersion(JSContext *cx) { (void)cx; return CX->version; }
JSVersion JS_SetVersion(JSContext *cx, JSVersion v) { JSVersion o = CX->version; (void)cx; CX->version = v; return o; }
const char *JS_VersionToString(JSVersion version) { (void)version; return "ECMAv5"; }
void JS_SetCStringsAreUTF8(void) { }

JSErrorReporter JS_SetErrorReporter(JSContext *cx, JSErrorReporter er)
{
    JSErrorReporter old = CX->reporter;
    (void)cx;
    CX->reporter = er;
    return old;
}

JSOperationCallback JS_SetOperationCallback(JSContext *cx, JSOperationCallback cb)
{
    JSOperationCallback old = CX->opCallback;
    (void)cx;
    CX->opCallback = cb;
    return old;
}

void JS_TriggerAllOperationCallbacks(JSRuntime *rt) { (void)rt; }
void JS_SetFunctionCallback(JSContext *cx, JSFunctionCallback fcb) { (void)cx; (void)fcb; }
void JS_SetGCZeal(JSContext *cx, uint8 zeal) { (void)cx; (void)zeal; }
JSBool JS_SetDebuggerHandler(JSRuntime *rt, JSDebuggerHandler hook, void *closure) { (void)rt; (void)hook; (void)closure; return JS_TRUE; }

JSObject *JS_NewCompartmentAndGlobalObject(JSContext *cx, JSClass *clasp, JSPrincipals *principals)
{
    (void)cx; (void)principals;
    if (!ensureReady()) return NULL;
    CX->globalClass = clasp;
    return CX->global;
}

void JS_SetGlobalObject(JSContext *cx, JSObject *obj) { (void)cx; if (obj) CX->global = obj; }
JSBool JS_InitStandardClasses(JSContext *cx, JSObject *obj) { (void)cx; (void)obj; return JS_TRUE; }
void JS_GC(JSContext *cx) { (void)cx; }
void JS_MaybeGC(JSContext *cx) { (void)cx; }
uint32 JS_GetGCParameter(JSRuntime *rt, JSGCParamKey key) { (void)rt; (void)key; return 0; }

int64 JS_Now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64)tv.tv_sec * 1000000 + tv.tv_usec;
}

/* ==== roots ============================================================================ */

static JSBool addRoot(void *loc, int kind, const char *name)
{
    OORoot *r;
    ooLValue h;
    CX->roots = xgrow(CX->roots, &CX->rootCap, CX->rootCount + 1, sizeof *CX->roots);
    r = &CX->roots[CX->rootCount++];
    r->loc = loc;
    r->kind = kind;
    r->name = name;
    h = rootHandle(r);
    r->pinned = h ? ooL_Dup(h) : NULL;
    return JS_TRUE;
}

static JSBool removeRoot(void *loc)
{
    int i;
    for (i = CX->rootCount - 1; i >= 0; i--) {
        if (CX->roots[i].loc == loc) {
            /* the value may still be in use in this scope: let the arena drop it */
            if (CX->roots[i].pinned) keep(CX->roots[i].pinned);
            CX->roots[i] = CX->roots[--CX->rootCount];
            return JS_TRUE;
        }
    }
    return JS_FALSE;
}

JSBool JS_AddValueRoot(JSContext *cx, jsval *vp) { (void)cx; return addRoot(vp, ROOT_VALUE, NULL); }
JSBool JS_AddStringRoot(JSContext *cx, JSString **rp) { (void)cx; return addRoot(rp, ROOT_THING, NULL); }
JSBool JS_AddObjectRoot(JSContext *cx, JSObject **rp) { (void)cx; return addRoot(rp, ROOT_THING, NULL); }
JSBool JS_AddGCThingRoot(JSContext *cx, void **rp) { (void)cx; return addRoot(rp, ROOT_THING, NULL); }
JSBool JS_AddNamedValueRoot(JSContext *cx, jsval *vp, const char *name) { (void)cx; return addRoot(vp, ROOT_VALUE, name); }
JSBool JS_AddNamedStringRoot(JSContext *cx, JSString **rp, const char *name) { (void)cx; return addRoot(rp, ROOT_THING, name); }
JSBool JS_AddNamedObjectRoot(JSContext *cx, JSObject **rp, const char *name) { (void)cx; return addRoot(rp, ROOT_THING, name); }
JSBool JS_AddNamedGCThingRoot(JSContext *cx, void **rp, const char *name) { (void)cx; return addRoot(rp, ROOT_THING, name); }
JSBool JS_RemoveValueRoot(JSContext *cx, jsval *vp) { (void)cx; return removeRoot(vp); }
JSBool JS_RemoveStringRoot(JSContext *cx, JSString **rp) { (void)cx; return removeRoot(rp); }
JSBool JS_RemoveObjectRoot(JSContext *cx, JSObject **rp) { (void)cx; return removeRoot(rp); }
JSBool JS_RemoveGCThingRoot(JSContext *cx, void **rp) { (void)cx; return removeRoot(rp); }

void JS_DumpNamedRoots(JSRuntime *rt, void (*dump)(const char *name, void *rp, JSGCRootType type, void *data), void *data)
{
    int i;
    (void)rt;
    for (i = 0; i < CX->rootCount; i++)
        if (CX->roots[i].name)
            dump(CX->roots[i].name, CX->roots[i].loc,
                 CX->roots[i].kind == ROOT_VALUE ? JS_GC_ROOT_VALUE_PTR : JS_GC_ROOT_GCTHING_PTR, data);
}

JSBool JS_DumpHeap(JSContext *cx, FILE *fp, void *startThing, uint32 startKind, void *thingToFind, size_t maxDepth,
                   void *thingToIgnore)
{
    (void)cx; (void)fp; (void)startThing; (void)startKind; (void)thingToFind; (void)maxDepth; (void)thingToIgnore;
    return JS_FALSE;
}

/* ==== class stubs ========================================================================= */

JSBool JS_PropertyStub(JSContext *cx, JSObject *obj, jsid id, jsval *vp) { (void)cx; (void)obj; (void)id; (void)vp; return JS_TRUE; }
JSBool JS_StrictPropertyStub(JSContext *cx, JSObject *obj, jsid id, JSBool strict, jsval *vp) { (void)cx; (void)obj; (void)id; (void)strict; (void)vp; return JS_TRUE; }
JSBool JS_EnumerateStub(JSContext *cx, JSObject *obj) { (void)cx; (void)obj; return JS_TRUE; }
JSBool JS_ResolveStub(JSContext *cx, JSObject *obj, jsid id) { (void)cx; (void)obj; (void)id; return JS_TRUE; }
JSBool JS_ConvertStub(JSContext *cx, JSObject *obj, JSType type, jsval *vp) { (void)cx; (void)obj; (void)type; (void)vp; return JS_TRUE; }
void JS_FinalizeStub(JSContext *cx, JSObject *obj) { (void)cx; (void)obj; }

/* ==== XDR: the library caches compiled scripts itself ======================================= */

JSXDRState *JS_XDRNewMem(JSContext *cx, JSXDRMode mode) { (void)cx; (void)mode; return NULL; }
void *JS_XDRMemGetData(JSXDRState *xdr, uint32 *lp) { (void)xdr; if (lp) *lp = 0; return NULL; }
void JS_XDRMemSetData(JSXDRState *xdr, void *data, uint32 len) { (void)xdr; (void)data; (void)len; }
void JS_XDRDestroy(JSXDRState *xdr) { (void)xdr; }
JSBool JS_XDRScript(JSXDRState *xdr, JSScript **scriptp) { (void)xdr; (void)scriptp; return JS_FALSE; }
