/* Oolite OOJSWorldScripts: registered native JS script objects visible to
 * other Oolite and OXP scripts, with stable enumerable read-only names.
 * GPL-2.0-or-later.
 */
#include "OOGFOSWorldScripts.h"
#include <proto/javascript.h>
#include <stdio.h>
static int validScriptIdentifier(CONST_STRPTR name)
{
    unsigned i;
    if(!name || !name[0])return 0;
    for(i=0;name[i];i++){
        unsigned char c=(unsigned char)name[i];
        if(i>=63 || !((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
           c=='_'||(i>0&&c>='0'&&c<='9')))return 0;
    }
    return 1;
}
static int validWorldName(CONST_STRPTR name)
{
    unsigned i;
    if(!name || !name[0])return 0;
    for(i=0;name[i];i++){
        unsigned char c=(unsigned char)name[i];
        if(i>=95 || !((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
           (c>='0'&&c<='9')||c=='_'||c=='-'||c=='.'))
            return 0;
    }
    return 1;
}
LONG OOGFOSWorldScriptsInit(JSTask task)
{
    JSValue global=NULL,world=NULL;
    LONG rc=JSERR_BADARGS;
    if(!task)return JSERR_BADARGS;
    global=JS_Global(task);
    if(!global)goto cleanup;
    world=JS_GetProp(task,global,"worldScripts");
    if(world && JS_TypeOf(task,world)==JST_OBJECT) {
        rc=JSERR_OK;goto cleanup;
    }
    if(world && JS_TypeOf(task,world)!=JST_UNDEFINED) {
        rc=JSERR_TYPE;goto cleanup;
    }
    if(world) {JS_Free(task,world);world=NULL;}
    world=JS_NewObject(task);
    if(!world){rc=JSERR_NOMEMORY;goto cleanup;}
    rc=JS_SetProp(task,global,"worldScripts",world);
    if(rc!=JSERR_OK)goto cleanup;
    rc=JS_Evaluate(task,
        "Object.defineProperty(this,'worldScripts',"
        "{value:this.worldScripts,writable:false,enumerable:true,configurable:false});",
        -1,"oolite-worldscripts-global.js",NULL);
cleanup:
    if(world)JS_Free(task,world);
    if(global)JS_Free(task,global);
    return rc;
}
LONG OOGFOSWorldScriptsRegister(JSTask task, CONST_STRPTR scriptGlobal,
                               CONST_STRPTR worldName)
{
    JSValue global=NULL,world=NULL,script=NULL;
    char source[320];int bytes;
    LONG rc=JSERR_BADARGS;
    if(!task || !validScriptIdentifier(scriptGlobal) ||
       !validWorldName(worldName))return JSERR_BADARGS;
    rc=OOGFOSWorldScriptsInit(task);
    if(rc!=JSERR_OK)return rc;
    global=JS_Global(task);
    if(!global)return JSERR_BADARGS;
    world=JS_GetProp(task,global,"worldScripts");
    script=JS_GetProp(task,global,scriptGlobal);
    if(!world || JS_TypeOf(task,world)!=JST_OBJECT ||
       !script || JS_TypeOf(task,script)!=JST_OBJECT){
        rc=JSERR_TYPE;goto cleanup;
    }
    rc=JS_SetProp(task,world,worldName,script);
    if(rc!=JSERR_OK)goto cleanup;
    /* Native GFOS property API sets writable enumerable fields. Conform
     * to the original OOJSWorldScripts read-only enumerated properties
     * using JS standard defineProperty. World names cannot contain a
     * quote, slash, control character or script syntax by validation. */
    bytes=snprintf(source,sizeof(source),
      "Object.defineProperty(worldScripts,'%s',"
      "{value:worldScripts['%s'],writable:false,"
      "enumerable:true,configurable:false});",worldName,worldName);
    if(bytes<0 || (unsigned)bytes>=sizeof(source)){
        rc=JSERR_BADARGS;goto cleanup;
    }
    rc=JS_Evaluate(task,(CONST_STRPTR)source,-1,
                   (CONST_STRPTR)"oolite-worldscripts-register.js",NULL);
cleanup:
    if(script)JS_Free(task,script);
    if(world)JS_Free(task,world);
    JS_Free(task,global);
    return rc;
}
