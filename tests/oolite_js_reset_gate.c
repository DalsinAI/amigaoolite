/* Oolite OOJavaScriptEngine-style reset against canonical javascript.library.
 * Native 68040 guest gate; no embedded SpiderMonkey.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include <stdio.h>
int main(void)
{
    struct OOGFOSJS state = {0};
    LONG before = -1, after = -1, rc1, rc2;
    int reset, ok;
    if (!OOGFOSJSOpen(&state)) {
        puts("OOLITE_JS_RESET status=UNAVAILABLE");
        return 77;
    }
    rc1 = OOGFOSJSEvaluateLong(&state,
        "var ooliteResetProbe=41; ooliteResetProbe;",
        "reset-before.js", &before);
    reset = OOGFOSJSReset(&state);
    rc2 = reset ? OOGFOSJSEvaluateLong(&state,
        "(typeof ooliteResetProbe === 'undefined') ? 42 : 0;",
        "reset-after.js", &after) : JSERR_BADARGS;
    ok = rc1 == JSERR_OK && before == 41 &&
         reset && rc2 == JSERR_OK && after == 42;
    OOGFOSJSClose(&state);
    printf("OOLITE_JS_RESET before=%ld reset=%d after=%ld status=%s\n",
           (long)before, reset, (long)after, ok ? "PASS" : "FAIL");
    return ok ? 0 : 2;
}
