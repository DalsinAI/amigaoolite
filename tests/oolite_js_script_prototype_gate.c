/* Original OOJSScript per-instance prototype inheritance on native
 * GFOS JSTask. Exercises the original global-prefix pattern in which
 * Script.prototype methods are added after class registration.
 * GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSScriptObjects.h"
#include <proto/javascript.h>
#include <stddef.h>
#include <stdio.h>
struct Ship {LONG bounty;};
static const struct JSHostSlot shipSlots[]={
    {(CONST_STRPTR)"bounty",JSS_FIELD,JSF_LONG,NULL,NULL,
      offsetof(struct Ship,bounty),0},
    {NULL,0,0,NULL,NULL,0,0}
};
static const struct JSHostClass shipClass={
    (CONST_STRPTR)"OoliteScriptProtoShip",NULL,shipSlots,NULL,NULL
};
int main(void)
{
    struct OOGFOSJS js={0};
    struct Ship ship={40};
    LONG early=JSERR_BADARGS,registerRC=JSERR_BADARGS,
         late=JSERR_BADARGS,check=JSERR_BADARGS,result=-1;
    int bound=0,ok;
    if(!OOGFOSJSOpen(&js))return 77;
    bound=OOGFOSJSExposeHostClass(&js,&shipClass,&ship,"ship");
    if(bound) early=OOGFOSScriptObjectCreate(js.task,"earlyScript","ship");
    if(early==JSERR_OK) registerRC=OOGFOSJSEvaluateSource(&js,
      "this.Script=function Script(){};"
      "Script.prototype.escortValue=function(){return this.ship.bounty+2;};",
      "oolite-script-prototype-setup.js");
    if(registerRC==JSERR_OK)
        late=OOGFOSScriptObjectCreate(js.task,"script","ship");
    if(late==JSERR_OK)
        check=OOGFOSJSEvaluateLong(&js,
          "Object.getPrototypeOf(earlyScript)===Object.prototype && "
          "Object.getPrototypeOf(script)===Script.prototype && "
          "script instanceof Script && !(earlyScript instanceof Script) && "
          "script.escortValue()===42 && "
          "Object.prototype.hasOwnProperty.call(script,'ship') && "
          "!Object.prototype.hasOwnProperty.call(script,'escortValue') "
          "? 42 : 0",
          "oolite-script-prototype-check.js",&result);
    ok=bound&&early==JSERR_OK&&registerRC==JSERR_OK&&
       late==JSERR_OK&&check==JSERR_OK&&result==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_SCRIPT_PROTOTYPE early=%ld class=%ld late=%ld check=%ld result=%ld status=%s\n",
           (long)early,(long)registerRC,(long)late,(long)check,
           (long)result,ok?"PASS":"FAIL");
    return ok?0:2;
}
