/* Oolite original OOJavaScriptEngine reset contract on canonical GFOS.
 * Prepares a replacement class/global graph before committing; failing
 * preparation must preserve all old script state. GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <stddef.h>

struct Probe {
    LONG energy, fuel, prepares, will, did, releases, failPrepare;
    LONG callbacksInCorrectOrder;
};
static VOID onRelease(JSTask task, APTR self, APTR user)
{
    struct Probe *p=(struct Probe *)user;
    (void)task;(void)self;
    ++p->releases;
}
static const struct JSHostSlot slots[]={
    { (CONST_STRPTR)"energy",JSS_FIELD,JSF_LONG,NULL,NULL,offsetof(struct Probe,energy),0 },
    { (CONST_STRPTR)"fuel",JSS_FIELD,JSF_LONG,NULL,NULL,offsetof(struct Probe,fuel),0 },
    { NULL,0,0,NULL,NULL,0,0 }
};
/* The class metadata remains owned by the game task, not script heap. */
static struct JSHostClass cls;
static int prepare(JSTask task, APTR user)
{
    struct Probe *p=(struct Probe*)user;
    JSClass klass=NULL;
    JSValue wrapped=NULL,global=NULL;
    LONG rc=JSERR_BADARGS;
    ++p->prepares;
    if(p->failPrepare) return 0;
    klass=JS_RegisterClass(task,&cls);
    if(!klass) return 0;
    wrapped=JS_Wrap(task,klass,p);
    if(!wrapped) return 0;
    global=JS_Global(task);
    if(global) rc=JS_SetProp(task,global,(CONST_STRPTR)"ship",wrapped);
    if(global) JS_Free(task,global);
    JS_Free(task,wrapped);
    return rc==JSERR_OK;
}
static void willReset(APTR user)
{
    struct Probe *p=(struct Probe*)user;
    ++p->will;
    if(p->did==0 && p->releases==0) ++p->callbacksInCorrectOrder;
}
static void didReset(APTR user)
{
    struct Probe *p=(struct Probe*)user;
    ++p->did;
    if(p->will==1 && p->releases==1) ++p->callbacksInCorrectOrder;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct Probe p={30,12,0,0,0,0,0,0};
    LONG original=-1, afterFailed=-1, afterSuccess=-1,rc1,rc2,rc3;
    int resetFailed,resetPassed,ok=0;
    cls.js_Name=(CONST_STRPTR)"OoliteResetShip";
    cls.js_Parent=NULL;
    cls.js_Slots=slots;
    cls.js_Release=onRelease;
    cls.js_Data=&p;
    if(!OOGFOSJSOpen(&js)){
        puts("OOLITE_JS_PREPARED_RESET status=UNAVAILABLE");return 77;
    }
    if(!prepare(js.task,&p)) goto done;
    rc1=OOGFOSJSEvaluateLong(&js,
      "var persistent=77; ship.energy+ship.fuel",
      "before-reset.js",&original);
    p.failPrepare=1;
    resetFailed=OOGFOSJSResetPrepared(&js,prepare,willReset,didReset,&p);
    rc2=OOGFOSJSEvaluateLong(&js,
      "persistent===77 ? ship.energy+ship.fuel : 0",
      "after-abort.js",&afterFailed);
    p.failPrepare=0;
    resetPassed=OOGFOSJSResetPrepared(&js,prepare,willReset,didReset,&p);
    rc3=OOGFOSJSEvaluateLong(&js,
      "typeof persistent==='undefined' ? ship.energy+ship.fuel : 0",
      "after-commit.js",&afterSuccess);
    ok=rc1==JSERR_OK && rc2==JSERR_OK && rc3==JSERR_OK &&
       original==42 && afterFailed==42 && afterSuccess==42 &&
       !resetFailed && resetPassed && p.prepares==3 &&
       p.will==1 && p.did==1 && p.releases==1 &&
       p.callbacksInCorrectOrder==2;
done:
    OOGFOSJSClose(&js);
    ok=ok && p.releases==2;
    printf("OOLITE_JS_PREPARED_RESET orig=%ld abort=%ld commit=%ld prepares=%ld will=%ld did=%ld releases=%ld order=%ld status=%s\n",
       (long)original,(long)afterFailed,(long)afterSuccess,
       (long)p.prepares,(long)p.will,(long)p.did,(long)p.releases,
       (long)p.callbacksInCorrectOrder,ok?"PASS":"FAIL");
    return ok?0:2;
}
