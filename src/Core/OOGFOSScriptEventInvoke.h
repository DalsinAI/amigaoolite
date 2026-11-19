/* Oolite original script event bridge via synchronous game-task invocation.
 * Script callbacks run on the JS task; game owner pumps Exec RPC while waiting.
 * GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_SCRIPT_EVENT_INVOKE_H
#define OOLITE_GFOS_SCRIPT_EVENT_INVOKE_H
#include "OOGFOSScriptInvoke.h"
/* Returns JSERR_*; *called is 0 when script/handler absent, 1 if invoked.
 * JavaScript exceptions still send a completion and reach the game caller.
 * argv values must belong to this task and remain valid for the call.
 */
LONG OOGFOSScriptInvokeEvent(struct OOGFOSJS *js,
                            struct OOGFOSGameDispatch *dispatch,
                            CONST_STRPTR scriptName, CONST_STRPTR eventName,
                            LONG argc, JSValue *argv, LONG *called);
#endif
