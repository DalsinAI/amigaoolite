/* Oolite JS_InitClass-compatible source-level bootstrap over the canonical
 * GFOS javascript.library v2 append-only ClassPrototype API.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJSClassBootstrap.h"
#include <proto/javascript.h>
#include <proto/exec.h>
#include <exec/libraries.h>
#include <string.h>
static int validName(CONST_STRPTR value)
{
    size_t i;
    if(!value || !value[0])return 0;
    for(i=0;value[i];i++){
        unsigned char c=(unsigned char)value[i];
        if(i>=63 || !((c>='a' && c<='z') ||
           (c>='A' && c<='Z') || c=='_' ||
           (i>0 && c>='0' && c<='9')))return 0;
    }
    return 1;
}
static JSValue unconstructable(JSTask task, APTR self, LONG argc,
                               JSValue *argv, APTR userdata)
{
    (void)self;(void)argc;(void)argv;(void)userdata;
    return JS_Throw(task,(CONST_STRPTR)"Oolite native class cannot be constructed");
}
int OOGFOSJSBootstrapClass(struct OOGFOSJS *js,
                           const struct JSHostClass *hostClass,
                           CONST_STRPTR constructorName)
{
#if defined(JAVASCRIPT_CLASS_PROTOTYPE_VERSION) && JAVASCRIPT_CLASS_PROTOTYPE_VERSION >= 2
    JSClass cls;
    JSValue global=NULL,proto=NULL,ctor=NULL,existing=NULL;
    LONG rc=JSERR_BADARGS;
    if(!js || !js->library || !js->task || !hostClass ||
       !validName(constructorName) ||
       js->library->lib_Version<JAVASCRIPT_CLASS_PROTOTYPE_VERSION)
       return 0;
    JavaScriptBase=js->library;
    global=JS_Global(js->task);
    if(!global)return 0;
    existing=JS_GetProp(js->task,global,constructorName);
    if(existing && JS_TypeOf(js->task,existing)!=JST_UNDEFINED)
        goto done; /* Never replace a class previously installed by Oolite. */
    cls=JS_RegisterClass(js->task,hostClass);
    if(!cls)goto done;
    /* Unlike the earlier wrapper-derived workaround this works at startup
     * when there are NO game-owned ship/entity objects yet. */
    proto=JS_ClassPrototype(js->task,cls);
    if(!proto)goto done;
    ctor=JS_NewFunction(js->task,constructorName,unconstructable,NULL);
    if(!ctor)goto done;
    rc=JS_SetProp(js->task,ctor,(CONST_STRPTR)"prototype",proto);
    if(rc!=JSERR_OK)goto done;
    rc=JS_SetProp(js->task,proto,(CONST_STRPTR)"constructor",ctor);
    if(rc!=JSERR_OK)goto done;
    rc=JS_SetProp(js->task,global,constructorName,ctor);
done:
    if(existing)JS_Free(js->task,existing);
    if(ctor)JS_Free(js->task,ctor);
    if(proto)JS_Free(js->task,proto);
    JS_Free(js->task,global);
    return rc==JSERR_OK;
#else
    (void)js;(void)hostClass;(void)constructorName;
    return 0; /* Clean v1 SDK build: the v2 vector MUST NOT be called. */
#endif
}
