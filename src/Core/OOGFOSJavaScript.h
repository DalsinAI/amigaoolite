/* Oolite-to-GFOS javascript.library integration seam.
 * Uses the canonical GFOpenSDK vector ABI. Not a SpiderMonkey JSAPI shim.
 * GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_JAVASCRIPT_H
#define OOLITE_GFOS_JAVASCRIPT_H
#include <exec/types.h>
#include <libraries/javascript.h>

struct OOGFOSJS {
    APTR task;
    struct Library *library;
};

/* 1 for available; 0 if LIBS:javascript.library or its engine is absent. */
int OOGFOSJSOpen(struct OOGFOSJS *state);
/* Return JSERR_*; on success *value is the expression's integer result. */
LONG OOGFOSJSEvaluateLong(struct OOGFOSJS *state, CONST_STRPTR source,
                         CONST_STRPTR filename, LONG *value);
/* Report engine's most recent error, length excluding the final NUL. */
LONG OOGFOSJSErrorText(struct OOGFOSJS *state, STRPTR buffer, LONG size);
/* Host object must outlive its JS wrapper; JSHostClass slots stay valid. */
int OOGFOSJSExposeHostClass(struct OOGFOSJS *state,
                           const struct JSHostClass *hostClass,
                           APTR object, CONST_STRPTR globalName);
/* Expose the canonical JSHostClass wrapper's real prototype as a named
 * unconstructable Oolite class constructor (e.g. Ship.prototype).
 * Oolite's original oolite-global-prefix.js installs methods there.
 * Requires a bound instance (wrapperGlobal) of that class and names that
 * are valid JavaScript identifiers. This is a SOURCE-compatibility bridge
 * for JS_InitClass; it does not claim binary SpiderMonkey ABI layout. */
int OOGFOSJSExposeHostConstructor(struct OOGFOSJS *state,
                                 CONST_STRPTR constructorName,
                                 CONST_STRPTR wrapperGlobal);

/* Expose a native callback on the JS global; callback runs on script task. */
int OOGFOSJSSetGlobalFunction(struct OOGFOSJS *state,
                             CONST_STRPTR name, JSMethodFunc callback, APTR data);
/* Evaluate a complete script for side effects, without coercing its result. */
LONG OOGFOSJSEvaluateSource(struct OOGFOSJS *state,
                            CONST_STRPTR source, CONST_STRPTR filename);
/* Evaluate and copy the library's last diagnostic only on failure.
 * The result code stays JSERR_*; buffer is blank on successful execution. */
LONG OOGFOSJSEvaluateChecked(struct OOGFOSJS *state, CONST_STRPTR source,
                             CONST_STRPTR filename, STRPTR errorBuffer,
                             LONG errorBufferSize);
/* Recreate a fresh script task without replacing or closing javascript.library.
 * The old task is ended only after the new task starts successfully.
 * The caller must discard old JSValue handles and re-register host classes.
 * Return 1 on success; 0 leaves the old task unchanged.
 */
int OOGFOSJSReset(struct OOGFOSJS *state);
/* Oolite engine reset transaction: prepare all script classes and global
 * objects on the replacement task before retiring the old one. Prepare must
 * only register resources owned by the replacement task; on failure that
 * task is disposed and the original task and its state remain live.
 * beforeCommit and afterCommit run on the caller's (game) task.
 * The caller owns time-limiter/frame-callback cancellation and notifications.
 * Do not hold live JSValue handles from an earlier task across a commit.
 */
typedef int (*OOGFOSJSPrepareTask)(JSTask candidate, APTR userData);
typedef void (*OOGFOSJSResetNotice)(APTR userData);
int OOGFOSJSResetPrepared(struct OOGFOSJS *state,
                           OOGFOSJSPrepareTask prepare,
                           OOGFOSJSResetNotice beforeCommit,
                           OOGFOSJSResetNotice afterCommit,
                           APTR userData);

void OOGFOSJSClose(struct OOGFOSJS *state);
#endif
