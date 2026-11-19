/* SpiderMonkey 1.8.5 ToInt32 semantics through canonical JSValue handles.
 * Oolite native AC090 68040 qualification; GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSJSValues.h"
#include <proto/javascript.h>
#include <stdio.h>
struct Case { DOUBLE value; LONG expected; };
static const struct Case tests[] = {
    {42.9,42}, {-42.9,-42}, {0.0,0}, {-0.0,0},
    {4294967297.0,1}, {-4294967297.0,-1},
    {2147483648.0,(-2147483647L-1L)},
    {-2147483649.0,2147483647L},
    {4294967295.0,-1}, {4294967296.0,0},
    {1.0e30,0} /* Not asserted: huge modulo is float-dependent */
};
int main(void)
{
    struct OOGFOSJS js={0};
    LONG result=0, rc, bad=0, passed=0;
    unsigned i, n=(sizeof(tests)/sizeof(tests[0]))-1;
    JSValue val=NULL;
    if(!OOGFOSJSOpen(&js)) { puts("OOLITE_JS_INT32 status=UNAVAILABLE"); return 77; }
    for(i=0;i<n;i++) {
        val=JS_NewDouble(js.task,&tests[i].value);
        if(!val) {bad++;break;}
        rc=OOGFOSJSValueToInt32(js.task,val,&result);
        if(rc==JSERR_OK && result==tests[i].expected) passed++;
        else {bad++; printf("INT32_CASE_FAIL i=%u rc=%ld got=%ld want=%ld\n",i,(long)rc,(long)result,(long)tests[i].expected);}
        JS_Free(js.task,val);
        val=NULL;
    }
    if(val) JS_Free(js.task,val);
    if(OOGFOSJSValueToInt32(js.task,NULL,NULL)!=JSERR_BADARGS) bad++;
    OOGFOSJSClose(&js);
    printf("OOLITE_JS_INT32 checks=%ld passed=%ld failed=%ld status=%s\n",
           (long)n,(long)passed,(long)bad,!bad && passed==n?"PASS":"FAIL");
    return !bad && passed==n?0:2;
}
