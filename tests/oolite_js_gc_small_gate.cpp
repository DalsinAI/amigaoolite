/* Real pinned SpiderMonkey 1.8.5 on GFOS Amiga 68040: allocation, objects,
 * JSON, exceptions and forced GC, CPU0 only. */
#include <jsapi.h>
#include <stdio.h>
#include <string.h>
static JSClass globalClass = {
 "global", JSCLASS_GLOBAL_FLAGS,
 JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
 JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
 JSCLASS_NO_OPTIONAL_MEMBERS
};
static void reporter(JSContext *,const char *msg,JSErrorReport *) {
 fprintf(stderr,"OOLITE_MOZJS_GC_SCRIPT_ERROR %s\n",msg);fflush(stderr);
}
int main() {
 JSRuntime *rt=JS_NewRuntime(16*1024*1024);
 JSContext *cx=rt?JS_NewContext(rt,8192):NULL;
 JSObject *global=cx?JS_NewCompartmentAndGlobalObject(cx,&globalClass,NULL):NULL;
 int iterations=0;int ok=0;int gcCalls=0;double value=-1;
 if(global) {
  JS_SetErrorReporter(cx,reporter);
  JSAutoEnterCompartment compartment;
  if(compartment.enter(cx,global) && JS_InitStandardClasses(cx,global)) {
   const char *script="var cargo=[]; var total=0; for(var i=0;i<20;i++){var e={id:i,ship:'Cobra'+i};cargo.push(e);total+=e.id;} var o=JSON.parse(JSON.stringify(cargo[10])); try{throw new Error('test');}catch(err){total+=100;} total+o.id;";
   ok=1;
   for(int i=0;i<1;i++) {
    jsval ret=JSVAL_VOID;jsdouble v=0;
    if(!JS_EvaluateScript(cx,global,script,(uintN)strlen(script),"oolite-js-gc",1,&ret) || !JS_ValueToNumber(cx,ret,&v) || v!=300.0) {ok=0;break;}
    value=v;iterations++;
    printf("OOLITE_MOZJS_SMALL_BEFORE_GC value=%.0f\n",value);fflush(stdout);JS_GC(cx);gcCalls++;printf("OOLITE_MOZJS_SMALL_AFTER_GC\n");fflush(stdout);
   }
  }
 }
 const bool pass=rt&&cx&&global&&ok&&iterations==1&&gcCalls==1;
 printf("OOLITE_MOZJS_GC runtime=%d context=%d scripts=%d gc=%d value=%.0f status=%s\n",rt!=NULL,cx!=NULL,iterations,gcCalls,value,pass?"PASS":"FAIL");fflush(stdout);
 if(cx)JS_DestroyContext(cx);if(rt)JS_DestroyRuntime(rt);JS_ShutDown();
 return pass?0:2;
}
