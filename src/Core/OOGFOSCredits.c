/* Oolite's original OOStringFromDeciCredits uses the optional JS global
 * formatCredits, not a JavaScript implementation of native game finances.
 * This adapter preserves that contract through javascript.library.
 *
 * No SpiderMonkey JS_GetPendingException or JS_SetPendingException:
 * each JSC execution has its own exception result and diagnostic.
 * GPL-2.0-or-later.
 */
#include "OOGFOSCredits.h"
#include <proto/javascript.h>
#include <string.h>

struct CreditsTask {
    JSTask task;
    struct Library *library;
};
static struct CreditsTask sBinding;

void OOGFOSCreditsBindTask(JSTask task, struct Library *library)
{
    /* Caller must ensure task lifecycle is quiescent when replacing binding. */
    sBinding.task=task;
    sBinding.library=task?library:NULL;
}
void OOGFOSCreditsUnbindTask(JSTask task)
{
    if(task && sBinding.task==task) {
        sBinding.task=NULL;
        sBinding.library=NULL;
    }
}
struct CreditsCall {
    unsigned long long tenths;
    int decimal, symbol;
    char *output;
    LONG size;
    LONG status;
};
static LONG runCredits(JSTask task, APTR opaque)
{
    struct CreditsCall *c=(struct CreditsCall *)opaque;
    JSValue global=NULL, method=NULL, number=NULL, decimals=NULL;
    JSValue symbols=NULL, returned=NULL, argv[3];
    DOUBLE credits;
    LONG rc=JSERR_TYPE, used;
    if(!c||!c->output||c->size<2)return JSERR_BADARGS;
    c->output[0]=0;
    global=JS_Global(task);
    if(!global)goto done;
    method=JS_GetProp(task,global,(CONST_STRPTR)"formatCredits");
    if(!method||JS_TypeOf(task,method)!=JST_FUNCTION)goto done;
    credits=(DOUBLE)c->tenths/10.0;
    number=JS_NewDouble(task,&credits);
    decimals=JS_NewBool(task,c->decimal?1:0);
    symbols=JS_NewBool(task,c->symbol?1:0);
    if(!number||!decimals||!symbols) {rc=JSERR_NOMEMORY;goto done;}
    argv[0]=number;argv[1]=decimals;argv[2]=symbols;
    rc=JS_Call(task,method,global,3,argv,&returned);
    if(rc!=JSERR_OK||!returned)goto done;
    /* Historical game code coerces any defined JS value to NSString.
     * JS_ToString performs equivalent normal ECMAScript conversion.
     * Return a length >= size without truncating a displayed credit total. */
    used=JS_ToString(task,returned,(STRPTR)c->output,c->size);
    if(used<0 || used>=c->size) {
        c->output[0]=0;
        rc=JSERR_BADARGS;
    }
done:
    if(returned)JS_Free(task,returned);
    if(symbols)JS_Free(task,symbols);
    if(decimals)JS_Free(task,decimals);
    if(number)JS_Free(task,number);
    if(method)JS_Free(task,method);
    if(global)JS_Free(task,global);
    c->status=rc;
    return rc;
}
LONG OOGFOSCreditsFormat(unsigned long long tenths,int decimal,int symbol,
                        char *utf8,LONG capacity)
{
    struct CreditsCall call;
    LONG rc;
    if(!utf8||capacity<2)return JSERR_BADARGS;
    utf8[0]=0;
    if(!sBinding.task||!sBinding.library)return JSERR_NOENGINE;
    call.tenths=tenths;
    call.decimal=decimal;
    call.symbol=symbol;
    call.output=utf8;
    call.size=capacity;
    call.status=JSERR_BADARGS;
    JavaScriptBase=sBinding.library;
    rc=JS_Post(sBinding.task,runCredits,&call,1);
    return rc==JSERR_OK?call.status:rc;
}
