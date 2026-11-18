/* Pinned Oolite SpiderMonkey 1.8.5: isolate JSON, exceptions, allocations. */
#include <jsapi.h>
#include <stdio.h>
#include <string.h>
static JSClass cls={"global",JSCLASS_GLOBAL_FLAGS,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_StrictPropertyStub,JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub,JSCLASS_NO_OPTIONAL_MEMBERS};
struct Script { const char *code; double expected; };
static const Script stages[]={
 {"var o={id:7,ship:'Cobra'}; JSON.parse(JSON.stringify(o)).id;",7},
 {"var result=0; try {throw new Error('test');} catch(err) {result=123;} result;",123},
 {"var cargo=[];var total=0;for(var i=0;i<20;i++){cargo.push({id:i,ship:'Cobra'+i});total+=i;}total;",190},
 {"var nums=[];for(var j=0;j<100;j++){nums.push(j);}nums.length;",100}
};
int main() {
 JSRuntime *rt=JS_NewRuntime(16*1024*1024);JSContext *cx=rt?JS_NewContext(rt,8192):NULL;
 JSObject *global=cx?JS_NewCompartmentAndGlobalObject(cx,&cls,NULL):NULL;
 if(!global){printf("OOLITE_FEATURES_SETUP_FAIL\n");fflush(stdout);return 2;}
 JSAutoEnterCompartment ac;if(!ac.enter(cx,global)||!JS_InitStandardClasses(cx,global)){printf("OOLITE_FEATURES_STDLIB_FAIL\n");fflush(stdout);return 2;}
 for(unsigned i=0;i<sizeof(stages)/sizeof(stages[0]);i++) {
  printf("OOLITE_FEATURES_BEFORE_%u\n",i);fflush(stdout);
  jsval ret=JSVAL_VOID;jsdouble number=-1;
  int ok=JS_EvaluateScript(cx,global,stages[i].code,(uintN)strlen(stages[i].code),"oolite-features",1,&ret) && JS_ValueToNumber(cx,ret,&number) && number==stages[i].expected;
  printf("OOLITE_FEATURES_AFTER_%u ok=%d value=%.0f\n",i,ok,number);fflush(stdout);
  if(!ok)return 2;
 }
 printf("OOLITE_FEATURES_BEFORE_GC\n");fflush(stdout);JS_GC(cx);
 printf("OOLITE_FEATURES_GC_PASS\n");fflush(stdout);
 JS_DestroyContext(cx);JS_DestroyRuntime(rt);JS_ShutDown();return 0;
}
