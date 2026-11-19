/* GFOS retained JSValue root semantics on native AC090.
 * JS_Dup owns a task-local handle: not SpiderMonkey address rooting.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSJSProperties.h"
#include <proto/javascript.h>
#include <stdio.h>
int main(void)
{
    struct OOGFOSJS js={0};
    JSValue global=NULL, obj=NULL, retained=NULL, value=NULL;
    LONG got=-1, rc=JSERR_BADARGS;
    int ok=0;
    if(!OOGFOSJSOpen(&js)) { puts("OOLITE_JS_RETAIN status=UNAVAILABLE"); return 77; }
    rc=OOGFOSJSEvaluateSource(&js,"var ooliteRootProbe={cargo:42};",
                               "oolite-root-setup.js");
    global=JS_Global(js.task);
    obj=global ? OOGFOSJSGetProperty(js.task,global,"ooliteRootProbe"):NULL;
    retained=obj ? OOGFOSJSRetainValue(js.task,obj):NULL;
    if(obj) OOGFOSJSReleaseValue(js.task,obj);
    obj=NULL;
    if(retained && rc==JSERR_OK) rc=OOGFOSJSEvaluateSource(&js,
        "delete this.ooliteRootProbe;","oolite-root-delete.js");
    if(retained && rc==JSERR_OK) value=OOGFOSJSGetProperty(js.task,retained,"cargo");
    if(value && JS_ToLong(js.task,value,&got)==JSERR_OK && got==42) ok=1;
    if(value) OOGFOSJSReleaseValue(js.task,value);
    if(retained) OOGFOSJSReleaseValue(js.task,retained);
    if(global) OOGFOSJSReleaseValue(js.task,global);
    OOGFOSJSClose(&js);
    printf("OOLITE_JS_RETAIN cargo=%ld status=%s\n",(long)got,ok?"PASS":"FAIL");
    return ok?0:2;
}
