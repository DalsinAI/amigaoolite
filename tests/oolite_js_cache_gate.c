/* Qualify canonical JavaScriptCore source cache on a persistent AmigaDOS
 * volume across Oolite JSTask reset. GPL-2.0-or-later.
 * This does not assert bytecode acceleration without inspecting the files.
 */
#include "OOGFOSJavaScript.h"
#include <stdio.h>
#include <string.h>
int main(void)
{
    struct OOGFOSJS js={0};
    LONG first=0,second=0,modified=0;
    LONG rc1,rc2,rc3;
    int reset,ok;
    if(OOGFOSJSOpenCached(&js,(CONST_STRPTR)"SYS:OoliteCache/PortTest")==0){
        puts("OOLITE_JS_CACHE status=UNAVAILABLE");return 77;
    }
    if(strcmp(js.cacheDir,"SYS:OoliteCache/PortTest")!=0){
        OOGFOSJSClose(&js);puts("OOLITE_JS_CACHE status=WRONG_PATH");return 2;
    }
    rc1=OOGFOSJSEvaluateLong(&js,(CONST_STRPTR)"6*7",
                            (CONST_STRPTR)"oolite-script.js",&first);
    reset=OOGFOSJSReset(&js);
    rc2=reset?OOGFOSJSEvaluateLong(&js,(CONST_STRPTR)"6*7",
                      (CONST_STRPTR)"oolite-script.js",&second):JSERR_NOENGINE;
    rc3=reset?OOGFOSJSEvaluateLong(&js,(CONST_STRPTR)"9*11",
                      (CONST_STRPTR)"oolite-script.js",&modified):JSERR_NOENGINE;
    ok=rc1==JSERR_OK&&rc2==JSERR_OK&&rc3==JSERR_OK&&
       first==42&&second==42&&modified==99&&reset==1&&
       strcmp(js.cacheDir,"SYS:OoliteCache/PortTest")==0;
    OOGFOSJSClose(&js);
    printf("OOLITE_JS_CACHE first=%ld repeat=%ld modified=%ld reset=%d rc=%ld,%ld,%ld status=%s\n",
           (long)first,(long)second,(long)modified,reset,
           (long)rc1,(long)rc2,(long)rc3,ok?"PASS":"FAIL");
    return ok?0:2;
}
