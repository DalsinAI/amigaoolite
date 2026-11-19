/* Native AmigaOS adapter to the existing GFOS javascript.library.
 * Does not embed JavaScriptCore or SpiderMonkey.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/javascript.h>
#include <libraries/javascript.h>
#include <string.h>

/* Canonical proto/javascript.h declares this library base. */
struct Library *JavaScriptBase = NULL;
/* Several Oolite script owners may share the same library base. A closing
 * owner must never invalidate the remaining owners' canonical proto base. */
static ULONG sOpenOwners = 0;

static void ownerOpened(struct Library *base)
{
    Forbid();
    JavaScriptBase = base;
    ++sOpenOwners;
    Permit();
}

static void ownerClosed(void)
{
    Forbid();
    if (sOpenOwners && --sOpenOwners == 0) JavaScriptBase = NULL;
    Permit();
}

int OOGFOSJSOpen(struct OOGFOSJS *s)
{
    if (!s) return 0;
    memset(s, 0, sizeof(*s));
    s->library = OpenLibrary((CONST_STRPTR)JAVASCRIPTNAME, JAVASCRIPTVERSION);
    if (!s->library) return 0;
    ownerOpened(s->library);
    s->task = JS_NewTask(NULL);
    if (!s->task) {
        ownerClosed();
        CloseLibrary(s->library);
        s->library = NULL;
        return 0;
    }
    return 1;
}

LONG OOGFOSJSEvaluateLong(struct OOGFOSJS *s, CONST_STRPTR src,
                         CONST_STRPTR filename, LONG *out)
{
    JSValue result = NULL;
    LONG code;
    if (!s || !s->task || !src || !out) return JSERR_BADARGS;
    JavaScriptBase = s->library;
    code = JS_Evaluate(s->task, src, -1, filename ? filename : (CONST_STRPTR)"oolite.js", &result);
    if (code != JSERR_OK) return code;
    code = JS_ToLong(s->task, result, out);
    if (result) JS_Free(s->task, result);
    return code;
}

LONG OOGFOSJSErrorText(struct OOGFOSJS *s, STRPTR buffer, LONG size)
{
    if (!s || !s->task || !buffer || size < 1) return JSERR_BADARGS;
    JavaScriptBase = s->library;
    return JS_ErrorText(s->task, buffer, size);
}

int OOGFOSJSResetPrepared(struct OOGFOSJS *s,
                           OOGFOSJSPrepareTask prepare,
                           OOGFOSJSResetNotice beforeCommit,
                           OOGFOSJSResetNotice afterCommit,
                           APTR userData)
{
    JSTask replacement, previous;
    if (!s || !s->library || !s->task) return 0;
    JavaScriptBase = s->library;
    /* Prepare a new script heap before altering the current Oolite state. */
    replacement = JS_NewTask(NULL);
    if (!replacement) return 0;
    if (prepare && !prepare(replacement, userData)) {
        /* Aborted reset: the old task and its ship/script state still work. */
        JS_EndTask(replacement);
        return 0;
    }
    previous = s->task;
    if (beforeCommit) beforeCommit(userData);
    /* Old wrappers receive their final release callbacks before the
     * replacement becomes visible to Oolite's subsequent script calls. */
    JS_EndTask(previous);
    s->task = replacement;
    if (afterCommit) afterCommit(userData);
    return 1;
}

int OOGFOSJSReset(struct OOGFOSJS *s)
{
    return OOGFOSJSResetPrepared(s, NULL, NULL, NULL, NULL);
}

void OOGFOSJSClose(struct OOGFOSJS *s)
{
    if (!s || !s->library) return; /* Repeated close must not clear another owner's base. */
    JavaScriptBase = s->library;
    if (s->task) JS_EndTask(s->task);
    if (s->library) {
        ownerClosed();
        CloseLibrary(s->library);
    }
    s->task = NULL;
    s->library = NULL;
}

int OOGFOSJSExposeHostClass(struct OOGFOSJS *s,
                           const struct JSHostClass *hostClass,
                           APTR object, CONST_STRPTR globalName)
{
    JSClass klass;
    JSValue global = NULL, wrapper = NULL;
    LONG code = JSERR_BADARGS;
    if (!s || !s->task || !hostClass || !object || !globalName) return 0;
    JavaScriptBase = s->library;
    klass = JS_RegisterClass(s->task, hostClass);
    if (!klass) return 0;
    wrapper = JS_Wrap(s->task, klass, object);
    if (!wrapper) return 0;
    global = JS_Global(s->task);
    if (global) code = JS_SetProp(s->task, global, globalName, wrapper);
    if (global) JS_Free(s->task, global);
    JS_Free(s->task, wrapper);
    return code == JSERR_OK;
}

int OOGFOSJSSetGlobalFunction(struct OOGFOSJS *s,
                             CONST_STRPTR name, JSMethodFunc callback, APTR data)
{
    JSValue global = NULL, func = NULL;
    LONG code = JSERR_BADARGS;
    if (!s || !s->task || !name || !callback) return 0;
    JavaScriptBase = s->library;
    func = JS_NewFunction(s->task, name, callback, data);
    if (!func) return 0;
    global = JS_Global(s->task);
    if (global) code = JS_SetProp(s->task, global, name, func);
    if (global) JS_Free(s->task, global);
    JS_Free(s->task, func);
    return code == JSERR_OK;
}

LONG OOGFOSJSEvaluateSource(struct OOGFOSJS *s,
                            CONST_STRPTR src, CONST_STRPTR filename)
{
    if (!s || !s->task || !src) return JSERR_BADARGS;
    JavaScriptBase = s->library;
    return JS_Evaluate(s->task, src, -1,
                       filename ? filename : (CONST_STRPTR)"oolite.js", NULL);
}

LONG OOGFOSJSEvaluateChecked(struct OOGFOSJS *s, CONST_STRPTR src,
                             CONST_STRPTR filename, STRPTR errorBuffer,
                             LONG errorBufferSize)
{
    LONG rc;
    if (!errorBuffer || errorBufferSize < 1) return JSERR_BADARGS;
    errorBuffer[0] = 0;
    if (!s || !s->task || !src) return JSERR_BADARGS;
    rc = OOGFOSJSEvaluateSource(s, src, filename);
    if (rc != JSERR_OK)
        OOGFOSJSErrorText(s, errorBuffer, errorBufferSize);
    return rc;
}

