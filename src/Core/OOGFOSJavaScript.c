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
#include <stdio.h>

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

/* Keep the task's source cache on the same AmigaDOS volume even after
 * a prepared scripting reset. The JavaScriptCore provider owns the bytes.
 * No SpiderMonkey XDR format is read or written here. */
static JSTask createScriptTask(const struct OOGFOSJS *s)
{
    struct TagItem tags[2];
    if(!s || !s->cacheDir[0])return JS_NewTask(NULL);
    tags[0].ti_Tag=JSA_CacheDir;
    tags[0].ti_Data=(ULONG)s->cacheDir;
    tags[1].ti_Tag=TAG_DONE;
    tags[1].ti_Data=0;
    return JS_NewTask(tags);
}

int OOGFOSJSOpenCached(struct OOGFOSJS *s, CONST_STRPTR cacheDir)
{
    size_t n=0;
    if (!s) return 0;
    memset(s,0,sizeof(*s));
    if(cacheDir && cacheDir[0]){
        n=strlen((const char*)cacheDir);
        if(n>=sizeof(s->cacheDir))return 0;
        memcpy(s->cacheDir,cacheDir,n+1);
    }
    s->library=OpenLibrary((CONST_STRPTR)JAVASCRIPTNAME,JAVASCRIPTVERSION);
    if(!s->library)return 0;
    ownerOpened(s->library);
    s->task=createScriptTask(s);
    if(!s->task){
        ownerClosed();
        CloseLibrary(s->library);
        s->library=NULL;
        return 0;
    }
    return 1;
}

int OOGFOSJSOpen(struct OOGFOSJS *s)
{
    return OOGFOSJSOpenCached(s,NULL);
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
    replacement = createScriptTask(s);
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

/* Original JS_InitClass exposes a named constructor and prototype. GFOS
 * JS_RegisterClass keeps a protected prototype internally. Retrieve it
 * through a real JS_Wrap instance and publish a constructor referring to
 * that exact prototype, without any new ABI vectors. */
static int validConstructorIdentifier(CONST_STRPTR value)
{
    size_t i;
    if(!value || !value[0]) return 0;
    for(i=0;value[i];i++){
        unsigned char c=(unsigned char)value[i];
        if(i>=63 || !((c>='a'&&c<='z') || (c>='A'&&c<='Z') ||
           c=='_' || (i>0 && c>='0'&&c<='9'))) return 0;
    }
    return 1;
}
int OOGFOSJSExposeHostConstructor(struct OOGFOSJS *s,
                                 CONST_STRPTR constructorName,
                                 CONST_STRPTR wrapperGlobal)
{
    char script[1000];
    int bytes;
    JSValue global=NULL,instance=NULL;
    if(!s || !s->library || !s->task ||
       !validConstructorIdentifier(constructorName) ||
       !validConstructorIdentifier(wrapperGlobal)) return 0;
    JavaScriptBase=s->library;
    global=JS_Global(s->task);
    if(!global)return 0;
    instance=JS_GetProp(s->task,global,wrapperGlobal);
    if(!instance || JS_TypeOf(s->task,instance)!=JST_OBJECT) {
        if(instance)JS_Free(s->task,instance);
        JS_Free(s->task,global);
        return 0;
    }
    JS_Free(s->task,instance);
    JS_Free(s->task,global);
    bytes=snprintf(script,sizeof(script),
      "(function(g){"
      "var p=Object.getPrototypeOf(g.%s);"
      "if(typeof g.%s==='function'){"
      "if(g.%s.prototype!==p)throw new TypeError('Oolite class conflict');"
      "return;}"
      "var C=function %s(){throw new TypeError('Oolite native class');};"
      "C.prototype=p;"
      "Object.defineProperty(p,'constructor',"
      "{value:C,writable:true,enumerable:false,configurable:true});"
      "Object.defineProperty(g,'%s',"
      "{value:C,writable:false,enumerable:false,configurable:false});"
      "})(this);",
      wrapperGlobal,constructorName,constructorName,constructorName,
      constructorName);
    if(bytes<0 || (size_t)bytes>=sizeof(script))return 0;
    return OOGFOSJSEvaluateSource(s,(CONST_STRPTR)script,
                    (CONST_STRPTR)"oolite-host-prototype.js")==JSERR_OK;
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

