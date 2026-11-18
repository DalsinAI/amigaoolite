/* Oolite-to-GFOS javascript.library integration seam.
 * Uses the canonical GFOpenSDK vector ABI. Not a SpiderMonkey JSAPI shim.
 * GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_JAVASCRIPT_H
#define OOLITE_GFOS_JAVASCRIPT_H
#include <exec/types.h>

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
void OOGFOSJSClose(struct OOGFOSJS *state);
#endif
