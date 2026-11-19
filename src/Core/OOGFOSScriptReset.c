/* Original Oolite engine context reset with pumped replacement bootstrap.
 * GPL-2.0-or-later.
 */
#include "OOGFOSScriptReset.h"
#include <proto/javascript.h>
#include <proto/exec.h>
struct ResetCall { OOGFOSScriptResetPrepare callback; APTR user; };
static LONG callPrepare(JSTask task,APTR data,LONG *result)
{
    struct ResetCall *p=(struct ResetCall *)data;
    if(result)*result=0;
    return p->callback(task,p->user);
}
LONG OOGFOSScriptResetRouted(struct OOGFOSJS *js,
                            struct OOGFOSGameDispatch *dispatch,
                            OOGFOSScriptResetPrepare prepare,
                            OOGFOSJSResetNotice willReset,
                            OOGFOSJSResetNotice didReset,
                            APTR user)
{
    struct OOGFOSJS candidate;
    struct ResetCall setup;
    JSTask old;
    LONG rc,ignored=0;
    if(!js || !js->library || !js->task || !dispatch || !prepare ||
       FindTask(NULL)!=dispatch->gameTask || dispatch->sealed)
        return JSERR_BADARGS;
    JavaScriptBase=js->library;
    candidate.library=js->library;
    candidate.task=JS_NewTask(NULL);
    if(!candidate.task)return JSERR_NOMEMORY;
    setup.callback=prepare;
    setup.user=user;
    /* A potentially complex original Oolite class-registration sequence
     * can issue game-owner RPCs even before reset finishes. */
    rc=OOGFOSScriptInvoke(&candidate,dispatch,callPrepare,&setup,&ignored);
    if(rc!=JSERR_OK){
        JS_EndTask(candidate.task);
        return rc;
    }
    old=js->task;
    if(willReset)willReset(user);
    /* Old task is still live while prepare happens; no old handles cross
     * into the new task. Old wrapper finalizers run during EndTask. */
    JS_EndTask(old);
    js->task=candidate.task;
    if(didReset)didReset(user);
    return JSERR_OK;
}
