/* Test GFOS host wrapper finalization on script-task teardown.
 * Native Oolite object is caller-owned: release callback must not free it.
 */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stddef.h>
#include <stdio.h>
struct Ship { LONG hull; LONG releases; };
static void shipReleased(JSTask task, APTR self, APTR data)
{
    struct Ship *ship=(struct Ship *)self;
    (void)task; (void)data;
    if (ship) ship->releases++;
}
static const struct JSHostSlot slots[] = {
    { (CONST_STRPTR)"hull", JSS_FIELD, JSF_LONG, NULL, NULL, offsetof(struct Ship, hull), 0 },
    { NULL,0,0,NULL,NULL,0,0 }
};
static const struct JSHostClass shipClass = {
    (CONST_STRPTR)"OoliteReleaseShip", NULL, slots, shipReleased, NULL
};
int main(void)
{
    struct OOGFOSJS js;
    struct Ship ship={101,0};
    LONG result=-1,rc=JSERR_BADARGS;
    int exposed=0,ok=0;
    if (!OOGFOSJSOpen(&js)) { puts("OOLITE_JS_RELEASE status=UNAVAILABLE"); return 77; }
    exposed=OOGFOSJSExposeHostClass(&js,&shipClass,&ship,(CONST_STRPTR)"ship");
    if (exposed) rc=OOGFOSJSEvaluateLong(&js,
        (CONST_STRPTR)"ship.hull===101 ? 1 : 0",
        (CONST_STRPTR)"oolite-release.js",&result);
    printf("OOLITE_JS_RELEASE before_end=%ld exposed=%d rc=%ld result=%ld\n",
           (long)ship.releases,exposed,(long)rc,(long)result);
    fflush(stdout);
    OOGFOSJSClose(&js);
    ok = exposed && rc == JSERR_OK && result==1 && ship.releases==1;
    printf("OOLITE_JS_RELEASE after_end=%ld expected=1 status=%s\n",
           (long)ship.releases,ok?"PASS":"FAIL");
    fflush(stdout);
    return ok?0:2;
}
