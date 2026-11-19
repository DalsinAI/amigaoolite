/* Original OOJSScript source bridge using existing GFOS Javascript
 * lifecycle, Exec game-task pump, canonical shared JSTask, and UTF16 loader.
 * No SpiderMonkey binary-ABI aliases, fake class structs, or second GC.
 * SPDX-License-Identifier: GPL-2.0-or-later.
 */
#include "OOGFOSOriginalScriptBridge.h"
#include "OOGFOSJavaScript.h"
#include "OOGFOSGameDispatch.h"
#include "OOGFOSScriptInvoke.h"
#include "OOGFOSScriptEventInvoke.h"
#include "OOGFOSScriptObjects.h"
#include <proto/javascript.h>
#include <proto/exec.h>
#include <string.h>
#include <limits.h>

static struct OOGFOSJS *sOriginalJS;
static struct OOGFOSGameDispatch *sOriginalDispatch;
static int bound(void)
{
    return sOriginalJS && sOriginalJS->task && sOriginalJS->library &&
           sOriginalDispatch && sOriginalDispatch->gamePort &&
           !sOriginalDispatch->sealed &&
           FindTask(NULL)==sOriginalDispatch->gameTask;
}
int OOGFOSOriginalScriptBind(struct OOGFOSJS *js,
                            struct OOGFOSGameDispatch *dispatch)
{
    if(!js || !js->library || !js->task || !dispatch ||
       !dispatch->gamePort || dispatch->sealed ||
       FindTask(NULL)!=dispatch->gameTask)return 0;
    sOriginalJS=js;
    sOriginalDispatch=dispatch;
    return 1;
}
void OOGFOSOriginalScriptUnbind(void)
{
    if(sOriginalDispatch &&
       FindTask(NULL)==sOriginalDispatch->gameTask){
        sOriginalJS=NULL;
        sOriginalDispatch=NULL;
    }
}
static LONG setProperty(JSTask task,const char *key,
                        const struct OOGFOSOriginalProperty *p)
{
    JSValue value=NULL;
    LONG rc=JSERR_BADARGS;
    DOUBLE number;
    if(!p || !p->name || !p->name[0])return rc;
    switch(p->kind){
    case OOGFOS_PROP_STRING:
        if(!p->utf8)return rc;
        value=JS_NewString(task,p->utf8,-1);
        break;
    case OOGFOS_PROP_DOUBLE:
        number=p->number;
        value=JS_NewDouble(task,&number);
        break;
    case OOGFOS_PROP_BOOLEAN:
        value=JS_NewBool(task,p->number!=0.0);
        break;
    default:return rc;
    }
    if(!value)return JSERR_NOMEMORY;
    rc=OOGFOSScriptObjectSetProperty(task,key,p->name,value,p->readonly?1:0);
    JS_Free(task,value);
    return rc;
}
static LONG readProperty(JSTask task,const char *key,const char *property,
                         char *buffer,uint32_t capacity)
{
    JSValue value=NULL;
    LONG rc,used;
    if(!buffer || capacity<2 || capacity>INT32_MAX)return JSERR_BADARGS;
    buffer[0]=0;
    rc=OOGFOSScriptObjectGetProperty(task,key,property,&value);
    if(rc==JSERR_TYPE && !value)return JSERR_OK; /* optional metadata */
    if(rc!=JSERR_OK)return rc;
    if(!value)return JSERR_OK;
    if(JS_TypeOf(task,value)!=JST_UNDEFINED &&
       JS_TypeOf(task,value)!=JST_NULL){
        used=JS_ToString(task,value,buffer,(LONG)capacity);
        if(used<0 || (uint32_t)used>=capacity){
            buffer[0]=0;
            rc=JSERR_BADARGS;
        }
    }
    JS_Free(task,value);
    return rc;
}
static LONG installScriptClass(JSTask task,APTR unused,LONG *ignored)
{
    JSValue returned=NULL;
    LONG rc;
    (void)unused;
    if(ignored)*ignored=0;
    /* Preserve original class identity: Script.prototype is stable and
     * extensible by unmodified oolite-global-prefix.js. Construction
     * outside the Oolite loader is deliberately forbidden. */
    rc=JS_Evaluate(task,
        (CONST_STRPTR)
        "(function(g){"
        "if(typeof g.Script!=='undefined')"
        " throw new Error('Script already registered');"
        "var S=function Script(){"
        "throw new TypeError('Script is an Oolite native class');};"
        "Object.defineProperty(g,'Script',"
        "{value:S,writable:false,enumerable:false,configurable:false});"
        "})(this);",
        -1,(CONST_STRPTR)"oolite-original-script-class.js",&returned);
    if(returned)JS_Free(task,returned);
    return rc;
}
int32_t OOGFOSOriginalScriptInitClasses(void)
{
    LONG unused=0;
    if(!bound())return JSERR_BADARGS;
    return OOGFOSScriptInvoke(sOriginalJS,sOriginalDispatch,
                              installScriptClass,NULL,&unused);
}

struct Load {
    const char *key,*fileName,*defaultName;
    const uint16_t *utf16;
    uint32_t units,propertyCount;
    const struct OOGFOSOriginalProperty *props;
    char *name,*description,*version;
    uint32_t nameSize,descriptionSize,versionSize;
};
static LONG loadOnTask(JSTask task,APTR opaque,LONG *unused)
{
    struct Load *job=(struct Load *)opaque;
    struct OOGFOSOriginalProperty initialName;
    LONG rc=JSERR_BADARGS;
    uint32_t i;
    if(unused)*unused=0;
    if(!job || !job->key || !job->utf16 || !job->units ||
       job->units>262144 || job->propertyCount>64 ||
       (job->propertyCount && !job->props))return rc;
    rc=OOGFOSScriptObjectCreate(task,job->key,NULL);
    if(rc!=JSERR_OK)return rc;
    for(i=0;i<job->propertyCount;i++){
        rc=setProperty(task,job->key,&job->props[i]);
        if(rc!=JSERR_OK)goto fail;
    }
    /* Exactly as original initWithPath: this is set before running
     * the script; its JS source is free to replace this default name. */
    initialName.name="name";
    initialName.utf8=job->defaultName?job->defaultName:job->key;
    initialName.number=0.0;
    initialName.kind=OOGFOS_PROP_STRING;
    initialName.readonly=0;
    rc=setProperty(task,job->key,&initialName);
    if(rc!=JSERR_OK)goto fail;
    rc=OOGFOSScriptObjectLoadUTF16(task,job->key,job->utf16,
                    (LONG)job->units,job->fileName);
    if(rc!=JSERR_OK)goto fail;
    rc=readProperty(task,job->key,"name",job->name,job->nameSize);
    if(rc!=JSERR_OK)goto fail;
    rc=readProperty(task,job->key,"description",job->description,
                    job->descriptionSize);
    if(rc!=JSERR_OK)goto fail;
    rc=readProperty(task,job->key,"version",job->version,
                    job->versionSize);
    if(rc!=JSERR_OK)goto fail;
    return JSERR_OK;
fail:
    (void)OOGFOSScriptObjectDrop(task,job->key);
    return rc;
}
int32_t OOGFOSOriginalScriptLoad(const char *scriptKey,
                                const uint16_t *utf16,uint32_t units,
                                const char *fileName,
                                const struct OOGFOSOriginalProperty *properties,
                                uint32_t propertyCount,
                                const char *defaultName,
                                char *name,uint32_t nameSize,
                                char *description,uint32_t descriptionSize,
                                char *version,uint32_t versionSize)
{
    struct Load job;
    LONG unused=0;
    if(!bound() || !scriptKey || !utf16 || !units || !name ||
       !description || !version || !nameSize || !descriptionSize ||
       !versionSize || (propertyCount && !properties))return JSERR_BADARGS;
    name[0]=description[0]=version[0]=0;
    memset(&job,0,sizeof(job));
    job.key=scriptKey;job.fileName=fileName;
    job.utf16=utf16;job.units=units;job.props=properties;
    job.propertyCount=propertyCount;job.defaultName=defaultName;
    job.name=name;job.nameSize=nameSize;
    job.description=description;job.descriptionSize=descriptionSize;
    job.version=version;job.versionSize=versionSize;
    return OOGFOSScriptInvoke(sOriginalJS,sOriginalDispatch,
                              loadOnTask,&job,&unused);
}
struct PropertyJob {
    const char *key;
    const struct OOGFOSOriginalProperty *property;
};
static LONG setOnTask(JSTask task,APTR opaque,LONG *unused)
{
    const struct PropertyJob *job=(const struct PropertyJob *)opaque;
    if(unused)*unused=0;
    return setProperty(task,job->key,job->property);
}
int32_t OOGFOSOriginalScriptSetProperty(const char *scriptKey,
                                       const struct OOGFOSOriginalProperty *property)
{
    struct PropertyJob job={scriptKey,property};
    LONG ignored=0;
    if(!bound() || !scriptKey || !property)return JSERR_BADARGS;
    return OOGFOSScriptInvoke(sOriginalJS,sOriginalDispatch,
                              setOnTask,&job,&ignored);
}
struct GetJob {
    const char *key,*property;
    char *output;
    uint32_t capacity;
};
static LONG getOnTask(JSTask task,APTR opaque,LONG *unused)
{
    const struct GetJob *job=(const struct GetJob *)opaque;
    if(unused)*unused=0;
    return readProperty(task,job->key,job->property,
                        job->output,job->capacity);
}
int32_t OOGFOSOriginalScriptGetProperty(const char *scriptKey,
                                       const char *property,
                                       char *output,uint32_t capacity)
{
    struct GetJob job={scriptKey,property,output,capacity};
    LONG ignored=0;
    if(!bound() || !scriptKey || !property || !output || capacity<2)
        return JSERR_BADARGS;
    return OOGFOSScriptInvoke(sOriginalJS,sOriginalDispatch,
                              getOnTask,&job,&ignored);
}
struct DropJob {const char *key;};
static LONG dropOnTask(JSTask task,APTR opaque,LONG *unused)
{
    const struct DropJob *job=(const struct DropJob *)opaque;
    if(unused)*unused=0;
    return OOGFOSScriptObjectDrop(task,job->key);
}
int32_t OOGFOSOriginalScriptDrop(const char *scriptKey)
{
    struct DropJob job={scriptKey};
    LONG ignored=0;
    if(!bound() || !scriptKey)return JSERR_BADARGS;
    return OOGFOSScriptInvoke(sOriginalJS,sOriginalDispatch,
                              dropOnTask,&job,&ignored);
}
int32_t OOGFOSOriginalScriptCallMethod(const char *scriptKey,
                                      const char *method,uint32_t argc,
                                      OOGFOSOwnedJSValue *args,
                                      OOGFOSOwnedJSValue *result,
                                      int32_t *called)
{
    LONG invoked=0;
    JSValue value=NULL;
    LONG rc;
    if(result)*result=NULL;
    if(called)*called=0;
    if(!bound() || !scriptKey || !method || !result || !called ||
       argc>INT32_MAX || (argc && !args))return JSERR_BADARGS;
    rc=OOGFOSScriptInvokeEventValue(sOriginalJS,sOriginalDispatch,
         scriptKey,method,(LONG)argc,(JSValue *)args,&value,&invoked);
    if(rc==JSERR_OK){
        *result=(OOGFOSOwnedJSValue)value;
        *called=invoked;
    }
    return rc;
}
void OOGFOSOriginalScriptReleaseValue(OOGFOSOwnedJSValue result)
{
    if(bound() && result){
        JavaScriptBase=sOriginalJS->library;
        JS_Free(sOriginalJS->task,(JSValue)result);
    }
}
