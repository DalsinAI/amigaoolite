/* GFOS Oolite JavaScript script-object lifecycle and event calls on ONE
 * shared JSTask. No SpiderMonkey APIs. GPL-2.0-or-later. */
#ifndef OOLITE_GFOS_SCRIPT_OBJECTS_H
#define OOLITE_GFOS_SCRIPT_OBJECTS_H
#include <libraries/javascript.h>
/* Call ONLY on the JavaScriptEngine task, normally from JS_Post. The
 * caller prepares a global script object and exposes its ship host wrapper
 * before loading. This wraps original JS in a function called with the
 * script object as this, instead of executing it in the global scope. */
/* Construct a per-ship JS object and attach the already-registered host
 * ship object by name. Both names are validated JS identifiers. Must run
 * on JSTask; scripts share one engine task but have distinct this objects. */
LONG OOGFOSScriptObjectCreate(JSTask task, CONST_STRPTR scriptGlobal,
                             CONST_STRPTR shipGlobal);
LONG OOGFOSScriptObjectLoad(JSTask task, CONST_STRPTR scriptGlobal,
                           CONST_STRPTR source, LONG length,
                           CONST_STRPTR filename);
/* Original OOJSScript passes UTF-16 code units without a BOM from NSString.
 * Decode to strict UTF-8 before calling the canonical JS_Evaluate API.
 * unitCount is in native-endian 16-bit words, not bytes. Embedded NUL and
 * malformed surrogate sequences are rejected rather than truncated. */
LONG OOGFOSScriptObjectLoadUTF16(JSTask task, CONST_STRPTR scriptGlobal,
                                CONST UWORD *source, LONG unitCount,
                                CONST_STRPTR filename);
/* Look up the event function as a member of the script object and call with
 * self=script. Missing handlers are not an error: called=0. Error returns
 * JSERR_* from GFOS javascript.library. */
/* Remove global script reference. Does not force GC or invalidate ship token;
 * owner must coordinate job completion then unregister ship separately. */
LONG OOGFOSScriptObjectDrop(JSTask task, CONST_STRPTR scriptGlobal);
LONG OOGFOSScriptObjectEvent(JSTask task, CONST_STRPTR scriptGlobal,
                            CONST_STRPTR eventName, LONG argc,
                            JSValue *argv, LONG *called);
/* Original OOJSScript -callMethod:result: semantics: return arbitrary
 * owned JSValue to the game caller rather than discarding it. Unlike the
 * historical jsval word this is an opaque per-JSTask handle. Caller must
 * JS_Free the returned value before reset, even when it is an object.
 * A missing event is success with called=0 and *outResult=NULL.
 * Execute on JSTask or through OOGFOSScriptInvokeEventValue. */
LONG OOGFOSScriptObjectEventValue(JSTask task, CONST_STRPTR scriptGlobal,
                                 CONST_STRPTR eventName, LONG argc,
                                 JSValue *argv, JSValue *outResult,
                                 LONG *called);

#endif
