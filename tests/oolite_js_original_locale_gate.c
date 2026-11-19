/* Entire unchanged Oolite locale script, through per-script this object and
 * global native-function seam (OOJSGlobal expandDescription fixture).
 * Native AC090 68040 qualification. GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include "OOGFOSScriptObjects.h"
#include "OOGFOSWorldScripts.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
struct Ship {LONG bounty;};
static const struct JSHostSlot slots[]={
 {(CONST_STRPTR)"bounty",JSS_FIELD,JSF_LONG,NULL,NULL,offsetof(struct Ship,bounty),0},
 {NULL,0,0,NULL,NULL,0,0}
};
static const struct JSHostClass shipClass={"OoliteLocaleFixtureShip",NULL,slots,NULL,NULL};
static JSValue expandDescription(JSTask task,APTR self,LONG argc,
                                 JSValue *argv,APTR data)
{
    char key[128]={0};char result[256]={0};
    JSValue value=NULL;
    (void)self;(void)data;
    if(argc<1 || !argv || JS_ToString(task,argv[0],key,sizeof(key))!=JSERR_OK)
        return JS_Throw(task,"Oolite expansion key required");
    if(!strcmp(key,"[number-group-threshold]"))return JS_NewString(task,"4",-1);
    if(!strcmp(key,"[number-group-size]"))return JS_NewString(task,"3",-1);
    if(!strcmp(key,"[number-decimal-separator]"))return JS_NewString(task,".",-1);
    if(!strcmp(key,"[number-group-separator]"))return JS_NewString(task,",",-1);
    if(argc>1) {
        value=JS_GetProp(task,argv[1],"value");
        if(value) {
            JS_ToString(task,value,result,sizeof(result));
            JS_Free(task,value);
        }
    }
    if(!strcmp(key,"[number-negative]"))
        return JS_NewString(task,"-",1); /* test excludes negative-number expansion */
    if(!strcmp(key,"[credits-negative]"))
        return JS_NewString(task,"-",1); /* test excludes negative credits */
    if(!strcmp(key,"[credits-format]"))return JS_NewString(task,result,-1);
    return JS_Throw(task,"Unknown Oolite expansion key");
}
static char *loadSource(const char *path,LONG *size)
{
    FILE *f=fopen(path,"rb");char *buf=NULL;long n=-1;
    if(!f)return NULL;
    if(fseek(f,0,SEEK_END)==0)n=ftell(f);
    if(n>0&&n<262144&&fseek(f,0,SEEK_SET)==0){
        buf=(char *)malloc((size_t)n+1);
        if(buf && fread(buf,1,(size_t)n,f)==(size_t)n){
            buf[n]=0;*size=(LONG)n;
        }else{free(buf);buf=NULL;}
    }
    fclose(f);return buf;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct Ship ship={42};
    LONG bytes=0,setup=-1,load=-1,check=-1,result=-1,
         world=-1,registered=-1;
    int bound=0,expanded=0,ok;
    char error[512]={0};
    char *source=loadSource("SYS:Oolite/Scripts/oolite-locale-functions.js",&bytes);
    if(!source){puts("OOLITE_LOCALE_FUNCTIONS asset=UNAVAILABLE");return 77;}
    if(!OOGFOSJSOpen(&js)){free(source);return 77;}
    bound=OOGFOSJSExposeHostClass(&js,&shipClass,&ship,"ship");
    if(bound)expanded=OOGFOSJSSetGlobalFunction(&js,"expandDescription",expandDescription,NULL);
    if(expanded)setup=OOGFOSJSEvaluateSource(&js,
        "this.global=this;","oolite-locale-setup.js");
    if(setup==JSERR_OK)world=OOGFOSWorldScriptsInit(js.task);
    if(world==JSERR_OK)setup=OOGFOSScriptObjectCreate(js.task,"localeScript","ship");
    if(setup==JSERR_OK)registered=OOGFOSWorldScriptsRegister(js.task,
                       "localeScript","oolite-locale-functions");
    if(registered==JSERR_OK)
       load=OOGFOSScriptObjectLoad(js.task,"localeScript",source,bytes,
                                  "oolite-locale-functions.js");
    if(load!=JSERR_OK)OOGFOSJSErrorText(&js,error,sizeof(error));
    if(load==JSERR_OK)check=OOGFOSJSEvaluateLong(&js,
        "formatInteger(1234567)==='1,234,567' && "
        "formatCredits(123456.7,true,false)==='123,456.7' && "
        "formatCredits(999.9,false,false)==='1,000' && "
        "formatCredits(12.0,false,true)==='12' && "
        "worldScripts['oolite-locale-functions']===localeScript && "
        "localeScript.name==='oolite-locale-functions' && "
        "typeof formatInteger==='function' ? 42:0",
        "oolite-locale-verify.js",&result);
    ok=bound&&expanded&&setup==JSERR_OK&&world==JSERR_OK&&
       registered==JSERR_OK&&load==JSERR_OK&&check==JSERR_OK&&result==42;
    OOGFOSJSClose(&js);
    printf("OOLITE_LOCALE_FUNCTIONS bytes=%ld setup=%ld world=%ld register=%ld load=%ld check=%ld result=%ld status=%s\n",
      (long)bytes,(long)setup,(long)world,(long)registered,
      (long)load,(long)check,(long)result,ok?"PASS":"FAIL");
    if(load!=JSERR_OK)printf("OOLITE_LOCALE_ERROR %s\\n",error);
    free(source);return ok?0:2;
}
