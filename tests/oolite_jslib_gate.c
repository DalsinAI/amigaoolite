#include "OOGFOSJavaScript.h"
#include <stdio.h>
int main(void)
{
    struct OOGFOSJS js;
    LONG value = -1, rc;
    if (!OOGFOSJSOpen(&js)) {
        puts("OOLITE_JSLIB status=UNAVAILABLE missing=javascript.library_or_engine");
        return 77; /* Not a pass: requires installed javascript.library + engine. */
    }
    rc = OOGFOSJSEvaluateLong(&js, "6*7", "oolite-jslib-gate.js", &value);
    printf("OOLITE_JSLIB rc=%ld value=%ld status=%s\n",
           (long)rc, (long)value, rc == 0 && value == 42 ? "PASS" : "FAIL");
    OOGFOSJSClose(&js);
    return rc == 0 && value == 42 ? 0 : 2;
}
