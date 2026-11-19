/* Multi-OXP task ownership and cleanup using current canonical
 * javascript.library ABI, without assuming stop/cancel v3 is qualified.
 * A task stays alive until its Oolite owner explicitly JS_EndTask's it.
 * All JSValues are task-private. GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <string.h>

enum { OWNED_TASKS=12 };
int main(void)
{
    struct OOGFOSJS js[OWNED_TASKS];
    int i,created=0,ok=1;
    LONG rc=0,number=0;
    char src[140];
    memset(js,0,sizeof js);
    setvbuf(stdout,NULL,_IONBF,0);
    puts("OOLITE_OWNED_TASKS_START");
    for(i=0;i<OWNED_TASKS;i++) {
        if(!OOGFOSJSOpen(&js[i])) {
            printf("OOLITE_OWNED_TASKS create_failure_at=%d\n",i);
            ok=0;break;
        }
        created++;
        snprintf(src,sizeof src,
          "this.oxpIdentity=%d; this.oxpState=%d; 6*7",i,i*13);
        rc=OOGFOSJSEvaluateLong(&js[i],(CONST_STRPTR)src,
                (CONST_STRPTR)"oolite-oxp-create.js",&number);
        if(rc!=JSERR_OK||number!=42){ok=0;break;}
    }
    if(created==OWNED_TASKS && ok) {
        for(i=0;i<OWNED_TASKS;i++) {
            snprintf(src,sizeof src,
                "oxpIdentity===%d && oxpState===%d ? 42 : 0",i,i*13);
            number=-1;
            rc=OOGFOSJSEvaluateLong(&js[i],(CONST_STRPTR)src,
                  (CONST_STRPTR)"oolite-oxp-isolation.js",&number);
            if(rc!=JSERR_OK||number!=42){ok=0;break;}
        }
    }
    /* Oolite unloads one script task; no other task is affected.
     * A subsequent script gets a fresh task without inheriting globals. */
    if(created==OWNED_TASKS && ok) {
        OOGFOSJSClose(&js[4]);
        if(!OOGFOSJSOpen(&js[4]))ok=0;
        else {
            number=-1;
            rc=OOGFOSJSEvaluateLong(&js[4],
                (CONST_STRPTR)"typeof oxpIdentity==='undefined'?42:0",
                (CONST_STRPTR)"oolite-oxp-replacement.js",&number);
            if(rc!=JSERR_OK||number!=42)ok=0;
        }
        number=-1;
        rc=OOGFOSJSEvaluateLong(&js[7],
            (CONST_STRPTR)"oxpIdentity===7?42:0",
            (CONST_STRPTR)"oolite-unaffected-oxp.js",&number);
        if(rc!=JSERR_OK||number!=42)ok=0;
    }
    /* Important: orderly cleanup of **every** owner, not shared-core exit. */
    for(i=created-1;i>=0;i--)
        if(js[i].task)OOGFOSJSClose(&js[i]);
    printf("OOLITE_OWNED_TASKS created=%d expected=%d lastCode=%ld cleaned=%d status=%s\n",
           created,OWNED_TASKS,(long)rc,created,ok?"PASS":"FAIL");
    return ok?0:2;
}
