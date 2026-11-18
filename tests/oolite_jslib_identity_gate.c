/* Oolite/GFOS ship identity gate against real javascript.library ABI. */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <stddef.h>
struct OoliteShip { LONG hull; LONG energy; };
static const struct JSHostSlot slots[] = {
    { (CONST_STRPTR)"hull", JSS_FIELD, JSF_LONG, NULL, NULL, offsetof(struct OoliteShip, hull), 0 },
    { (CONST_STRPTR)"energy", JSS_FIELD, JSF_LONG, NULL, NULL, offsetof(struct OoliteShip, energy), 0 },
    { NULL,0,0,NULL,NULL,0,0 }
};
static const struct JSHostClass klass = {
    (CONST_STRPTR)"OoliteIdentityShip", NULL, slots, NULL, NULL
};
int main(void) {
    struct OOGFOSJS js;
    struct OoliteShip first = {101, 44}, second = {202, 88};
    JSClass cls = NULL;
    JSValue a = NULL, b = NULL, c = NULL, global = NULL;
    LONG value = -1, rc = JSERR_BADARGS;
    int same = 0, distinct = 0, pointerOK = 0, passed = 0;
    if (!OOGFOSJSOpen(&js)) {
        puts("OOLITE_JS_IDENTITY status=UNAVAILABLE"); return 77;
    }
    cls = JS_RegisterClass(js.task, &klass);
    if (cls) {
        a = JS_Wrap(js.task, cls, &first);
        b = JS_Wrap(js.task, cls, &first);
        c = JS_Wrap(js.task, cls, &second);
        if (a && b && c) {
            pointerOK = JS_Unwrap(js.task, a, cls) == (APTR)&first &&
                        JS_Unwrap(js.task, b, cls) == (APTR)&first &&
                        JS_Unwrap(js.task, c, cls) == (APTR)&second;
            global = JS_Global(js.task);
            if (global && JS_SetProp(js.task, global, (CONST_STRPTR)"shipA", a) == JSERR_OK &&
                          JS_SetProp(js.task, global, (CONST_STRPTR)"shipB", b) == JSERR_OK &&
                          JS_SetProp(js.task, global, (CONST_STRPTR)"shipC", c) == JSERR_OK) {
                rc = OOGFOSJSEvaluateLong(&js,
                    (CONST_STRPTR)"shipA===shipB && shipA!==shipC && shipA.hull===101 && shipB.energy===44 && shipC.hull===202 ? 1 : 0",
                    (CONST_STRPTR)"oolite-identity.js", &value);
                same = value == 1;
                distinct = value == 1;
            }
        }
    }
    passed = pointerOK && same && distinct && rc == JSERR_OK;
    printf("OOLITE_JS_IDENTITY pointers=%d same=%d distinct=%d rc=%ld status=%s\n",
           pointerOK, same, distinct, (long)rc, passed ? "PASS" : "FAIL");
    fflush(stdout);
    if (global) JS_Free(js.task, global);
    if (a) JS_Free(js.task, a);
    if (b) JS_Free(js.task, b);
    if (c) JS_Free(js.task, c);
    OOGFOSJSClose(&js);
    return passed ? 0 : 2;
}
