/* Multiple Oolite script owners must not invalidate the shared library base.
 * Run in an isolated AmigaOS guest with JavaScriptEngine installed.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include <stdio.h>
extern struct Library *JavaScriptBase;

int main(void)
{
    struct OOGFOSJS first = {0}, second = {0};
    LONG a = -1, b = -1, rc1 = -1, rc2 = -1;
    int ok = 0;
    if (!OOGFOSJSOpen(&first)) {
        puts("OOLITE_JSLIB_TWO_OWNERS status=UNAVAILABLE");
        return 77;
    }
    if (!OOGFOSJSOpen(&second)) {
        OOGFOSJSClose(&first);
        puts("OOLITE_JSLIB_TWO_OWNERS status=UNAVAILABLE second_owner");
        return 77;
    }
    rc1 = OOGFOSJSEvaluateLong(&first, "6*7", "first-owner.js", &a);
    OOGFOSJSClose(&first);
    /* A closing owner may not clear the canonical proto base used by the
     * remaining owner or by its task callbacks. */
    if (JavaScriptBase == second.library)
        rc2 = OOGFOSJSEvaluateLong(&second, "7*8", "second-owner.js", &b);
    ok = rc1 == JSERR_OK && rc2 == JSERR_OK && a == 42 && b == 56;
    OOGFOSJSClose(&second);
    ok = ok && JavaScriptBase == NULL;
    printf("OOLITE_JSLIB_TWO_OWNERS first=%ld second=%ld base_cleared=%d status=%s\n",
           (long)a, (long)b, JavaScriptBase == NULL, ok ? "PASS" : "FAIL");
    return ok ? 0 : 2;
}
