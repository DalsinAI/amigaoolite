/* Isolated AC090 qualification of source-level property, index and call APIs.
 * Uses the installed canonical javascript.library and JavaScriptEngine.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSJSProperties.h"
#include <proto/javascript.h>
#include <stdio.h>
int main(void)
{
    struct OOGFOSJS js = {0};
    JSValue glob = NULL, obj = NULL, arr = NULL, v = NULL, get = NULL;
    JSValue method = NULL, answer = NULL, setVal = NULL;
    LONG n = -1, rc = JSERR_BADARGS;
    int ok = 0, failed = 0;
    if (!OOGFOSJSOpen(&js)) { puts("OOLITE_JS_PROPERTIES status=UNAVAILABLE"); return 77; }
    rc = OOGFOSJSEvaluateSource(&js,
        "var ooliteProbe={energy:42,items:[5,7],twice:function(x){return x*2;}};",
        "oolite-js-properties-gate.js");
    if (rc != JSERR_OK) { failed = 1; goto done; }
    glob = JS_Global(js.task);
    obj = glob ? OOGFOSJSGetProperty(js.task, glob, "ooliteProbe") : NULL;
    arr = obj ? OOGFOSJSGetProperty(js.task, obj, "items") : NULL;
    v = obj ? OOGFOSJSGetProperty(js.task, obj, "energy") : NULL;
    if (!obj || !arr || !v || JS_ToLong(js.task, v, &n) != JSERR_OK || n != 42)
    { failed = 2; goto done; }
    JS_Free(js.task, v); v = NULL;
    v = OOGFOSJSGetElement(js.task, arr, 1);
    if (!v || JS_ToLong(js.task, v, &n) != JSERR_OK || n != 7)
    { failed = 3; goto done; }
    JS_Free(js.task, v); v = NULL;
    setVal = JS_NewLong(js.task, 13);
    if (!setVal || OOGFOSJSSetElement(js.task, arr, 1, setVal) != JSERR_OK)
    { failed = 4; goto done; }
    get = OOGFOSJSGetElement(js.task, arr, 1);
    if (!get || JS_ToLong(js.task, get, &n) != JSERR_OK || n != 13)
    { failed = 5; goto done; }
    JS_Free(js.task, get); get = NULL;
    if (OOGFOSJSSetProperty(js.task, obj, "energy", setVal) != JSERR_OK)
    { failed = 6; goto done; }
    get = OOGFOSJSGetProperty(js.task, obj, "energy");
    if (!get || JS_ToLong(js.task, get, &n) != JSERR_OK || n != 13)
    { failed = 7; goto done; }
    JS_Free(js.task, get); get = NULL;
    method = OOGFOSJSGetProperty(js.task, obj, "twice");
    if (!method || OOGFOSJSCallFunction(js.task, method, obj, 1, &setVal, &answer) != JSERR_OK ||
        !answer || JS_ToLong(js.task, answer, &n) != JSERR_OK || n != 26)
    { failed = 8; goto done; }
    if (OOGFOSJSSetProperty(js.task, NULL, "bad", setVal) != JSERR_BADARGS ||
        OOGFOSJSCallFunction(js.task, NULL, obj, 0, NULL, NULL) != JSERR_BADARGS)
    { failed = 9; goto done; }
    ok = 1;
done:
    if (answer) JS_Free(js.task, answer);
    if (method) JS_Free(js.task, method);
    if (get) JS_Free(js.task, get);
    if (setVal) JS_Free(js.task, setVal);
    if (v) JS_Free(js.task, v);
    if (arr) JS_Free(js.task, arr);
    if (obj) JS_Free(js.task, obj);
    if (glob) JS_Free(js.task, glob);
    OOGFOSJSClose(&js);
    printf("OOLITE_JS_PROPERTIES eval=%ld result=%ld failure=%d status=%s\n",
           (long)rc, (long)n, failed, ok ? "PASS" : "FAIL");
    return ok ? 0 : 2;
}
