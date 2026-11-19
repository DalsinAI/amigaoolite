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
/* Expose a native callback on the JS global; callback runs on script task. */
int OOGFOSJSSetGlobalFunction(struct OOGFOSJS *state,
                             CONST_STRPTR name, JSMethodFunc callback, APTR data);
/* Evaluate a complete script for side effects, without coercing its result. */
LONG OOGFOSJSEvaluateSource(struct OOGFOSJS *state,
                            CONST_STRPTR source, CONST_STRPTR filename);
/* Recreate a fresh script task without replacing or closing javascript.library.
 * The old task is ended only after the new task starts successfully.
 * The caller must discard old JSValue handles and re-register host classes.
 * Return 1 on success; 0 leaves the old task unchanged.
 */
int OOGFOSJSReset(struct OOGFOSJS *state);
void OOGFOSJSClose(struct OOGFOSJS *state);
#endif
