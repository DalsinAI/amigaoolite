/* GFOS JSHostClass inheritance: original Oolite ship class hierarchy seam.
 * Native AC090 68040 qualification, GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stddef.h>
#include <stdio.h>
struct ShipProbe { LONG energy; LONG cargo; };
static const struct JSHostSlot entitySlots[]={
    { (CONST_STRPTR)"energy",JSS_FIELD,JSF_LONG,NULL,NULL,offsetof(struct ShipProbe,energy),0 },
    { NULL,0,0,NULL,NULL,0,0 }
};
static const struct JSHostClass entityClass={
    (CONST_STRPTR)"OoliteEntityProbe",NULL,entitySlots,NULL,NULL
};
static const struct JSHostSlot shipSlots[]={
    { (CONST_STRPTR)"cargo",JSS_FIELD,JSF_LONG,NULL,NULL,offsetof(struct ShipProbe,cargo),0 },
    { NULL,0,0,NULL,NULL,0,0 }
};
static const struct JSHostClass shipClass={
    (CONST_STRPTR)"OoliteShipProbe",&entityClass,shipSlots,NULL,NULL
};
int main(void)
{
    struct OOGFOSJS js={0};
    struct ShipProbe ship={32,10};
    LONG value=-1,rc=JSERR_BADARGS;
    int wrapped,ok;
    if(!OOGFOSJSOpen(&js)) { puts("OOLITE_JS_INHERIT status=UNAVAILABLE");return 77; }
    wrapped=OOGFOSJSExposeHostClass(&js,&shipClass,&ship,(CONST_STRPTR)"ship");
    if(wrapped) rc=OOGFOSJSEvaluateLong(&js,
        "ship.energy+ship.cargo",
        "oolite-host-inherit.js",&value);
    ok=wrapped && rc==JSERR_OK && value==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_JS_INHERIT wrapped=%d rc=%ld value=%ld status=%s\n",
           wrapped,(long)rc,(long)value,ok?"PASS":"FAIL");
    return ok?0:2;
}
