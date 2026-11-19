/* Original Oolite OOJSWorldScripts enumerated, immutable script registry.
 * Links the same per-script JS objects exposed by OOGFOSScriptObjects,
 * and supports script names containing dashes/dots. GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSScriptObjects.h"
#include "OOGFOSWorldScripts.h"
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
 "OoliteWorldScriptShip",NULL,shipSlots,NULL,NULL
};
int main(void)
{
    struct OOGFOSJS js={0};
    struct Ship ship={42};
    LONG init=-1,first=-1,second=-1,registerA=-1,registerB=-1,
         duplicate=-1,unsafe=-1,check=-1,result=-1;
    int bound=0,ok=0;
    if(!OOGFOSJSOpen(&js))return 77;
    bound=OOGFOSJSExposeHostClass(&js,&shipClass,&ship,"ship");
    if(bound)init=OOGFOSWorldScriptsInit(js.task);
    if(init==JSERR_OK)first=OOGFOSScriptObjectCreate(js.task,"scriptA","ship");
    if(first==JSERR_OK)second=OOGFOSScriptObjectCreate(js.task,"scriptB","ship");
    if(second==JSERR_OK)registerA=OOGFOSWorldScriptsRegister(js.task,
                           "scriptA","oolite-world-a");
    if(registerA==JSERR_OK)registerB=OOGFOSWorldScriptsRegister(js.task,
                           "scriptB","oolite.oxp-b");
    if(registerB==JSERR_OK)duplicate=OOGFOSWorldScriptsRegister(js.task,
                           "scriptB","oolite-world-a");
    unsafe=OOGFOSWorldScriptsRegister(js.task,"scriptA","evil');alert(1);//");
    if(registerB==JSERR_OK)check=OOGFOSJSEvaluateLong(&js,
        "worldScripts['oolite-world-a']===scriptA && "
        "worldScripts['oolite.oxp-b']===scriptB && "
        "worldScripts['oolite-world-a'].ship.bounty===42 && "
        "Object.keys(worldScripts).length===2 && "
        "Object.getOwnPropertyDescriptor(worldScripts,'oolite-world-a').writable===false && "
        "Object.getOwnPropertyDescriptor(worldScripts,'oolite-world-a').configurable===false && "
        "Object.getOwnPropertyDescriptor(this,'worldScripts').writable===false && "
        "Object.getOwnPropertyDescriptor(this,'worldScripts').configurable===false "
        "? 42:0","oolite-worldscripts-test.js",&result);
    ok=bound&&init==JSERR_OK&&first==JSERR_OK&&second==JSERR_OK&&
       registerA==JSERR_OK&&registerB==JSERR_OK&&
       duplicate==JSERR_TYPE&&unsafe==JSERR_BADARGS&&
       check==JSERR_OK&&result==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_WORLD_SCRIPTS init=%ld registered=%ld,%ld duplicate=%ld unsafe=%ld check=%ld result=%ld status=%s\n",
       (long)init,(long)registerA,(long)registerB,(long)duplicate,
       (long)unsafe,(long)check,(long)result,ok?"PASS":"FAIL");
    return ok?0:2;
}
