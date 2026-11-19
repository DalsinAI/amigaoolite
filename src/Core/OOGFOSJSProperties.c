/* Oolite reusable GFOS properties/calls adapter.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJSProperties.h"
#include <proto/javascript.h>
JSValue OOGFOSJSGetProperty(JSTask task, JSValue object, CONST_STRPTR name)
{
    if (!task || !object || !name) return NULL;
    return JS_GetProp(task, object, name);
}
LONG OOGFOSJSSetProperty(JSTask task, JSValue object, CONST_STRPTR name, JSValue value)
{
    if (!task || !object || !name) return JSERR_BADARGS;
    return JS_SetProp(task, object, name, value);
}
JSValue OOGFOSJSGetElement(JSTask task, JSValue array, ULONG index)
{
    if (!task || !array) return NULL;
    return JS_GetIndex(task, array, index);
}
LONG OOGFOSJSSetElement(JSTask task, JSValue array, ULONG index, JSValue value)
{
    if (!task || !array) return JSERR_BADARGS;
    return JS_SetIndex(task, array, index, value);
}
LONG OOGFOSJSCallFunction(JSTask task, JSValue function, JSValue self, LONG argc,
                         JSValue *args, JSValue *result)
{
    if (!task || !function || argc < 0 || (argc && !args)) return JSERR_BADARGS;
    if (result) *result = NULL;
    return JS_Call(task, function, self, argc, args, result);
}
