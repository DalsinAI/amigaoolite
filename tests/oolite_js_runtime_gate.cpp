/* Pinned Oolite SpiderMonkey 1.8.5 AmigaOS 68040 native runtime qualification. */
#include <jsapi.h>
#include <stdio.h>
#include <string.h>
static JSClass globalClass = {
  "global", JSCLASS_GLOBAL_FLAGS,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
  JSCLASS_NO_OPTIONAL_MEMBERS
};
int main() {
  JSRuntime *rt = JS_NewRuntime(16 * 1024 * 1024);
  JSContext *cx = rt ? JS_NewContext(rt,8192) : NULL;
  JSObject *global = cx ? JS_NewCompartmentAndGlobalObject(cx,&globalClass,NULL) : NULL;
  bool worked=false;
  double answer=-1;
  if (global) {
    JSAutoEnterCompartment guard;
    if (guard.enter(cx,global) && JS_InitStandardClasses(cx,global)) {
      const char *program="var result = 0; for (var i=1; i<=100; i++) result+=i; result;";
      jsval rval=JSVAL_VOID;
      jsdouble value=0;
      worked=JS_EvaluateScript(cx,global,program,(uintN)strlen(program),"oolite-js-gate",1,&rval) &&
              JS_ValueToNumber(cx,rval,&value);
      if(worked) answer=value;
    }
  }
  bool pass=worked && answer==5050.0;
  printf("OOLITE_MOZJS_68040 runtime=%d context=%d global=%d eval=%d value=%.0f status=%s\n",rt!=NULL,cx!=NULL,global!=NULL,worked,answer,pass?"PASS":"FAIL");fflush(stdout);
  if(cx) JS_DestroyContext(cx);
  if(rt) JS_DestroyRuntime(rt);
  JS_ShutDown();
  return pass?0:2;
}
