/* Shared javascript.library value conversion helpers.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJSValues.h"
#include <proto/javascript.h>

LONG OOGFOSJSValueToNumber(JSTask task, JSValue value, DOUBLE *out)
{
    if (!task || !out) return JSERR_BADARGS;
    return JS_ToDouble(task, value, out);
}
LONG OOGFOSJSValueToInt32(JSTask task, JSValue value, LONG *out)
{
    if (!task || !out) return JSERR_BADARGS;
    return JS_ToLong(task, value, out);
}
LONG OOGFOSJSValueToBool(JSTask task, JSValue value, LONG *out)
{
    if (!task || !out) return JSERR_BADARGS;
    *out = JS_ToBool(task, value) ? 1 : 0;
    return JSERR_OK;
}
LONG OOGFOSJSNumberValue(JSTask task, CONST DOUBLE *number, JSValue *out)
{
    if (!task || !number || !out) return JSERR_BADARGS;
    *out = NULL;
    *out = JS_NewDouble(task, number);
    return *out ? JSERR_OK : JSERR_NOMEMORY;
}
LONG OOGFOSJSIntValue(JSTask task, LONG number, JSValue *out)
{
    if (!task || !out) return JSERR_BADARGS;
    *out = NULL;
    *out = JS_NewLong(task, number);
    return *out ? JSERR_OK : JSERR_NOMEMORY;
}
