/* Script-task-owned JavaScriptCore regexp adapter for original OORegExpMatcher.
 * No legacy JSContext or JS_NewUCRegExpObjectNoStatics dependency.
 */
#include "OOGFOSRegExp.h"
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
static struct OOGFOSJS sRegexTask;
static unsigned sRefs;

struct RegexRequest {
    const char *source, *pattern;
    unsigned long flags;
    int matched;
    LONG error;
};

static LONG evaluateRegex(JSTask t, APTR opaque)
{
    struct RegexRequest *r=(struct RegexRequest*)opaque;
    JSValue global=NULL,ctor=NULL,pattern=NULL,flags=NULL;
    JSValue rx=NULL,test=NULL,source=NULL,result=NULL,argv[2];
    char flagString[4];
    unsigned n=0;
    LONG rc=JSERR_BADARGS;
    if(!r||!r->source||!r->pattern)return rc;
    r->matched=0;
    if(r->flags&1)flagString[n++]='i';
    if(r->flags&4)flagString[n++]='m';
    flagString[n]=0;
    global=JS_Global(t);
    if(!global)goto done;
    ctor=JS_GetProp(t,global,(CONST_STRPTR)"RegExp");
    pattern=JS_NewString(t,(CONST_STRPTR)r->pattern,-1);
    flags=JS_NewString(t,(CONST_STRPTR)flagString,n);
    if(!ctor||!pattern||!flags){rc=JSERR_NOMEMORY;goto done;}
    argv[0]=pattern;argv[1]=flags;
    rc=JS_Call(t,ctor,global,2,argv,&rx);
    if(rc!=JSERR_OK||!rx)goto done;
    test=JS_GetProp(t,rx,(CONST_STRPTR)"test");
    source=JS_NewString(t,(CONST_STRPTR)r->source,-1);
    if(!test||!source){rc=JSERR_NOMEMORY;goto done;}
    rc=JS_Call(t,test,rx,1,&source,&result);
    if(rc==JSERR_OK&&result)r->matched=JS_ToBool(t,result)?1:0;
done:
    r->error=rc;
    if(result)JS_Free(t,result);
    if(source)JS_Free(t,source);
    if(test)JS_Free(t,test);
    if(rx)JS_Free(t,rx);
    if(flags)JS_Free(t,flags);
    if(pattern)JS_Free(t,pattern);
    if(ctor)JS_Free(t,ctor);
    if(global)JS_Free(t,global);
    return rc;
}
int OOGFOSRegExpOpen(void)
{
    if(sRefs){++sRefs;return 1;}
    if(!OOGFOSJSOpen(&sRegexTask))return 0;
    sRefs=1;
    return 1;
}
void OOGFOSRegExpClose(void)
{
    if(sRefs&&--sRefs==0)OOGFOSJSClose(&sRegexTask);
}
int OOGFOSRegExpMatch(const char *source,const char *pattern,
                      unsigned long flags)
{
    struct RegexRequest r;
    LONG rc;
    if(!sRefs||!source||!pattern||!pattern[0]||(flags & ~5UL))
        return 0;
    r.source=source;r.pattern=pattern;r.flags=flags;
    r.matched=0;r.error=JSERR_BADARGS;
    JavaScriptBase=sRegexTask.library;
    rc=JS_Post(sRegexTask.task,evaluateRegex,&r,1);
    return rc==JSERR_OK&&r.error==JSERR_OK&&r.matched;
}
