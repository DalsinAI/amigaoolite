/* Synchronous Oolite game-task -> GFOS JSTask dispatch without deadlock.
 * Worker completion is always sent by this layer, not by each script job.
 * GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_SCRIPT_INVOKE_H
#define OOLITE_GFOS_SCRIPT_INVOKE_H
#include "OOGFOSJavaScript.h"
#include "OOGFOSGameDispatch.h"
/* Called on javascript.library's script task; no direct game-entity access.
 * To read/modify an entity use its game task token/Exec proxy.
 * Sets a scalar result and returns JSERR_*; called once per invocation. */
typedef LONG (*OOGFOSScriptInvokeFunc)(JSTask task, APTR user, LONG *result);
/* Call from dispatch->gameTask ONLY. This pumps the game's Exec requests
 * until the worker sends DONE; then joins the JavaScript task's queue.
 * Return JSERR_* and write result on success. Never call from JS task. */
LONG OOGFOSScriptInvoke(struct OOGFOSJS *js,
                       struct OOGFOSGameDispatch *dispatch,
                       OOGFOSScriptInvokeFunc callback,
                       APTR user, LONG *result);
/* Source-level replacement for OOJavaScriptEngine -callJSFunction:.
 * function, self and argv are borrowed JSValue handles from js->task.
 * On success, *result is an OWNED JSValue and caller must JS_Free it before
 * resetting/ending the task; NULL represents JavaScript undefined.
 * On JSERR_* failure *result remains NULL. This is NOT a jsval binary ABI.
 * Script may synchronously invoke game-thread ShipEntity selectors.
 */
LONG OOGFOSScriptInvokeFunction(struct OOGFOSJS *js,
                               struct OOGFOSGameDispatch *dispatch,
                               JSValue function, JSValue self,
                               LONG argc, JSValue *argv, JSValue *result);

#endif
