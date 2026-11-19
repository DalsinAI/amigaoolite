/* Full unchanged Oolite Constrictor library mission conditions script:
 * validate actual game event return values and mission variable changes
 * through canonical GFOS WorldScripts. Native AC090, GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSScriptObjects.h"
#include "OOGFOSWorldScripts.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char *readSource(const char *path,LONG *size)
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
static LONG missionAllows(JSTask task,CONST_STRPTR role,int *out)
{
 JSValue global=NULL,script=NULL,fn=NULL,arg=NULL,result=NULL;
 LONG rc=JSERR_BADARGS;
 *out=-1;
 global=JS_Global(task);
 if(!global)goto done;
 script=JS_GetProp(task,global,"constrictorScript");
 if(!script)goto done;
 fn=JS_GetProp(task,script,"allowShowLibraryShip");
 arg=JS_NewString(task,role,-1);
 if(!fn||!arg||JS_TypeOf(task,fn)!=JST_FUNCTION)goto done;
 rc=JS_Call(task,fn,script,1,&arg,&result);
 if(rc==JSERR_OK)*out=JS_ToBool(task,result)!=0;
done:
 if(result)JS_Free(task,result);
 if(arg)JS_Free(task,arg);
 if(fn)JS_Free(task,fn);
 if(script)JS_Free(task,script);
 if(global)JS_Free(task,global);
 return rc;
}
int main(void)
{
 struct OOGFOSJS js={0};
 LONG bytes=0,setup=-1,create=-1,world=-1,load=-1,empty=-1,set=-1,reset=-1;
 int other=-1,before=-1,after=-1,cleared=-1,ok=0;
 char *source=readSource("SYS:Oolite/Scripts/oolite-constrictor-library.js",&bytes);
 if(!source){puts("OOLITE_CONSTRICTOR asset=UNAVAILABLE");return 77;}
 if(!OOGFOSJSOpen(&js)){free(source);return 77;}
 puts("OOLITE_CONSTRICTOR_STAGE js_open");
 setup=OOGFOSJSEvaluateSource(&js,"this.missionVariables={};","mission-variables.js");
 if(setup==JSERR_OK)create=OOGFOSScriptObjectCreate(js.task,"constrictorScript","missionVariables");
 /* For this test the documented shipGlobal must reference a JS object;
  * missionVariables is the canonical game-owned mock dictionary. */
 if(create==JSERR_OK)world=OOGFOSWorldScriptsRegister(js.task,
                "constrictorScript","oolite-constrictor-library");
 if(world==JSERR_OK)load=OOGFOSScriptObjectLoad(js.task,
               "constrictorScript",source,bytes,"oolite-constrictor-library.js");
 printf("OOLITE_CONSTRICTOR_STAGE load=%ld\\n",(long)load);
 if(load==JSERR_OK){
  puts("OOLITE_CONSTRICTOR_STAGE other_begin");
  empty=missionAllows(js.task,"anaconda",&other);
  printf("OOLITE_CONSTRICTOR_STAGE other_end=%ld\\n",(long)empty);
  if(empty==JSERR_OK)empty=missionAllows(js.task,"constrictor",&before);
  printf("OOLITE_CONSTRICTOR_STAGE before_end=%ld\\n",(long)empty);
  puts("OOLITE_CONSTRICTOR_STAGE set_begin");
  set=OOGFOSJSEvaluateSource(&js,
    "missionVariables.conhunt='HUNTING';","constrictor-save.js");
  if(set==JSERR_OK)set=missionAllows(js.task,"constrictor",&after);
  printf("OOLITE_CONSTRICTOR_STAGE set_end=%ld\\n",(long)set);
  reset=OOGFOSJSEvaluateSource(&js,
    "delete missionVariables.conhunt;","constrictor-reset.js");
  if(reset==JSERR_OK)reset=missionAllows(js.task,"constrictor",&cleared);
 }
 ok=setup==JSERR_OK&&create==JSERR_OK&&world==JSERR_OK&&
    load==JSERR_OK&&empty==JSERR_OK&&set==JSERR_OK&&reset==JSERR_OK&&
    other==1&&before==0&&after==1&&cleared==0;
 OOGFOSJSClose(&js);
 printf("OOLITE_CONSTRICTOR bytes=%ld setup=%ld load=%ld other=%d before=%d after=%d cleared=%d status=%s\n",
   (long)bytes,(long)setup,(long)load,other,before,after,cleared,
   ok?"PASS":"FAIL");
 free(source);return ok?0:2;
}
