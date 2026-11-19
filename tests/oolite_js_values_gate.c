/* Native 68040 JSValue conversion seam qualification.
 * Tests actual javascript.library, not a mock interpreter.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSJSValues.h"
#include <proto/javascript.h>
#include <stdio.h>
int main(void)
{
    struct OOGFOSJS js = {0};
    JSValue number = NULL, integer = NULL, negative = NULL, zero = NULL;
    DOUBLE input = 42.5, converted = 0;
    LONG intResult = -1, negativeResult = 0, yes = -1, no = -1, zeroResult = -1;
    LONG rc1, rc2, rc3, rc4, rc5;
    int ok;
    if (!OOGFOSJSOpen(&js)) {
        puts("OOLITE_JS_VALUES status=UNAVAILABLE");
        return 77;
    }
    rc1 = OOGFOSJSNumberValue(js.task, &input, &number);
    rc2 = rc1 == JSERR_OK ?
        OOGFOSJSValueToNumber(js.task, number, &converted) : JSERR_BADARGS;
    rc3 = OOGFOSJSIntValue(js.task, 42, &integer);
    rc4 = rc3 == JSERR_OK ?
        OOGFOSJSValueToLong(js.task, integer, &intResult) : JSERR_BADARGS;
    rc5 = rc4 == JSERR_OK ?
        OOGFOSJSValueToBool(js.task, integer, &yes) : JSERR_BADARGS;
    if (rc5 == JSERR_OK) rc5 = OOGFOSJSValueToBool(js.task, NULL, &no);
    /* Negative and zero values are common in ship/state properties; ensure
     * their signed conversions and JavaScript falsiness are preserved. */
    negative = JS_NewLong(js.task, -42);
    zero = JS_NewLong(js.task, 0);
    if (negative && zero) {
        rc5 = OOGFOSJSValueToLong(js.task, negative, &negativeResult);
        if (rc5 == JSERR_OK)
            rc5 = OOGFOSJSValueToBool(js.task, zero, &zeroResult);
    } else rc5 = JSERR_NOMEMORY;
    ok = rc1 == JSERR_OK && rc2 == JSERR_OK && rc3 == JSERR_OK &&
         rc4 == JSERR_OK && rc5 == JSERR_OK &&
         converted == input && intResult == 42 && negativeResult == -42 && 
         yes == 1 && no == 0 && zeroResult == 0 &&
         OOGFOSJSValueToLong(js.task, integer, NULL) == JSERR_BADARGS &&
         OOGFOSJSNumberValue(js.task, NULL, &number) == JSERR_BADARGS;
    if (number) JS_Free(js.task, number);
    if (integer) JS_Free(js.task, integer); 
    if (negative) JS_Free(js.task, negative); 
    if (zero) JS_Free(js.task, zero);
    OOGFOSJSClose(&js);
    printf("OOLITE_JS_VALUES double=%d long=%ld true=%ld false=%ld status=%s\n",
           converted == input, (long)intResult, (long)negativeResult, (long)yes, (long)no, (long)zeroResult,
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 2;
}
