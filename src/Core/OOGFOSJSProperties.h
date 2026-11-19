/* GFOS property/call seam for Oolite's original OOJS migration.
 * Not binary-compatible with SpiderMonkey JSObject or jsval.
 * GPL-2.0-or-later.
 */
#ifndef OOGFOS_JS_PROPERTIES_H
#define OOGFOS_JS_PROPERTIES_H
#include <libraries/javascript.h>
/* Returned handles belong to caller until JS_Free(). NULL means undefined
 * for the canonical ABI, not a SpiderMonkey null JSObject. */
JSValue OOGFOSJSGetProperty(JSTask task, JSValue object, CONST_STRPTR name);
LONG OOGFOSJSSetProperty(JSTask task, JSValue object, CONST_STRPTR name, JSValue value);
JSValue OOGFOSJSGetElement(JSTask task, JSValue array, ULONG index);
LONG OOGFOSJSSetElement(JSTask task, JSValue array, ULONG index, JSValue value);
/* Script-owned functions can be called only on their owning JSTask. */
LONG OOGFOSJSCallFunction(JSTask task, JSValue function, JSValue self, LONG argc,
                         JSValue *args, JSValue *result);
/* Explicit retained-value ownership replaces the old JSAPI pattern of
 * rooting a jsval* address. Never use a retained value after task reset. */
JSValue OOGFOSJSRetainValue(JSTask task, JSValue value);
VOID OOGFOSJSReleaseValue(JSTask task, JSValue value);
#endif
