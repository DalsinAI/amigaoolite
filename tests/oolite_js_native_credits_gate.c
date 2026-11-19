/* Oolite native-credit formatting delegates only optional scripted
 * presentation to GFOpenJavaScript, never the credit arithmetic itself.
 * This exercises no-script fallback, real JS callback, exceptions,
 * Unicode, precision and recovery. GPL-2.0-or-later.
 */
#include "OOGFOSCredits.h"
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <string.h>
struct Case { const char *source; const char *expected; LONG rc; };
int main(void)
{
    struct OOGFOSJS js={0};
    char buffer[512];
    LONG before,noMethod,one,twob,thrown,recovered,after;
    int ok;
    before=OOGFOSCreditsFormat(205,1,1,buffer,sizeof buffer);
    if(!OOGFOSJSOpen(&js))return 77;
    OOGFOSCreditsBindTask(js.task,js.library);
    noMethod=OOGFOSCreditsFormat(205,1,1,buffer,sizeof buffer);
    JS_Evaluate(js.task,
        "this.formatCredits=function(amount,decimal,symbol){"
        "return (symbol?'₢':'')+amount.toFixed(decimal?1:0);};",
        -1,"oolite-credits-script.js",NULL);
    one=OOGFOSCreditsFormat(205,1,1,buffer,sizeof buffer);
    ok=one==JSERR_OK&&strcmp(buffer,"₢20.5")==0;
    twob=OOGFOSCreditsFormat(205,0,0,buffer,sizeof buffer);
    ok=ok&&twob==JSERR_OK&&strcmp(buffer,"21")==0;
    JS_Evaluate(js.task,
        "this.formatCredits=function(){throw new Error('expected');};",
        -1,"oolite-credits-error.js",NULL);
    thrown=OOGFOSCreditsFormat(205,1,1,buffer,sizeof buffer);
    ok=ok&&thrown==JSERR_EXCEPTION&&buffer[0]==0;
    JS_Evaluate(js.task,
        "this.formatCredits=function(){return 'Recovered';};",
        -1,"oolite-credits-recovery.js",NULL);
    recovered=OOGFOSCreditsFormat(205,1,1,buffer,sizeof buffer);
    ok=ok&&recovered==JSERR_OK&&strcmp(buffer,"Recovered")==0;
    OOGFOSCreditsUnbindTask(js.task);
    OOGFOSJSClose(&js);
    after=OOGFOSCreditsFormat(205,1,1,buffer,sizeof buffer);
    ok=ok&&before==JSERR_NOENGINE&&noMethod!=JSERR_OK&&
       after==JSERR_NOENGINE&&buffer[0]==0;
    printf("OOLITE_NATIVE_CREDITS before=%ld noMethod=%ld formatted=%ld,%ld exception=%ld recovery=%ld after=%ld status=%s\n",
        (long)before,(long)noMethod,(long)one,(long)twob,
        (long)thrown,(long)recovered,(long)after,ok?"PASS":"FAIL");
    return ok?0:2;
}
