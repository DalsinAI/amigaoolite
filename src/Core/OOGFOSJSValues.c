/* Shared javascript.library value conversion helpers.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJSValues.h"
#include <proto/javascript.h>
#include <math.h>

LONG OOGFOSJSValueToNumber(JSTask task, JSValue value, DOUBLE *out)
{
    if (!task || !out) return JSERR_BADARGS;
    return JS_ToDouble(task, value, out);
}
LONG OOGFOSJSValueToLong(JSTask task, JSValue value, LONG *out)
{
    if (!task || !out) return JSERR_BADARGS;
    return JS_ToLong(task, value, out);
}
/* ECMA-262 ToInt32 on a GFOS opaque number: truncate toward zero,
 * wrap modulo 2^32, then reinterpret the upper half as negative.
 * JS_ToLong is deliberately NOT used: it need not have these semantics. */
LONG OOGFOSJSValueToInt32(JSTask task, JSValue value, LONG *out)
{
    DOUBLE number, integral, wrapped;
    LONG rc;
    if (!task || !out) return JSERR_BADARGS;
    rc = JS_ToDouble(task, value, &number);
    if (rc != JSERR_OK) return rc;
    if (!isfinite(number) || number == 0.0) {
        *out = 0;
        return JSERR_OK;
    }
    integral = trunc(number);
    wrapped = fmod(integral, 4294967296.0);
    if (wrapped < 0.0) wrapped += 4294967296.0;
    if (wrapped >= 2147483648.0) wrapped -= 4294967296.0;
    *out = (LONG)wrapped;
    return JSERR_OK;
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
