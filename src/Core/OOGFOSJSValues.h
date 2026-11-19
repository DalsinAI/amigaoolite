/* GFOS value conversion seam for Oolite's SpiderMonkey-era OOJS bindings.
 * This is a SOURCE adapter, not a binary-compatible SpiderMonkey jsval.
 * JSValue handles always belong to one canonical JSTask.
 * GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_JS_VALUES_H
#define OOLITE_GFOS_JS_VALUES_H
#include <libraries/javascript.h>

/* JS_ToLong is not a guaranteed SpiderMonkey/ECMAScript ToInt32 modulo-2^32\n * conversion; keep this explicitly named ToLong until parity is proven.\n * All conversions report JSERR_* (including bad-argument and type failures)
 * separately from the converted result. Never confuse false with an error. */
LONG OOGFOSJSValueToNumber(JSTask task, JSValue value, DOUBLE *out);
LONG OOGFOSJSValueToLong(JSTask task, JSValue value, LONG *out);
/* ECMAScript ToInt32: truncate toward zero, modulo 2^32, signed range. */
LONG OOGFOSJSValueToInt32(JSTask task, JSValue value, LONG *out);
LONG OOGFOSJSValueToBool(JSTask task, JSValue value, LONG *out);
LONG OOGFOSJSNumberValue(JSTask task, CONST DOUBLE *number, JSValue *out);
LONG OOGFOSJSIntValue(JSTask task, LONG number, JSValue *out);
#endif
