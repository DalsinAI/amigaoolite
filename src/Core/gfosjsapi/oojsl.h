/*
 * oojsl: javascript.library, under names that don't clash with SpiderMonkey's
 * (the library's own calls are JS_NewObject, JS_Call, ... too). Only oojsl.c
 * includes the library's headers. Handles are void *: a JSValue (NULL is
 * undefined), the JSTask, a host class.
 * Team 5, 19 Nov 2026. MIT.
 */
#ifndef GFOSJSAPI_OOJSL_H
#define GFOSJSAPI_OOJSL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *ooLValue;

/* JST_ */
enum { OOL_UNDEFINED = 0, OOL_NULL, OOL_BOOLEAN, OOL_NUMBER, OOL_STRING, OOL_OBJECT, OOL_FUNCTION, OOL_SYMBOL, OOL_BIGINT };
enum { OOL_OK = 0, OOL_EXCEPTION = 1 };

/* A C function scripts can call: argv[0] is `this`. Return a new value
 * (ooL_Dup/ooL_New*) or NULL for undefined. */
typedef ooLValue (*ooLFunc)(long argc, ooLValue *argv, void *data);
/* Called when a boxed object's wrapper is collected. */
typedef void (*ooLRelease)(void *box);

/* The JSTask everything runs on. The program creates it and runs its game on
 * it (JS_Post); ooL_SetTask must be called before any other ooL_ call. */
void ooL_SetTask(void *task);
void *ooL_Task(void);
int ooL_Init(ooLRelease release);          /* registers the box class */

long ooL_Evaluate(const char *source, long length, const char *name, ooLValue *result);
ooLValue ooL_Global(void);
void ooL_Free(ooLValue v);
ooLValue ooL_Dup(ooLValue v);
long ooL_TypeOf(ooLValue v);
ooLValue ooL_NewLong(long n);
ooLValue ooL_NewDouble(double d);
ooLValue ooL_NewBool(int b);
ooLValue ooL_NewNull(void);
ooLValue ooL_NewString(const char *utf8, long length);
ooLValue ooL_NewObject(void);
ooLValue ooL_NewFunction(const char *name, ooLFunc func, void *data);
long ooL_ToLong(ooLValue v, long *result);
long ooL_ToDouble(ooLValue v, double *result);
int ooL_ToBool(ooLValue v);
long ooL_ToString(ooLValue v, char *buffer, long size);   /* full UTF-8 length, -1 on error */
ooLValue ooL_GetProp(ooLValue obj, const char *name);
long ooL_SetProp(ooLValue obj, const char *name, ooLValue value);
ooLValue ooL_GetIndex(ooLValue obj, unsigned long index);
long ooL_SetIndex(ooLValue obj, unsigned long index, ooLValue value);
long ooL_Call(ooLValue func, ooLValue self, long argc, ooLValue *argv, ooLValue *result);
ooLValue ooL_WrapBox(void *box);           /* the one wrapper for box */
void *ooL_UnwrapBox(ooLValue v);           /* the box, or NULL */
void ooL_Throw(const char *message);
long ooL_ErrorText(char *buffer, long size);

#ifdef __cplusplus
}
#endif

#endif
