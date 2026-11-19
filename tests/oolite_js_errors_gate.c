/* Oolite scripts: canonical JS exception/error text recovery gate.
 * Native AC090 68040; GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include <stdio.h>
#include <string.h>
int main(void)
{
    struct OOGFOSJS js = {0};
    char error[512];
    LONG threw, recovered, answer=-1;
    int ok;
    if(!OOGFOSJSOpen(&js)) { puts("OOLITE_JS_ERRORS status=UNAVAILABLE"); return 77; }
    threw=OOGFOSJSEvaluateChecked(&js,
        "throw new Error('oolite expected test exception');",
        "oolite-error-test.js",error,sizeof(error));
    ok=(threw==JSERR_EXCEPTION && error[0]!=0);
    recovered=OOGFOSJSEvaluateLong(&js, "6*7", "after-exception.js",&answer);
    ok=ok && recovered==JSERR_OK && answer==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_JS_ERRORS threw=%ld error_present=%d recovered=%ld result=%ld status=%s\n",
           (long)threw, error[0]!=0, (long)recovered, (long)answer,
           ok?"PASS":"FAIL");
    return ok?0:2;
}
