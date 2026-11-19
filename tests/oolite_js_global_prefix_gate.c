/* Original Oolite global-prefix.js on canonical GFOS JSHostClass prototype.
 * Only Ship is a GFOS native wrapper; other globals are test fixtures until
 * the complete original OOJS class graph is ported. GPL-2.0-or-later.
 */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
struct ShipFixture { LONG bounty, spawnCalls; };
static JSValue shipSpawn(JSTask task, APTR self, LONG argc, JSValue *argv, APTR unused)
{
    struct ShipFixture *ship=(struct ShipFixture*)self;
    JSValue array=NULL,number=NULL;
    LONG count=-1;
    (void)unused;
    if(!ship || argc!=2 || !argv || !argv[1] ||
       JS_ToLong(task,argv[1],&count)!=JSERR_OK || count!=1)
        return JS_Throw(task,"Oolite spawn arguments invalid");
    ++ship->spawnCalls;
    array=JS_NewObject(task);
    number=JS_NewLong(task,42);
    if(!array || !number || JS_SetIndex(task,array,0,number)!=JSERR_OK){
        if(number)JS_Free(task,number);
        if(array)JS_Free(task,array);
        return JS_Throw(task,"Oolite spawn allocation error");
    }
    JS_Free(task,number);
    return array;
}
static const struct JSHostSlot shipSlots[]={
    {(CONST_STRPTR)"bounty",JSS_FIELD,JSF_LONG,NULL,NULL,
      offsetof(struct ShipFixture,bounty),0},
    {(CONST_STRPTR)"spawn",JSS_METHOD,0,(APTR)shipSpawn,NULL,0,2},
    {NULL,0,0,NULL,NULL,0,0}
};
static const struct JSHostClass shipClass={
    "OolitePrefixShip",NULL,shipSlots,NULL,NULL
};
static char *readSource(const char *path)
{
    FILE *f=fopen(path,"rb");char *buf=NULL;long size=-1;
    if(!f)return NULL;
    if(fseek(f,0,SEEK_END)==0)size=ftell(f);
    if(size>0 && size<262144 && fseek(f,0,SEEK_SET)==0){
        buf=(char*)malloc((size_t)size+1);
        if(!buf || fread(buf,1,(size_t)size,f)!=(size_t)size){
            free(buf);buf=NULL;
        } else buf[size]=0;
    }
    fclose(f);
    return buf;
}
int main(void)
{
    struct OOGFOSJS js={0};
    struct ShipFixture shipA={10,0},shipB={20,0};
    char error[512]={0};
    char *source=readSource("SYS:Oolite/Scripts/oolite-global-prefix.js");
    LONG setup=JSERR_BADARGS,load=JSERR_BADARGS,verify=JSERR_BADARGS,
         result=-1;
    int boundA=0,boundB=0,exported=0,again=0,rejected=0,ok=0;
    if(!source){puts("OOLITE_GLOBAL_PREFIX asset=UNAVAILABLE");return 77;}
    if(!OOGFOSJSOpen(&js)){free(source);return 77;}
    boundA=OOGFOSJSExposeHostClass(&js,&shipClass,&shipA,"shipA");
    boundB=OOGFOSJSExposeHostClass(&js,&shipClass,&shipB,"shipB");
    if(boundA && boundB)
        exported=OOGFOSJSExposeHostConstructor(&js,"Ship","shipA");
    if(exported){
        again=OOGFOSJSExposeHostConstructor(&js,"Ship","shipB");
        rejected=!OOGFOSJSExposeHostConstructor(&js,"Ship);evil","shipA");
    }
    /* All non-Ship Oolite classes below are TEST-ONLY JS constructors.
     * This checks the actual source load; other OOJS host-class ports
     * must replace these fixtures before game startup. */
    if(exported && again && rejected)
        setup=OOGFOSJSEvaluateSource(&js,
            "this.global=this;"
            "this.worldScripts={ooliteWorld:{}};"
            "this.special={jsWarning:function(){}};"
            "this.Mission=function Mission(){};"
            "this.SystemInfo=function SystemInfo(){};"
            "this.System=function System(){};"
            "this.SoundSource=function SoundSource(){};"
            "this.Script=function Script(){};"
            "this.Vector3D=function Vector3D(x,y,z){this.x=x;this.y=y;this.z=z;};"
            "Vector3D.prototype.multiply=function(n){return new Vector3D(this.x*n,this.y*n,this.z*n);};"
            "this.system={info:{}};",
            "oolite-prefix-fixtures.js");
    if(setup==JSERR_OK)
        load=OOGFOSJSEvaluateChecked(&js,source,
            "oolite-global-prefix.js",error,sizeof(error));
    if(load==JSERR_OK)
        verify=OOGFOSJSEvaluateLong(&js,
            "shipA instanceof Ship && shipB instanceof Ship && "
            "Ship.prototype===Object.getPrototypeOf(shipA) && "
            "Ship.prototype===Object.getPrototypeOf(shipB) && "
            "Ship.prototype.constructor===Ship && "
            "shipA.spawnOne('trader')===42 && "
            "worldScriptNames.length===1 && "
            "worldScriptNames[0]==='ooliteWorld' && "
            "typeof Script.prototype.coordinatesForEscortPosition==='function' && "
            "typeof System.prototype.scrambledPseudoRandomNumber==='function' && "
            "typeof Mission.prototype.addMessageTextKey==='function' && "
            "typeof SoundSource.prototype.playSound==='function' && "
            "(function(){try{new Ship();return false;}catch(e){return e instanceof TypeError;}})()"
            " ? 42 : 0",
            "oolite-prefix-verify.js",&result);
    ok=boundA&&boundB&&exported&&again&&rejected &&
       setup==JSERR_OK&&load==JSERR_OK&&verify==JSERR_OK&&
       result==42&&shipA.spawnCalls==1&&shipB.spawnCalls==0;
    OOGFOSJSClose(&js);
    printf("OOLITE_GLOBAL_PREFIX setup=%ld load=%ld verify=%ld result=%ld shipCalls=%ld,%ld exported=%d idempotent=%d rejected=%d status=%s\n",
           (long)setup,(long)load,(long)verify,(long)result,
           (long)shipA.spawnCalls,(long)shipB.spawnCalls,exported,again,rejected,
           ok?"PASS":"FAIL");
    if(load!=JSERR_OK && error[0])printf("OOLITE_GLOBAL_PREFIX_ERROR %s\n",error);
    free(source);
    return ok?0:2;
}
