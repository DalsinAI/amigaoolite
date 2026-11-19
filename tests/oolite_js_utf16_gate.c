/* Original Oolite OOJSScript UTF-16 loader on canonical GFOS JS engine.
 * Native AC090 68040 regression, GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSScriptObjects.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <string.h>
struct Test {
    LONG valid, badSurrogate, badNul, badTrailing, called, result;
};
static size_t appendAscii(UWORD *dst,size_t pos,const char *text)
{
    while (*text) dst[pos++]=(UWORD)(unsigned char)*text++;
    return pos;
}
static LONG run(JSTask task, APTR user)
{
    struct Test *t=(struct Test *)user;
    JSValue global=NULL,object=NULL;
    UWORD source[300],bad[]={'t','h','i','s','.',0xD800,';'},
          nul[]={'x','=',0,'1',';'},trailing[]={'x','=',0xDC00,';'};
    size_t n=0;
    LONG called=0;
    JSValue val=NULL, fn=NULL;
    global=JS_Global(task);
    if(!global) return JSERR_BADARGS;
    object=JS_NewObject(task);
    if(!object) {JS_Free(task,global);return JSERR_NOMEMORY;}
    if(JS_SetProp(task,global,(CONST_STRPTR)"script",object)!=JSERR_OK)
        goto fail;
    n=appendAscii(source,n,"this.note='");
    source[n++]=0x00E9; /* U+00E9, two-byte UTF-8 */
    source[n++]=0xD83D; source[n++]=0xDE80; /* U+1F680, UTF-16 pair */
    n=appendAscii(source,n,
        "';this.shipSpawned=function(){return this.note.length===3 && "
        "this.note.charCodeAt(0)===233 && "
        "this.note.charCodeAt(1)===55357 && "
        "this.note.charCodeAt(2)===56960 ? 42 : 0;};");
    t->valid=OOGFOSScriptObjectLoadUTF16(task,"script",source,(LONG)n,
                                         "oolite-utf16-source.js");
    t->badSurrogate=OOGFOSScriptObjectLoadUTF16(task,"script",bad,7,"bad-surrogate.js");
    t->badNul=OOGFOSScriptObjectLoadUTF16(task,"script",nul,5,"bad-nul.js");
    t->badTrailing=OOGFOSScriptObjectLoadUTF16(task,"script",trailing,4,"bad-low.js");
    fn=JS_GetProp(task,object,(CONST_STRPTR)"shipSpawned");
    if(fn && JS_TypeOf(task,fn)==JST_FUNCTION &&
       JS_Call(task,fn,object,0,NULL,&val)==JSERR_OK &&
       val && JS_ToLong(task,val,&t->result)==JSERR_OK)
        called=1;
    if(val)JS_Free(task,val);
    if(fn)JS_Free(task,fn);
    t->called=called;
fail:
    JS_Free(task,object);
    JS_Free(task,global);
    return JSERR_OK;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct Test t={-1,-1,-1,-1,0,-1};
    LONG posted=-1;
    int ok;
    if(!OOGFOSJSOpen(&js)){
        puts("OOLITE_JS_UTF16 status=UNAVAILABLE");
        return 77;
    }
    posted=JS_Post(js.task,run,&t,1);
    ok=posted==JSERR_OK && t.valid==JSERR_OK &&
       t.badSurrogate==JSERR_BADARGS && t.badNul==JSERR_BADARGS &&
       t.badTrailing==JSERR_BADARGS && t.called==1 && t.result==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_JS_UTF16 posted=%ld valid=%ld malformed=%ld nul=%ld low=%ld called=%ld result=%ld status=%s\n",
       (long)posted,(long)t.valid,(long)t.badSurrogate,(long)t.badNul,
       (long)t.badTrailing,(long)t.called,(long)t.result,
       ok?"PASS":"FAIL");
    return ok?0:2;
}
