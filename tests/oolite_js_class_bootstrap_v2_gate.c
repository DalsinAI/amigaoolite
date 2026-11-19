/* True Oolite JS_InitClass bootstrap before any Ship exists, using the
 * append-only javascript.library v2 JS_ClassPrototype vector.
 * Native AC090 68040 feature qualification; GPL-2.0-or-later.
 */
#include "OOGFOSJSClassBootstrap.h"
#include <proto/javascript.h>
#include <exec/libraries.h>
#include <stdio.h>
#include <stddef.h>
struct TestShip {LONG energy;};
static const struct JSHostSlot slots[]={
 {(CONST_STRPTR)"energy",JSS_FIELD,JSF_LONG,NULL,NULL,
  offsetof(struct TestShip,energy),0},
 {NULL,0,0,NULL,NULL,0,0}
};
static const struct JSHostClass klass={"Ship",NULL,slots,NULL,NULL};
int main(void)
{
    struct OOGFOSJS js={0};
    struct TestShip ship={42};
    LONG answer=-1,rc=JSERR_BADARGS;
    int started=0,bound=0,ok;
    if(!OOGFOSJSOpen(&js)) {
        puts("OOLITE_CLASS_BOOTSTRAP status=UNAVAILABLE");return 77;
    }
    printf("OOLITE_CLASS_BOOTSTRAP library_version=%ld\n",
           (long)js.library->lib_Version);
    /* The critical Oolite ordering: expose 'Ship' BEFORE any ship is born.
     * A v2 library paired with v1 engine must fail cleanly, not call a
     * nonexistent table entry or invent a phantom game-owned ship. */
    started=OOGFOSJSBootstrapClass(&js,&klass,"Ship");
    if(started){
        bound=OOGFOSJSExposeHostClass(&js,&klass,&ship,"ship");
        if(bound)rc=OOGFOSJSEvaluateLong(&js,
          "ship instanceof Ship && "
          "Object.getPrototypeOf(ship)===Ship.prototype && "
          "Ship.prototype.constructor===Ship && "
          "ship.energy===42 && "
          "(function(){try{Ship();return false;}catch(e){return true;}})() "
          "? 42:0",
          "oolite-no-phantom-ship.js",&answer);
    }
    ok=started&&bound&&rc==JSERR_OK&&answer==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_CLASS_BOOTSTRAP class=%d wrapper=%d rc=%ld answer=%ld status=%s\n",
           started,bound,(long)rc,(long)answer,ok?"PASS":
           !started?"REQUIRES_V2_ENGINE":"FAIL");
    return ok?0:!started?77:2;
}
