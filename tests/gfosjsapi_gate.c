/* gfosjsapi gate, body: SpiderMonkey 1.8.5 API as Oolite uses it, on
 * javascript.library. Runs on the script task. Lines starting INFO report
 * behaviour to know about without failing the gate. Team 5, 19 Nov 2026. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsapi.h"

static int failures;
static char lastReport[512];
static unsigned lastLine;
static int finalized;

static void check(const char *what, int ok)
{
    printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) failures++;
}

static void reporter(JSContext *cx, const char *message, JSErrorReport *report)
{
    (void)cx;
    snprintf(lastReport, sizeof lastReport, "%s", message ? message : "");
    lastLine = report ? report->lineno : 0;
}

static JSClass globalClass = { "global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS };

/* ---- a native ---- */
static JSBool nativeAdd(JSContext *cx, uintN argc, jsval *vp)
{
    jsdouble a = 0, b = 0;
    if (argc < 2) { JS_ReportError(cx, "add needs two numbers"); return JS_FALSE; }
    JS_ValueToNumber(cx, JS_ARGV(cx, vp)[0], &a);
    JS_ValueToNumber(cx, JS_ARGV(cx, vp)[1], &b);
    return JS_NewNumberValue(cx, a + b, vp);
}

static JSBool nativeEmpty(JSContext *cx, uintN argc, jsval *vp) { (void)cx; (void)argc; JS_SET_RVAL(cx, vp, JSVAL_VOID); return JS_TRUE; }

/* ---- a class like Oolite's Vector3D ---- */
typedef struct { double x, y; } Vec;
enum { kVec_x, kVec_y };

static void vecFinalize(JSContext *cx, JSObject *obj) { free(JS_GetPrivate(cx, obj)); finalized++; }

static JSClass vecClass = { "Vec", JSCLASS_HAS_PRIVATE, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, vecFinalize, JSCLASS_NO_OPTIONAL_MEMBERS };

static JSBool vecGet(JSContext *cx, JSObject *this, jsid id, jsval *vp)
{
    Vec *v = JS_GetInstancePrivate(cx, this, &vecClass, NULL);
    if (!v || !JSID_IS_INT(id)) return JS_TRUE;
    return JS_NewNumberValue(cx, JSID_TO_INT(id) == kVec_x ? v->x : v->y, vp);
}

static JSBool vecSet(JSContext *cx, JSObject *this, jsid id, JSBool strict, jsval *vp)
{
    Vec *v = JS_GetInstancePrivate(cx, this, &vecClass, NULL);
    jsdouble d = 0;
    (void)strict;
    if (!v || !JSID_IS_INT(id)) return JS_TRUE;
    JS_ValueToNumber(cx, *vp, &d);
    if (JSID_TO_INT(id) == kVec_x) v->x = d; else v->y = d;
    return JS_TRUE;
}

static JSPropertySpec vecProps[] = {
    { "x", kVec_x, JSPROP_PERMANENT | JSPROP_ENUMERATE, vecGet, vecSet },
    { "y", kVec_y, JSPROP_PERMANENT | JSPROP_ENUMERATE, vecGet, vecSet },
    { 0, 0, 0, 0, 0 }
};

static JSBool vecLength(JSContext *cx, uintN argc, jsval *vp)
{
    JSObject *this = JS_THIS_OBJECT(cx, vp);
    Vec *v = JS_GetInstancePrivate(cx, this, &vecClass, NULL);
    (void)argc;
    if (!v) { JS_ReportError(cx, "length called on a non-Vec"); return JS_FALSE; }
    return JS_NewNumberValue(cx, v->x * v->x + v->y * v->y, vp);   /* squared, to stay exact */
}

static JSFunctionSpec vecMethods[] = { { "lengthSq", vecLength, 0, 0 }, { 0, 0, 0, 0 } };

static JSBool vecConstruct(JSContext *cx, uintN argc, jsval *vp)
{
    Vec *v;
    JSObject *o;
    jsdouble a = 0, b = 0;
    if (!JS_IsConstructing(cx, vp)) { JS_ReportError(cx, "Vec must be called with new"); return JS_FALSE; }
    if (argc > 0) JS_ValueToNumber(cx, JS_ARGV(cx, vp)[0], &a);
    if (argc > 1) JS_ValueToNumber(cx, JS_ARGV(cx, vp)[1], &b);
    v = malloc(sizeof *v);
    v->x = a; v->y = b;
    o = JS_NewObject(cx, &vecClass, NULL, NULL);
    JS_SetPrivate(cx, o, v);
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(o));
    return JS_TRUE;
}

/* ---- helpers ---- */
static JSBool run(JSContext *cx, JSObject *obj, const char *src, jsval *rval)
{
    size_t n = strlen(src), i;
    jschar *u = malloc((n + 1) * sizeof *u);
    JSScript *s;
    JSBool ok;
    for (i = 0; i < n; i++) u[i] = (unsigned char)src[i];
    s = JS_CompileUCScript(cx, obj, u, n, "gate.js", 1);
    free(u);
    if (!s) return JS_FALSE;
    ok = JS_ExecuteScript(cx, obj, s, rval);
    JS_DestroyScript(cx, s);
    return ok;
}

static int isNum(JSContext *cx, jsval v, double want) { jsdouble d = -1; JS_ValueToNumber(cx, v, &d); return d == want; }

int gfosjsapiGateBody(void)
{
    JSRuntime *rt;
    JSContext *cx;
    JSObject *global, *proto, *o, *rooted = NULL;
    jsval v, args[2];

    int64 t0, t1;
    int i;

    rt = JS_NewRuntime(8L << 20);
    check("JS_NewRuntime", rt != NULL);
    if (!rt) return failures;
    cx = JS_NewContext(rt, 8192);
    check("JS_NewContext", cx != NULL);
    JS_SetErrorReporter(cx, reporter);
    JS_BeginRequest(cx);
    global = JS_NewCompartmentAndGlobalObject(cx, &globalClass, NULL);
    check("global object", global != NULL && JS_GetClass(cx, global) == &globalClass);
    check("JS_InitStandardClasses", JS_InitStandardClasses(cx, global));

    /* values both ways */
    v = INT_TO_JSVAL(7);       JS_SetProperty(cx, global, "seven", &v);
    v = DOUBLE_TO_JSVAL(2.5);  JS_SetProperty(cx, global, "twoAndHalf", &v);
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "caf\xc3\xa9"));  JS_SetProperty(cx, global, "word", &v);
    check("script sees int, double, UTF-8 string", run(cx, global, "seven * twoAndHalf + word.length", &v) && isNum(cx, v, 21.5));
    check("int comes back as int", run(cx, global, "40 + 2", &v) && JSVAL_IS_INT(v) && JSVAL_TO_INT(v) == 42);
    check("double comes back as double", run(cx, global, "0.5", &v) && JSVAL_IS_DOUBLE(v) && JSVAL_TO_DOUBLE(v) == 0.5);
    check("bool, null, undefined", run(cx, global, "true", &v) && v == JSVAL_TRUE
          && run(cx, global, "null", &v) && JSVAL_IS_NULL(v) && run(cx, global, "undefined", &v) && JSVAL_IS_VOID(v));
    if (run(cx, global, "'\\u00fcber'", &v) && JSVAL_IS_STRING(v)) {
        size_t n = 0;
        const jschar *c = JS_GetStringCharsAndLength(cx, JSVAL_TO_STRING(v), &n);
        check("string chars are UTF-16", n == 4 && c && c[0] == 0xFC && c[1] == 'b');
    } else check("string chars are UTF-16", 0);

    /* strings and ids */
    {
        JSBool m = JS_FALSE;
        jsid a = INTERNED_STRING_TO_JSID(JS_InternString(cx, "position")), b = INTERNED_STRING_TO_JSID(JS_InternString(cx, "position"));
        check("interned ids are equal", a == b && JSID_IS_STRING(a));
        JS_StringEqualsAscii(cx, JS_NewStringCopyZ(cx, "abc"), "abc", &m);
        check("JS_StringEqualsAscii", m);
    }

    /* natives */
    JS_DefineFunction(cx, global, "add", nativeAdd, 2, 0);
    JS_DefineFunction(cx, global, "empty", nativeEmpty, 0, 0);
    check("native called from script", run(cx, global, "add(2, 3)", &v) && isNum(cx, v, 5));
    check("native's length is nargs", run(cx, global, "add.length", &v) && isNum(cx, v, 2));
    check("native error caught by script",
          run(cx, global, "var m; try { add(1); m = 'none'; } catch (e) { m = e.message; } m", &v)
          && JSVAL_IS_STRING(v));
    {
        JSBool m = JS_FALSE;
        if (JSVAL_IS_STRING(v)) JS_StringEqualsAscii(cx, JSVAL_TO_STRING(v), "add needs two numbers", &m);
        check("... with the native's message", m);
    }

    /* C -> JS call */
    run(cx, global, "function twice(n) { return n * 2; }", NULL);
    JS_GetProperty(cx, global, "twice", &v);
    check("function found on the global", JSVAL_IS_OBJECT(v) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(v)));
    args[0] = INT_TO_JSVAL(21);
    {
        jsval r = JSVAL_VOID;
        check("JS_CallFunctionValue", JS_CallFunctionValue(cx, global, v, 1, args, &r) && isNum(cx, r, 42));
    }

    /* uncaught exception goes to the reporter with a line number */
    lastReport[0] = 0;
    check("uncaught throw fails the script", !run(cx, global, "\n\nthrow new Error('boom');", NULL));
    check("exception pending", JS_IsExceptionPending(cx));
    JS_ReportPendingException(cx);
    check("reporter got the message", strstr(lastReport, "boom") != NULL);
    printf("INFO reporter: \"%s\" line %u (want 3)\n", lastReport, lastLine);
    check("no exception pending after report", !JS_IsExceptionPending(cx));

    /* a class */
    proto = JS_InitClass(cx, global, NULL, &vecClass, vecConstruct, 2, vecProps, vecMethods, NULL, NULL);
    check("JS_InitClass", proto != NULL);
    check("new Vec, tinyid getters", run(cx, global, "var a = new Vec(3, 4); a.x * 10 + a.y", &v) && isNum(cx, v, 34));
    check("method on the prototype", run(cx, global, "a.lengthSq()", &v) && isNum(cx, v, 25));
    check("setter through the class", run(cx, global, "a.x = 6; a.lengthSq()", &v) && isNum(cx, v, 52));
    check("instanceof", run(cx, global, "a instanceof Vec", &v) && v == JSVAL_TRUE);
    check("calling without new throws", !run(cx, global, "Vec(1, 2)", NULL) && JS_IsExceptionPending(cx));
    JS_ClearPendingException(cx);
    check("method on a non-Vec throws", !run(cx, global, "Vec.prototype.lengthSq.call({})", NULL));
    JS_ClearPendingException(cx);
    run(cx, global, "a", &v);
    o = JSVAL_TO_OBJECT(v);
    {
        Vec *p = JS_GetInstancePrivate(cx, o, &vecClass, NULL);
        check("private from C", p && p->x == 6 && p->y == 4 && JS_GetClass(cx, o) == &vecClass);
    }
    o = JS_NewObject(cx, &vecClass, NULL, NULL);
    { Vec *p = malloc(sizeof *p); p->x = 1; p->y = 1; JS_SetPrivate(cx, o, p); }
    v = OBJECT_TO_JSVAL(o);
    JS_SetProperty(cx, global, "fromC", &v);
    check("C-made object has the class prototype", run(cx, global, "fromC.lengthSq() + (fromC instanceof Vec ? 10 : 0)", &v) && isNum(cx, v, 12));

    /* enumerate */
    run(cx, global, "var bag = { p: 1, q: 2, r: 3 }", NULL);
    JS_GetProperty(cx, global, "bag", &v);
    {
        JSIdArray *ida = JS_Enumerate(cx, JSVAL_TO_OBJECT(v));
        jsval k = JSVAL_VOID;
        check("JS_Enumerate", ida && ida->length == 3);
        if (ida && ida->length > 0) {
            JS_IdToValue(cx, ida->vector[0], &k);
            check("ids back to strings", JSVAL_IS_STRING(k));
        }
        JS_DestroyIdArray(cx, ida);
    }

    /* roots survive the end of a request */
    run(cx, global, "({ kept: 'yes' })", &v);
    rooted = JSVAL_TO_OBJECT(v);
    JS_AddObjectRoot(cx, &rooted);
    JS_EndRequest(cx);
    JS_BeginRequest(cx);
    v = JSVAL_VOID;
    JS_GetProperty(cx, rooted, "kept", &v);
    {
        JSBool m = JS_FALSE;
        if (JSVAL_IS_STRING(v)) JS_StringEqualsAscii(cx, JSVAL_TO_STRING(v), "yes", &m);
        check("rooted object usable after the request ends", m);
    }
    JS_RemoveObjectRoot(cx, &rooted);

    /* script with this = an object (how Oolite runs world scripts) */
    o = JS_NewObject(cx, NULL, NULL, NULL);
    check("script with this = object", run(cx, o, "this.answer = 42; this.answer", &v) && isNum(cx, v, 42));
    JS_GetProperty(cx, o, "answer", &v);
    check("... property landed on the object", isNum(cx, v, 42));
    run(cx, o, "var topLevel = 1; function topFn() {}", NULL);
    JS_GetProperty(cx, global, "topLevel", &v);
    printf("INFO top-level var in a this-script reaches the global: %s (SpiderMonkey: yes)\n", JSVAL_IS_VOID(v) ? "no" : "yes");

    /* handle identity: does the library hand back the same pointer for the same object? */
    {
        jsval a1, a2;
        JS_GetProperty(cx, global, "bag", &a1);
        JS_GetProperty(cx, global, "bag", &a2);
        printf("INFO same object, same handle: %s\n", a1 == a2 ? "yes" : "no (identity compares need care)");
    }

    /* JS_SaveExceptionState round trip */
    run(cx, global, "throw 'one'", NULL);
    {
        JSExceptionState *st = JS_SaveExceptionState(cx);
        JS_ClearPendingException(cx);
        JS_RestoreExceptionState(cx, st);
        check("exception state restored", JS_IsExceptionPending(cx));
        JS_ClearPendingException(cx);
    }

    /* timings */
    t0 = JS_Now();
    run(cx, global, "for (var i = 0; i < 1000; i++) empty();", NULL);
    t1 = JS_Now();
    printf("INFO JS -> C empty native through gfosjsapi: %ld us per call\n", (long)((t1 - t0) / 1000));
    run(cx, global, "var g = new Vec(1, 2), sum = 0; for (var i = 0; i < 1000; i++) sum += g.x;", NULL);
    t0 = JS_Now();
    run(cx, global, "for (var i = 0; i < 1000; i++) sum += g.x;", NULL);
    t1 = JS_Now();
    printf("INFO JS -> C property getter: %ld us per get\n", (long)((t1 - t0) / 1000));
    JS_GetProperty(cx, global, "twice", &v);
    t0 = JS_Now();
    for (i = 0; i < 1000; i++) {
        jsval r;
        args[0] = INT_TO_JSVAL(i);
        JS_CallFunctionValue(cx, global, v, 1, args, &r);
        if (i % 100 == 99) { JS_EndRequest(cx); JS_BeginRequest(cx); JS_GetProperty(cx, global, "twice", &v); }
    }
    t1 = JS_Now();
    printf("INFO C -> JS call through gfosjsapi: %ld us per call\n", (long)((t1 - t0) / 1000));

    JS_EndRequest(cx);
    printf("INFO finalizers run so far: %d\n", finalized);
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    return failures;
}
