/* Native pinned SpiderMonkey 1.8.5 diagnostic: isolate first blocked feature. */
#include <jsapi.h>
#include <stdio.h>
#include <string.h>
static JSClass globalClass={"global",JSCLASS_GLOBAL_FLAGS,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_StrictPropertyStub,JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub,JSCLASS_NO_OPTIONAL_MEMBERS};
static const char *scripts[]={"var x=3; x*14;","var o={x:1}; o.x;","var a=[1,2]; a[0]+a[1];","var s='abc'; s.length;"};
static const double expected[]={42,1,3,3};
int main(void){
 JSRuntime *rt=JS_NewRuntime(16*1024*1024);
 JSContext *cx=rt?JS_NewContext(rt,8192):NULL;
 JSObject *global=cx?JS_NewCompartmentAndGlobalObject(cx,&globalClass,NULL):NULL;
 if(!global){printf("OOLITE_JS_STAGE_SETUP_FAIL\n");fflush(stdout);return 2;}
 JSAutoEnterCompartment ac;
 if(!ac.enter(cx,global) || !JS_InitStandardClasses(cx,global)){printf("OOLITE_JS_STAGE_STDLIB_FAIL\n");fflush(stdout);return 2;}
 for(unsigned i=0;i<4;i++){
  printf("OOLITE_JS_STAGE_BEFORE_%u\n",i);fflush(stdout);
  jsval value=JSVAL_VOID;jsdouble number=-1;
  int ok=JS_EvaluateScript(cx,global,scripts[i],(uintN)strlen(scripts[i]),"oolite-stage",1,&value)&&JS_ValueToNumber(cx,value,&number)&&number==expected[i];
  printf("OOLITE_JS_STAGE_AFTER_%u ok=%d value=%.0f\n",i,ok,number);fflush(stdout);
  if(!ok)return 2;
 }
 printf("OOLITE_JS_STAGE_BEFORE_GC\n");fflush(stdout);
 JS_GC(cx);
 printf("OOLITE_JS_STAGE_AFTER_GC PASS\n");fflush(stdout);
 JS_DestroyContext(cx);JS_DestroyRuntime(rt);JS_ShutDown();return 0;
}
