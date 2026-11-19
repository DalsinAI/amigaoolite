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
    /* Original OOJSScript constructs its object with sScriptPrototype
     * from JS_InitClass. Script.prototype methods installed by the
     * unmodified oolite-global-prefix.js MUST be inherited by each script.
     * During early tests/startup, where Script is not registered yet, use
     * a normal object as before. All objects remain on the same JSTask.
     * No SpiderMonkey-private object layouts are involved. */
    rc=JS_Evaluate(task,
       (CONST_STRPTR)
       "typeof Script==='function' && Script.prototype !== null && "
       "typeof Script.prototype==='object' "
       "? Object.create(Script.prototype) : ({})",
       -1,(CONST_STRPTR)"oolite-script-instance.js",&object);
    if(rc!=JSERR_OK)goto cleanup;
    if(!object || JS_TypeOf(task,object)!=JST_OBJECT) {
        rc=JSERR_TYPE;goto cleanup;
    }
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
LONG OOGFOSScriptObjectDrop(JSTask task, CONST_STRPTR scriptGlobal)
{
    JSValue global;
    LONG rc;
    if (!task || !validName(scriptGlobal)) return JSERR_BADARGS;
    global=JS_Global(task);
    if (!global) return JSERR_BADARGS;
    /* GFOS NULL JSValue represents JavaScript undefined. Dropping the global
     * reference does not forcibly destroy scripts held by other objects. */
    rc=JS_SetProp(task,global,scriptGlobal,NULL);
    JS_Free(task,global);
    return rc;
}
LONG OOGFOSScriptObjectEventValue(JSTask task, CONST_STRPTR scriptGlobal,
                                 CONST_STRPTR eventName, LONG argc,
                                 JSValue *argv, JSValue *outResult,
                                 LONG *called)
{
    JSValue global=NULL,script=NULL,func=NULL,returned=NULL;
    LONG rc=JSERR_BADARGS;
    if(called) *called=0;
    if(outResult) *outResult=NULL;
    if(!outResult || !task || !validName(scriptGlobal) ||
       !eventName || !eventName[0] || argc<0 ||
       (argc>0 && !argv))return JSERR_BADARGS;
    global=JS_Global(task);
    if(!global)goto cleanup;
    script=JS_GetProp(task,global,scriptGlobal);
    if(!script || JS_TypeOf(task,script)==JST_UNDEFINED) {
        rc=JSERR_OK;goto cleanup;
    }
    if(JS_TypeOf(task,script)!=JST_OBJECT)goto cleanup;
    func=JS_GetProp(task,script,eventName);
    if(!func || JS_TypeOf(task,func)==JST_UNDEFINED) {
        rc=JSERR_OK;goto cleanup;
    }
    if(JS_TypeOf(task,func)!=JST_FUNCTION) {
        rc=JSERR_TYPE;goto cleanup;
    }
    rc=JS_Call(task,func,script,argc,argv,&returned);
    if(rc==JSERR_OK) {
        *outResult=returned;
        returned=NULL; /* caller owns the returned handle */
        if(called)*called=1;
    }
cleanup:
    if(returned)JS_Free(task,returned); /* exception result must not escape */
    if(func)JS_Free(task,func);
    if(script)JS_Free(task,script);
    if(global)JS_Free(task,global);
    return rc;
}
LONG OOGFOSScriptObjectEvent(JSTask task, CONST_STRPTR scriptGlobal,
                            CONST_STRPTR eventName, LONG argc,
                            JSValue *argv, LONG *called)
{
    JSValue ignored=NULL;
    LONG rc=OOGFOSScriptObjectEventValue(task,scriptGlobal,eventName,
                                        argc,argv,&ignored,called);
    if(ignored)JS_Free(task,ignored);
    return rc;
}

/* Decode native-endian unichar input of OOJSScript.m (JS_CompileUCScript)
 * to canonical javascript.library UTF-8. Reject invalid UTF-16 rather than
 * silently replacing invalid surrogate pairs or truncating embedded NULs. */
LONG OOGFOSScriptObjectLoadUTF16(JSTask task, CONST_STRPTR scriptGlobal,
                                CONST UWORD *source, LONG unitCount,
                                CONST_STRPTR filename)
{
    unsigned char *utf8;
    size_t i=0, n=0, capacity;
    LONG rc;
    if (!task || !validName(scriptGlobal) || !source ||
        unitCount < 1 || unitCount > 262144) return JSERR_BADARGS;
    /* Max 3 UTF-8 bytes per BMP word. A pair encodes in 4 bytes. */
    capacity = (size_t)unitCount * 3u + 1u;
    utf8=(unsigned char *)malloc(capacity);
    if (!utf8) return JSERR_NOMEMORY;
    while (i < (size_t)unitCount) {
        unsigned long scalar=(unsigned long)source[i++];
        if (!scalar) {rc=JSERR_BADARGS;goto cleanup;}
        if (scalar >= 0xD800ul && scalar <= 0xDBFFul) {
            unsigned long trail;
            if (i == (size_t)unitCount) {rc=JSERR_BADARGS;goto cleanup;}
            trail=(unsigned long)source[i++];
            if (trail < 0xDC00ul || trail > 0xDFFFul) {
                rc=JSERR_BADARGS;goto cleanup;
            }
            scalar=0x10000ul + ((scalar-0xD800ul)<<10) +
                   (trail-0xDC00ul);
        } else if (scalar >= 0xDC00ul && scalar <= 0xDFFFul) {
            rc=JSERR_BADARGS;goto cleanup;
        }
        if (scalar < 0x80ul) utf8[n++]=(unsigned char)scalar;
        else if (scalar < 0x800ul) {
            utf8[n++]=(unsigned char)(0xC0ul|(scalar>>6));
            utf8[n++]=(unsigned char)(0x80ul|(scalar&0x3Ful));
        } else if (scalar < 0x10000ul) {
            utf8[n++]=(unsigned char)(0xE0ul|(scalar>>12));
            utf8[n++]=(unsigned char)(0x80ul|((scalar>>6)&0x3Ful));
            utf8[n++]=(unsigned char)(0x80ul|(scalar&0x3Ful));
        } else {
            utf8[n++]=(unsigned char)(0xF0ul|(scalar>>18));
            utf8[n++]=(unsigned char)(0x80ul|((scalar>>12)&0x3Ful));
            utf8[n++]=(unsigned char)(0x80ul|((scalar>>6)&0x3Ful));
            utf8[n++]=(unsigned char)(0x80ul|(scalar&0x3Ful));
        }
    }
    utf8[n]=0;
    rc=OOGFOSScriptObjectLoad(task,scriptGlobal,
                             (CONST_STRPTR)utf8,(LONG)n,filename);
cleanup:
    free(utf8);
    return rc;
}
