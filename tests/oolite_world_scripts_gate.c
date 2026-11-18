/* Original upstream Oolite world JS assets; their functions are executed in
 * the existing GFOS JavaScriptEngine, no bundled SpiderMonkey/JS library.
 */
#include "OOGFOSJavaScript.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char *readScript(const char *path, long *n)
{
    FILE *f = fopen(path,"rb"); char *p=NULL;
    *n=0; if (!f) return NULL;
    if (!fseek(f,0,SEEK_END)) *n=ftell(f);
    if (*n>0 && *n<262144 && !fseek(f,0,SEEK_SET)) {
        p=malloc((size_t)*n+1);
        if (p && fread(p,1,(size_t)*n,f)==(size_t)*n) p[*n]=0;
        else { free(p);p=NULL; }
    }
    fclose(f);return p;
}
static int test(const char *name,const char *predicate,const char *setup)
{
    char path[256]; long bytes=0; char *src; struct OOGFOSJS js;
    LONG rc=JSERR_BADARGS, value=-1; int opened=0,ok=0;
    snprintf(path,sizeof path,"SYS:Oolite/Scripts/%s.js",name);
    src=readScript(path,&bytes);
    if (src && OOGFOSJSOpen(&js)) {
        opened=1;
        if (setup) rc=OOGFOSJSEvaluateSource(&js,(CONST_STRPTR)setup,(CONST_STRPTR)"world-setup.js");
        else rc=JSERR_OK;
        if (rc==JSERR_OK) rc=OOGFOSJSEvaluateSource(&js,(CONST_STRPTR)src,(CONST_STRPTR)path);
        if (rc==JSERR_OK) rc=OOGFOSJSEvaluateLong(&js,(CONST_STRPTR)predicate,(CONST_STRPTR)"world-check.js",&value);
        if (rc!=JSERR_OK) {
            char err[180]={0};OOGFOSJSErrorText(&js,(STRPTR)err,sizeof err);
            printf("OOLITE_WORLD_SCRIPT_ERROR name=%s text=%s\n",name,err);
        }
        OOGFOSJSClose(&js);
    }
    ok=src && opened && rc==JSERR_OK && value==1;
    printf("OOLITE_WORLD_SCRIPT name=%s bytes=%ld rc=%ld check=%ld status=%s\n",
           name,bytes,(long)rc,(long)value,ok?"PASS":"FAIL"); fflush(stdout);
    free(src);return ok;
}
int main(void)
{
    int a=test("oolite-tutorial","this.name==='oolite-tutorial' && typeof this.startUp==='function' && this.startUp()===undefined ? 1:0","this.missionVariables={};");
    int b=test("oolite-populator","this.name==='oolite-populator' && typeof this.startUp==='function' ? 1:0",NULL);
    int c=test("oolite-conditions","this.name==='oolite-conditions' && typeof this.allowAwardEquipment==='function' && this.allowAwardEquipment('EQ_NONE',{scriptInfo:{}},'scripted')===true ? 1:0",NULL);
    printf("OOLITE_WORLD_SCRIPT_TOTAL passed=%d total=3 status=%s\n",a+b+c,a&&b&&c?"PASS":"FAIL");
    return a&&b&&c?0:2;
}
