/* Unmodified ORIGINAL 178KB Oolite PriorityAI world script, native
 * GFOS javascript.library. Exercise real communication setup/fallback,
 * startup one-shot removal and witchspace message gating. GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSScriptObjects.h"
#include "OOGFOSWorldScripts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char *readOriginal(const char *path,LONG *size)
{
 FILE *f=fopen(path,"rb");char *buf=NULL;long n=-1;
 if(!f)return NULL;
 if(fseek(f,0,SEEK_END)==0)n=ftell(f);
 if(n>0&&n<262144&&fseek(f,0,SEEK_SET)==0){
  buf=(char*)malloc((size_t)n+1);
  if(!buf||fread(buf,1,(size_t)n,f)!=(size_t)n){free(buf);buf=NULL;}
  else {buf[n]=0;*size=(LONG)n;}
 }
 fclose(f);return buf;
}
int main(void)
{
 struct OOGFOSJS js={0};
 char error[512]={0},*source;
 LONG bytes=0,setup=-1,created=-1,world=-1,load=-1,
      startup=-1,check=-1,final=-1;
 int ok=0;
 source=readOriginal("SYS:Oolite/Scripts/oolite-priorityai.js",&bytes);
 if(!source){puts("OOLITE_PRIORITY_AI asset=UNAVAILABLE");return 77;}
 if(!OOGFOSJSOpen(&js)){free(source);return 77;}
 setup=OOGFOSJSEvaluateSource(&js,
       "this.scriptHost={AIScript:{},scannerRange:128};",
       "oolite-priorityai-mock-ship.js");
 if(setup==JSERR_OK)world=OOGFOSWorldScriptsInit(js.task);
 if(world==JSERR_OK)created=OOGFOSScriptObjectCreate(js.task,
                          "priorityScript","scriptHost");
 if(created==JSERR_OK)world=OOGFOSWorldScriptsRegister(js.task,
                           "priorityScript","oolite-libPriorityAI");
 if(world==JSERR_OK)load=OOGFOSScriptObjectLoad(js.task,"priorityScript",
                           source,bytes,"oolite-priorityai.js");
 if(load!=JSERR_OK)OOGFOSJSErrorText(&js,error,sizeof(error));
 if(load==JSERR_OK)startup=OOGFOSJSEvaluateSource(&js,
                           "priorityScript.startUp();",
                           "oolite-priorityai-startup.js");
 if(startup!=JSERR_OK && load==JSERR_OK)
      OOGFOSJSErrorText(&js,error,sizeof(error));
 if(startup==JSERR_OK)check=OOGFOSJSEvaluateLong(&js,
    "typeof priorityScript.PriorityAIController==='function' && "
    "priorityScript.name==='oolite-libPriorityAI' && "
    "typeof priorityScript.startUp==='undefined' && "
    "priorityScript.$commsAllowed===true && "
    "priorityScript._getCommunication('police','generic','oolite_markForFines')==='[oolite-comms-markForFines]' && "
    "priorityScript._getCommunication('trader','generic','oolite_thanksForHelp')==='[oolite-comms-thanksForHelp]' && "
    "worldScripts['oolite-libPriorityAI']===priorityScript "
    "? 42:0",
    "oolite-priorityai-check.js",&final);
 if(check==JSERR_OK) {
    check=OOGFOSJSEvaluateLong(&js,
      "priorityScript.shipWillEnterWitchspace();"
      "var blocked=priorityScript.$commsAllowed===false;"
      "priorityScript.shipExitedWitchspace();"
      "blocked && priorityScript.$commsAllowed===true ? 42:0",
      "oolite-priorityai-comms-gating.js",&final);
 }
 ok=setup==JSERR_OK&&created==JSERR_OK&&world==JSERR_OK&&
    load==JSERR_OK&&startup==JSERR_OK&&check==JSERR_OK&&final==42;
 OOGFOSJSClose(&js);
 printf("OOLITE_PRIORITY_AI bytes=%ld setup=%ld world=%ld load=%ld startup=%ld check=%ld result=%ld status=%s\n",
  (long)bytes,(long)setup,(long)world,(long)load,(long)startup,
  (long)check,(long)final,ok?"PASS":"FAIL");
 if(!ok && error[0])printf("OOLITE_PRIORITY_AI_ERROR %s\n",error);
 free(source);return ok?0:2;
}
