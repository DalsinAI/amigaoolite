/* Transactional OOJavaScriptEngine replacement WITH game-task RPC pumping.
 * Oolite's original bootstrap calls real ship/object selectors while it
 * initializes JS classes. Never block the game task on an unpumped JS_Post.
 * GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_SCRIPT_RESET_H
#define OOLITE_GFOS_SCRIPT_RESET_H
#include "OOGFOSScriptInvoke.h"
/* prepare runs on the replacement script task and may call game-owned
 * entities ONLY via the dispatch's Exec token backend.
 * Return JSERR_OK on success; any other code aborts and keeps old task.
 * Old JSValue handles expire ONLY after a successful commit. */
typedef LONG (*OOGFOSScriptResetPrepare)(JSTask task, APTR user);
LONG OOGFOSScriptResetRouted(struct OOGFOSJS *js,
                            struct OOGFOSGameDispatch *dispatch,
                            OOGFOSScriptResetPrepare prepare,
                            OOGFOSJSResetNotice willReset,
                            OOGFOSJSResetNotice didReset,
                            APTR user);
#endif
