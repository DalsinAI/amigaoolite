/* Host-class integration gate: genuine GFOpenSDK javascript.library calls.
 * Callbacks execute in the JavaScriptEngine task: no app libc calls there.
 */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stddef.h>
#include <stdio.h>
struct ShipProbe { LONG cargo; LONG fuel; };
static JSValue shipTonnage(JSTask task, APTR self, LONG argc, JSValue *argv, APTR data)
{
    const struct ShipProbe *ship = (const struct ShipProbe *)self;
    (void)argc; (void)argv; (void)data;
    return JS_NewLong(task, ship->cargo + ship->fuel);
}
static const struct JSHostSlot shipSlots[] = {
    { (CONST_STRPTR)"cargo", JSS_FIELD, JSF_LONG, NULL, NULL, offsetof(struct ShipProbe, cargo), 0 },
    { (CONST_STRPTR)"fuel", JSS_FIELD, JSF_LONG, NULL, NULL, offsetof(struct ShipProbe, fuel), 0 },
    { (CONST_STRPTR)"tonnage", JSS_METHOD, 0, (APTR)shipTonnage, NULL, 0, 0 },
    { NULL, 0, 0, NULL, NULL, 0, 0 }
};
static const struct JSHostClass shipClass = {
    (CONST_STRPTR)"OoliteShipProbe", NULL, shipSlots, NULL, NULL
};
int main(void)
{
    struct OOGFOSJS js;
    struct ShipProbe ship = { 30, 12 };
    LONG value = -1;
    LONG rc = JSERR_BADARGS;
    int wrapped;
    if (!OOGFOSJSOpen(&js)) {
        puts("OOLITE_JSLIB_HOSTCLASS status=UNAVAILABLE missing=javascript.library_or_engine");
        return 77;
    }
    wrapped = OOGFOSJSExposeHostClass(&js, &shipClass, &ship, (CONST_STRPTR)"ship");
    if (wrapped) rc = OOGFOSJSEvaluateLong(&js,
             (CONST_STRPTR)"ship.cargo + ship.fuel + (ship.tonnage() - 42)",
             (CONST_STRPTR)"oolite-hostclass.js", &value);
    printf("OOLITE_JSLIB_HOSTCLASS registered=%d rc=%ld value=%ld status=%s\n",
           wrapped, (long)rc, (long)value,
           wrapped && rc == JSERR_OK && value == 42 ? "PASS" : "FAIL");
    OOGFOSJSClose(&js);
    return wrapped && rc == JSERR_OK && value == 42 ? 0 : 2;
}
