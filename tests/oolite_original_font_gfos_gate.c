/* Original defaultFont.measureString port: JavaScriptCore --> bounded
 * game-task Exec RPC --> real Oolite font callback interface.
 * Controlled width fixture verifies contract; full HUD binding is separate.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSGameDispatch.h"
#include "OOJSFontGFOS.h"
#include <proto/javascript.h>
#include <proto/exec.h>
#include <stdio.h>
#include <string.h>
struct Game {
    struct Task *owner;
    LONG wrongTask;
    LONG widthCalls;
    char last[256];
};
static int readDummy(void *unused,unsigned key,LONG *out)
{(void)unused;(void)key;(void)out;return 0;}
static int writeDummy(void *unused,unsigned key,LONG value)
{(void)unused;(void)key;(void)value;return 0;}
static int measureFixture(void *owner,const char *utf8,float *out)
{
    struct Game *g=(struct Game *)owner;
    if(FindTask(NULL)!=g->owner){g->wrongTask++;return 0;}
    if(strlen(utf8)>=sizeof(g->last))return 0;
    strcpy(g->last,utf8);
    g->widthCalls++;
    *out=0.5f*(float)strlen(utf8);
    return 1;
}
/* Only the fixture links this substitute: production uses the exact
 * original OOStringWidthInEm callback from OOJSFontGame.m. */
int OOGFOSJSFontMeasureOnGameTask(void *o,const char *t,float *w)
{return measureFixture(o,t,w);}
struct Test {
    struct OOGFOSGameDispatch *dispatch;
    LONG init,eval,converted,returnValue,done;
    LONG readonly;
};
static LONG scriptJob(JSTask task,APTR arg)
{
    struct Test *test=(struct Test *)arg;
    JSValue result=NULL;
    LONG out=-1,rc;
    const char source[]=
      "(function(){"
      "var a=defaultFont.measureString('Elite');"
      "var b=defaultFont.measureString('X');"
      "var readonly=Object.isFrozen(defaultFont)"
      "&&!Object.getOwnPropertyDescriptor(defaultFont,'measureString').writable;"
      "if(!readonly)throw Error('font not read-only');"
      "if(a!==2.5||b!==0.5)throw Error('font width mismatch');"
      "return 42;"
      "})()";
    test->init=OOGFOSJSFontInstall(task,test->dispatch);
    test->eval=test->init;
    if(test->init==JSERR_OK){
        test->eval=JS_Evaluate(task,(CONST_STRPTR)source,-1,
                             (CONST_STRPTR)"oolite-original-font.js",&result);
        if(test->eval==JSERR_OK&&result){
            test->converted=JS_ToLong(task,result,&out);
            test->returnValue=out;
        }
    }
    if(result)JS_Free(task,result);
    rc=test->init!=JSERR_OK?test->init:test->eval;
    if(rc==JSERR_OK&&test->converted!=JSERR_OK)rc=test->converted;
    test->done=OOGFOSGameDispatchDone(test->dispatch,rc,out);
    return rc;
}
static LONG scriptJoin(JSTask task,APTR arg)
{(void)task;(void)arg;return JSERR_OK;}
int main(void)
{
    const struct OOGFOSShipBackend ownerBackend={readDummy,writeDummy};
    struct Game game={0};
    struct OOGFOSGameDispatch dispatch;
    struct OOGFOSJS js={0};
    struct Test test={0};
    LONG post=-1,join=-1;
    int event,ok=0,closed=0;
    game.owner=FindTask(NULL);
    if(!OOGFOSGameDispatchInit(&dispatch,&game,&ownerBackend))return 75;
    if(!OOGFOSGameDispatchSetMeasureText(&dispatch,measureFixture))return 74;
    if(!OOGFOSJSOpen(&js)){OOGFOSGameDispatchClose(&dispatch);return 77;}
    test.dispatch=&dispatch;
    post=JS_Post(js.task,scriptJob,&test,0);
    if(post==JSERR_OK) {
        do {event=OOGFOSGameDispatchServe(&dispatch);}while(event==0);
        if(event==1)join=JS_Post(js.task,scriptJoin,NULL,1);
    }
    ok=post==JSERR_OK&&join==JSERR_OK&&
       test.init==JSERR_OK&&test.eval==JSERR_OK&&
       test.converted==JSERR_OK&&test.returnValue==42&&
       test.done==1&&game.wrongTask==0&&game.widthCalls==2&&
       strcmp(game.last,"X")==0&&dispatch.requests==2&&
       dispatch.completions==1&&dispatch.resultCode==JSERR_OK&&
       dispatch.result==42;
    OOGFOSJSClose(&js);
    closed=OOGFOSGameDispatchTryClose(&dispatch);
    ok=ok&&closed;
    printf("OOLITE_ORIGINAL_FONT_GFOSTRANSPORT post=%ld init=%ld eval=%ld result=%ld "
           "measure_calls=%ld wrong_task=%ld messages=%lu closed=%d status=%s\n",
           (long)post,(long)test.init,(long)test.eval,(long)test.returnValue,
           (long)game.widthCalls,(long)game.wrongTask,
           (unsigned long)dispatch.requests,closed,ok?"PASS":"FAIL");
    return ok?0:2;
}
