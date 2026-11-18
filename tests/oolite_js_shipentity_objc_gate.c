/* One JS task -> game-task Exec token router -> real Objective-C selectors.
 * Uses native GFOS Foundation and controlled test ShipEntity class. */
#include "OOGFOSJavaScript.h"
#include "OOGFOSShipEntityBackend.h"
#include <proto/javascript.h>
#include <stdio.h>
extern void *OOGFOSTestObjCShipNew(void);
extern void OOGFOSTestObjCShipFree(void *ship);
extern int OOGFOSTestObjCShipValues(void *ship,int *b,int *h,int *e,int *r);
extern int OOGFOSTestObjCReasonByScript(void);
struct Job{struct OOGFOSGameDispatch *d;LONG rc,result,finished;};
static LONG job(JSTask task,APTR data){
 struct Job *j=(struct Job *)data;JSValue result=NULL;LONG out=-1;
 j->rc=JS_Evaluate(task,(CONST_STRPTR)"ship.bounty=44;ship.homeSystem=42;ship.maxEscorts=4;ship.bounty+ship.homeSystem+ship.maxEscorts;",-1,(CONST_STRPTR)"oolite-real-objc-selector.js",&result);
 if(j->rc==JSERR_OK)j->rc=JS_ToLong(task,result,&out);
 if(result)JS_Free(task,result);
 j->result=out;j->finished=OOGFOSGameDispatchDone(j->d,j->rc,out);
 return j->rc;
}
static LONG join(JSTask task,APTR data){(void)task;(void)data;return JSERR_OK;}
int main(void){
 struct OOGFOSShipEntityRegistry registry;
 struct OOGFOSGameDispatch dispatch;
 struct OOGFOSGameShipProxy proxy;
 struct OOGFOSShipHandle handle;
 struct OOGFOSJS js;
 struct Job work;
 void *ship;ULONG token;LONG posted=-1,joined=-1;int bound=0,ok=0,closed=0,b=0,h=0,e=0,r=0,ret;
 OOGFOSShipEntityRegistryInit(&registry);
 ship=OOGFOSTestObjCShipNew();
 if(!ship)return 75;
 token=OOGFOSShipEntityRegister(&registry,ship);
 if(!token || !OOGFOSGameDispatchInitRouted(&dispatch,&registry,OOGFOSShipEntityGameBackend()))return 75;
 if(!OOGFOSJSOpen(&js)){OOGFOSGameDispatchClose(&dispatch);return 77;}
 proxy.dispatch=&dispatch;proxy.token=token;
 handle.ship=&proxy;handle.backend=OOGFOSGameShipProxyBackend();handle.valid=1;
 bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,(CONST_STRPTR)"ship");
 work.d=&dispatch;work.rc=-1;work.result=-1;work.finished=-1;
 if(bound)posted=JS_Post(js.task,job,&work,0);
 if(posted==JSERR_OK){do{ret=OOGFOSGameDispatchServe(&dispatch);}while(ret==0);joined=JS_Post(js.task,join,NULL,1);}
 OOGFOSTestObjCShipValues(ship,&b,&h,&e,&r);
 ok=bound&&posted==JSERR_OK&&joined==JSERR_OK&&dispatch.completions==1&&
    dispatch.requests==6&&dispatch.result==90&&work.rc==JSERR_OK&&
    work.finished==1&&b==44&&h==42&&e==4&&r==OOGFOSTestObjCReasonByScript();
 OOGFOSShipInvalidate(&handle);OOGFOSJSClose(&js);
 OOGFOSShipEntityUnregister(&registry,token);
 closed=OOGFOSGameDispatchTryClose(&dispatch);
 printf("OOLITE_JS_OBJC_BRIDGE bound=%d rpc=%lu done=%lu value=%ld bounty=%d home=%d escorts=%d legal_reason=%d closed=%d status=%s\n",bound,(unsigned long)dispatch.requests,(unsigned long)dispatch.completions,(long)dispatch.result,b,h,e,r,closed,ok&&closed?"PASS":"FAIL");fflush(stdout);
 OOGFOSTestObjCShipFree(ship);
 return ok&&closed?0:2;
}
