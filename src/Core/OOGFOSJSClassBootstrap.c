/* Oolite JS_InitClass-compatible source-level bootstrap over the canonical
 * GFOS javascript.library v2 append-only ClassPrototype API.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJSClassBootstrap.h"
#include <proto/javascript.h>
#include <proto/exec.h>
#include <exec/libraries.h>
#include <string.h>
#include <stdio.h>
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
    static const char temporaryName[]="_gfosOoliteNativeCtorFactory";
    static const char cleanup[]="delete this._gfosOoliteNativeCtorFactory";
    JSClass cls;
    JSValue global=NULL,proto=NULL,ctor=NULL,existing=NULL;
    JSValue factory=NULL,existingFactory=NULL;
    char source[256];
    int n,staged=0;
    LONG rc=JSERR_BADARGS;
    if(!js || !js->library || !js->task || !hostClass ||
       !validName(constructorName) ||
       strcmp((const char*)constructorName,temporaryName)==0 ||
       js->library->lib_Version<JAVASCRIPT_CLASS_PROTOTYPE_VERSION)
       return 0;
    JavaScriptBase=js->library;
    global=JS_Global(js->task);
    if(!global)return 0;
    existing=JS_GetProp(js->task,global,constructorName);
    if(existing && JS_TypeOf(js->task,existing)!=JST_UNDEFINED)
        goto done; /* Never replace a class previously installed by Oolite. */
    existingFactory=JS_GetProp(js->task,global,(CONST_STRPTR)temporaryName);
    if(existingFactory && JS_TypeOf(js->task,existingFactory)!=JST_UNDEFINED)
        goto done; /* Do not disturb an existing application global. */
    cls=JS_RegisterClass(js->task,hostClass);
    if(!cls)goto done;
    /* Works before ANY game-owned ship/entity instance exists. */
    proto=JS_ClassPrototype(js->task,cls);
    if(!proto)goto done;
    factory=JS_NewFunction(js->task,(CONST_STRPTR)"_gfosUnconstructable",
                           unconstructable,NULL);
    if(!factory)goto done;
    rc=JS_SetProp(js->task,global,(CONST_STRPTR)temporaryName,factory);
    if(rc!=JSERR_OK)goto done;
    staged=1;
    /* JavaScriptCore HostFunction objects are callable but do not provide
     * ordinary instanceof semantics even when .prototype is assigned.
     * Wrap the host callback in an actual JS constructor function, as
     * OOGFOSJSVector does. The original class name is validated above.
     * The native callback always throws, so no phantom Ship is constructed. */
    n=snprintf(source,sizeof(source),
        "(function(native){return function %s(){return native.apply(null,arguments);};})"
        "(this.%s)",(const char*)constructorName,temporaryName);
    if(n<0 || n>=(int)sizeof(source)){rc=JSERR_BADARGS;goto done;}
    rc=JS_Evaluate(js->task,(CONST_STRPTR)source,-1,
                   (CONST_STRPTR)"oolite-class-native-constructor.js",&ctor);
    if(rc!=JSERR_OK || !ctor)goto done;
    rc=JS_SetProp(js->task,ctor,(CONST_STRPTR)"prototype",proto);
    if(rc!=JSERR_OK)goto done;
    rc=JS_SetProp(js->task,proto,(CONST_STRPTR)"constructor",ctor);
    if(rc!=JSERR_OK)goto done;
    rc=JS_SetProp(js->task,global,constructorName,ctor);
done:
    if(staged)
        JS_Evaluate(js->task,(CONST_STRPTR)cleanup,-1,
                    (CONST_STRPTR)"oolite-class-factory-cleanup.js",NULL);
    if(existingFactory)JS_Free(js->task,existingFactory);
    if(existing)JS_Free(js->task,existing);
    if(factory)JS_Free(js->task,factory);
    if(ctor)JS_Free(js->task,ctor);
    if(proto)JS_Free(js->task,proto);
    JS_Free(js->task,global);
    return rc==JSERR_OK;

#else
    (void)js;(void)hostClass;(void)constructorName;
    return 0; /* Clean v1 SDK build: the v2 vector MUST NOT be called. */
#endif
}
