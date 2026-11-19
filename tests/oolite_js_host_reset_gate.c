/* Oolite host-class release and rebind during shared JS task reset.
 * Native AC090 test of task-owned wrapper lifetime (no fake interpreter).
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stddef.h>
#include <stdio.h>
struct ShipProbe { LONG cargo; LONG fuel; };
static LONG releaseCount = 0;
static VOID shipReleased(JSTask task, APTR self, APTR data)
{
    (void)task; (void)self; (void)data;
    ++releaseCount;
}
static const struct JSHostSlot slots[] = {
    { (CONST_STRPTR)"cargo", JSS_FIELD, JSF_LONG, NULL, NULL, offsetof(struct ShipProbe, cargo), 0 },
    { (CONST_STRPTR)"fuel", JSS_FIELD, JSF_LONG, NULL, NULL, offsetof(struct ShipProbe, fuel), 0 },
    { NULL, 0, 0, NULL, NULL, 0, 0 }
};
static const struct JSHostClass cls = {
    (CONST_STRPTR)"OoliteResetShip", NULL, slots, shipReleased, NULL
};
int main(void)
{
    struct OOGFOSJS js = {0};
    struct ShipProbe ship = {30,12};
    LONG first=-1, second=-1, rc1=-1, rc2=-1;
    int exposed=0, reset=0, rebound=0, ok;
    if (!OOGFOSJSOpen(&js)) {
        puts("OOLITE_JS_HOST_RESET status=UNAVAILABLE");
        return 77;
    }
    exposed = OOGFOSJSExposeHostClass(&js,&cls,&ship,(CONST_STRPTR)"ship");
    if (exposed) rc1=OOGFOSJSEvaluateLong(&js,"ship.cargo+ship.fuel","host-before.js",&first);
    if (exposed && rc1==JSERR_OK && first==42) reset=OOGFOSJSReset(&js);
    if (reset && releaseCount==1)
        rebound=OOGFOSJSExposeHostClass(&js,&cls,&ship,(CONST_STRPTR)"ship");
    if (rebound) rc2=OOGFOSJSEvaluateLong(&js,"ship.cargo+ship.fuel","host-after.js",&second);
    ok=exposed && reset && rebound && rc1==JSERR_OK && rc2==JSERR_OK &&
       first==42 && second==42 && releaseCount==1;
    OOGFOSJSClose(&js);
    ok=ok && releaseCount==2;
    printf("OOLITE_JS_HOST_RESET before=%ld reset=%d rebound=%d after=%ld releases=%ld status=%s\n",
           (long)first,reset,rebound,(long)second,(long)releaseCount,ok?"PASS":"FAIL");
    return ok?0:2;
}
