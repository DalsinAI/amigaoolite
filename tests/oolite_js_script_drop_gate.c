/* Oolite OOJSScript deallocation/recreation: dropping the global
 * script registry lookup must delete it, not leave an enumerable
 * undefined property; another retained JS reference is still usable.
 * Native AC090 68040, GPL-2.0-or-later.
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
 "OoliteScriptReleaseShip",NULL,shipSlots,NULL,NULL
};
int main(void)
{
    struct OOGFOSJS js={0};
    struct Ship ship={42};
    LONG first=-1,save=-1,dropped=-1,verify=-1,
         renewed=-1,check=-1,attack=-1,answer=-1;
    int bound=0,ok;
    if(!OOGFOSJSOpen(&js))return 77;
    bound=OOGFOSJSExposeHostClass(&js,&shipClass,&ship,"ship");
    if(bound)first=OOGFOSScriptObjectCreate(js.task,"script","ship");
    if(first==JSERR_OK)
        save=OOGFOSJSEvaluateSource(&js,
            "this.saved=script;saved.test=42;",
            "oolite-script-retain-test.js");
    if(save==JSERR_OK)
        dropped=OOGFOSScriptObjectDrop(js.task,"script");
    if(dropped==JSERR_OK)
        verify=OOGFOSJSEvaluateLong(&js,
            "typeof this.script==='undefined' && "
            "!Object.prototype.hasOwnProperty.call(this,'script') && "
            "saved.test===42 && saved.ship.bounty===42 ? 42:0",
            "oolite-script-after-drop.js",&answer);
    if(verify==JSERR_OK && answer==42)
        renewed=OOGFOSScriptObjectCreate(js.task,"script","ship");
    if(renewed==JSERR_OK)
        check=OOGFOSJSEvaluateLong(&js,
            "this.script!==saved && script.ship===saved.ship && "
            "saved.test===42 && typeof script.test==='undefined'"
            " ? 42:0",
            "oolite-script-after-renew.js",&answer);
    attack=OOGFOSScriptObjectDrop(js.task,"script;evil()");
    ok=bound && first==JSERR_OK && save==JSERR_OK &&
       dropped==JSERR_OK && verify==JSERR_OK &&
       renewed==JSERR_OK && check==JSERR_OK &&
       attack==JSERR_BADARGS && answer==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_SCRIPT_DROP first=%ld save=%ld drop=%ld verify=%ld renewed=%ld check=%ld attack=%ld result=%ld status=%s\n",
        (long)first,(long)save,(long)dropped,(long)verify,
        (long)renewed,(long)check,(long)attack,(long)answer,
        ok?"PASS":"FAIL");
    return ok?0:2;
}
