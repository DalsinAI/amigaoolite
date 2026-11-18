#include "OOGFOSScriptObjects.h"
#include <proto/javascript.h>
#include <stdlib.h>
#include <string.h>

static int validName(CONST_STRPTR name)
{
    ULONG i;
    if (!name || !name[0]) return 0;
    for (i=0;name[i];i++) {
        unsigned char c=(unsigned char)name[i];
        if (i>=63 || !((c>='a' && c<='z') ||
                       (c>='A' && c<='Z') || c=='_' ||
                       (i>0 && c>='0' && c<='9'))) return 0;
    }
    return 1;
}
LONG OOGFOSScriptObjectCreate(JSTask task, CONST_STRPTR scriptGlobal,
                             CONST_STRPTR shipGlobal)
{
    JSValue global=NULL,ship=NULL,object=NULL;
    LONG rc=JSERR_BADARGS;
    if (!task || !validName(scriptGlobal) || !validName(shipGlobal))
        return JSERR_BADARGS;
    global=JS_Global(task);
    if (!global) goto cleanup;
    ship=JS_GetProp(task,global,shipGlobal);
    if (!ship || JS_TypeOf(task,ship)!=JST_OBJECT) goto cleanup;
    object=JS_NewObject(task);
    if (!object) {rc=JSERR_NOMEMORY;goto cleanup;}
    rc=JS_SetProp(task,object,(CONST_STRPTR)"ship",ship);
    if (rc!=JSERR_OK) goto cleanup;
    rc=JS_SetProp(task,global,scriptGlobal,object);
cleanup:
    if (object) JS_Free(task,object);
    if (ship) JS_Free(task,ship);
    if (global) JS_Free(task,global);
    return rc;
}
LONG OOGFOSScriptObjectLoad(JSTask task, CONST_STRPTR scriptGlobal,
                           CONST_STRPTR source, LONG length,
                           CONST_STRPTR filename)
{
    const char *prefix="(function(){\n";
    const char *middle="\n}).call(";
    const char *suffix=");";
    size_t n,prefixLength;
    char *wrapped;
    LONG rc;
    if (!task || !validName(scriptGlobal) || !source || length<1 ||
        length>262144) return JSERR_BADARGS;
    prefixLength=strlen(prefix);
    n=prefixLength+(size_t)length+strlen(middle)+strlen((const char *)scriptGlobal)+strlen(suffix);
    wrapped=(char *)malloc(n+1);
    if (!wrapped) return JSERR_NOMEMORY;
    memcpy(wrapped,prefix,prefixLength);
    memcpy(wrapped+prefixLength,source,(size_t)length);
    strcpy(wrapped+prefixLength+(size_t)length,middle);
    strcat(wrapped,(const char *)scriptGlobal);
    strcat(wrapped,suffix);
    rc=JS_Evaluate(task,(CONST_STRPTR)wrapped,-1,
                   filename ? filename : (CONST_STRPTR)"oolite-script.js",NULL);
    free(wrapped);
    return rc;
}
LONG OOGFOSScriptObjectEvent(JSTask task, CONST_STRPTR scriptGlobal,
                            CONST_STRPTR eventName, LONG argc,
                            JSValue *argv, LONG *called)
{
    JSValue global=NULL,script=NULL,func=NULL,result=NULL;
    LONG rc=JSERR_BADARGS;
    if (called) *called=0;
    if (!task || !validName(scriptGlobal) || !eventName || !eventName[0] ||
        argc<0 || (argc>0 && !argv)) return JSERR_BADARGS;
    global=JS_Global(task);
    if (!global) goto cleanup;
    script=JS_GetProp(task,global,scriptGlobal);
    if (!script || JS_TypeOf(task,script)!=JST_OBJECT) goto cleanup;
    func=JS_GetProp(task,script,eventName);
    if (!func || JS_TypeOf(task,func)==JST_UNDEFINED) {
        rc=JSERR_OK;goto cleanup;
    }
    if (JS_TypeOf(task,func)!=JST_FUNCTION) {
        rc=JSERR_TYPE;goto cleanup;
    }
    rc=JS_Call(task,func,script,argc,argv,&result);
    if (rc==JSERR_OK && called) *called=1;
cleanup:
    if(result)JS_Free(task,result);
    if(func)JS_Free(task,func);
    if(script)JS_Free(task,script);
    if(global)JS_Free(task,global);
    return rc;
}
