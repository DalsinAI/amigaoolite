/*
 * oojsl: javascript.library under clash-free names (see oojsl.h).
 * Team 5, 19 Nov 2026. MIT.
 */
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/javascript.h>
#include <string.h>

#include "oojsl.h"

static JSTask sTask;
static JSClass sBoxClass;
static ooLRelease sRelease;

/* A function made by ooL_NewFunction: the library calls this with self NULL;
 * argv[0] is `this` (the glue in gfosjsapi passes it). */
struct OOLFuncRec { ooLFunc func; void *data; };

static JSValue funcTrampoline(JSTask task, APTR self, LONG argc, JSValue *argv, APTR data)
{
    struct OOLFuncRec *r = (struct OOLFuncRec *)data;
    (void)task; (void)self;
    return (JSValue)r->func(argc, (ooLValue *)argv, r->data);
}

static VOID boxRelease(JSTask task, APTR object, APTR data)
{
    (void)task; (void)data;
    if (sRelease)
        sRelease(object);
}

static const struct JSHostSlot sBoxSlots[] = { { NULL, 0, 0, NULL, NULL, 0, 0 } };
static const struct JSHostClass sBoxHostClass = { (CONST_STRPTR)"OOJSObject", NULL, sBoxSlots, boxRelease, NULL };

void ooL_SetTask(void *task) { sTask = (JSTask)task; }
void *ooL_Task(void) { return sTask; }

int ooL_Init(ooLRelease release)
{
    sRelease = release;
    sBoxClass = JS_RegisterClass(sTask, &sBoxHostClass);
    return sBoxClass != NULL;
}

long ooL_Evaluate(const char *source, long length, const char *name, ooLValue *result)
{
    return JS_Evaluate(sTask, (CONST_STRPTR)source, length, (CONST_STRPTR)name, (JSValue *)result);
}
ooLValue ooL_Global(void) { return JS_Global(sTask); }
void ooL_Free(ooLValue v) { if (v) JS_Free(sTask, (JSValue)v); }
ooLValue ooL_Dup(ooLValue v) { return v ? JS_Dup(sTask, (JSValue)v) : NULL; }
long ooL_TypeOf(ooLValue v) { return v ? JS_TypeOf(sTask, (JSValue)v) : OOL_UNDEFINED; }
ooLValue ooL_NewLong(long n) { return JS_NewLong(sTask, n); }
ooLValue ooL_NewDouble(double d) { DOUBLE x = d; return JS_NewDouble(sTask, &x); }
ooLValue ooL_NewBool(int b) { return JS_NewBool(sTask, b ? 1 : 0); }
ooLValue ooL_NewNull(void) { return JS_NewNull(sTask); }
ooLValue ooL_NewString(const char *utf8, long length) { return JS_NewString(sTask, (CONST_STRPTR)utf8, length); }
ooLValue ooL_NewObject(void) { return JS_NewObject(sTask); }

ooLValue ooL_NewFunction(const char *name, ooLFunc func, void *data)
{
    /* Lives as long as the program: functions are made once per class/spec. */
    struct OOLFuncRec *r = (struct OOLFuncRec *)AllocVec(sizeof *r, MEMF_ANY);
    if (!r)
        return NULL;
    r->func = func;
    r->data = data;
    return JS_NewFunction(sTask, (CONST_STRPTR)name, funcTrampoline, r);
}

long ooL_ToLong(ooLValue v, long *result) { LONG n = 0; LONG rc = JS_ToLong(sTask, (JSValue)v, &n); *result = n; return rc; }
long ooL_ToDouble(ooLValue v, double *result) { DOUBLE d = 0; LONG rc = JS_ToDouble(sTask, (JSValue)v, &d); *result = d; return rc; }
int ooL_ToBool(ooLValue v) { return v ? JS_ToBool(sTask, (JSValue)v) != 0 : 0; }
long ooL_ToString(ooLValue v, char *buffer, long size) { return JS_ToString(sTask, (JSValue)v, (STRPTR)buffer, size); }
ooLValue ooL_GetProp(ooLValue obj, const char *name) { return JS_GetProp(sTask, (JSValue)obj, (CONST_STRPTR)name); }
long ooL_SetProp(ooLValue obj, const char *name, ooLValue value) { return JS_SetProp(sTask, (JSValue)obj, (CONST_STRPTR)name, (JSValue)value); }
ooLValue ooL_GetIndex(ooLValue obj, unsigned long index) { return JS_GetIndex(sTask, (JSValue)obj, index); }
long ooL_SetIndex(ooLValue obj, unsigned long index, ooLValue value) { return JS_SetIndex(sTask, (JSValue)obj, index, (JSValue)value); }

long ooL_Call(ooLValue func, ooLValue self, long argc, ooLValue *argv, ooLValue *result)
{
    return JS_Call(sTask, (JSValue)func, (JSValue)self, argc, (JSValue *)argv, (JSValue *)result);
}

ooLValue ooL_WrapBox(void *box) { return JS_Wrap(sTask, sBoxClass, box); }
void *ooL_UnwrapBox(ooLValue v) { return v ? JS_Unwrap(sTask, (JSValue)v, sBoxClass) : NULL; }
void ooL_Throw(const char *message) { JS_Throw(sTask, (CONST_STRPTR)message); }
long ooL_ErrorText(char *buffer, long size) { return JS_ErrorText(sTask, (STRPTR)buffer, size); }
