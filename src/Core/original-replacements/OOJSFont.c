/* Oolite's original defaultFont.measureString Javascript interface, ported
 * from OOJSFont.m to existing GFOpenJavaScript v2. MIT licence as upstream.
 * Source migration: no SpiderMonkey JSAPI, fake type or silent fallback.
 */
#include "OOJSFontGFOS.h"
#include "OOGFOSGameDispatch.h"
#include <proto/javascript.h>
#include <string.h>
#include <math.h>

/* javascript.library callbacks run on its JSTask, NOT the game owner.
 * The original font width code must run on the game owner task. */
static JSValue measureString(JSTask task,APTR self,LONG argc,JSValue *argv,APTR data)
{
    struct OOGFOSGameDispatch *dispatch=(struct OOGFOSGameDispatch *)data;
    char text[256];
    LONG used;
    float width;
    DOUBLE number;
    (void)self;
    if(!dispatch || argc<1 || !argv || !argv[0] ||
       JS_TypeOf(task,argv[0])==JST_UNDEFINED)
        return JS_Throw(task,(CONST_STRPTR)"defaultFont.measureString requires text");
    /* JS_ToString's return is byte count (or a negative error), including
     * final NUL checking. Reject rather than silently truncate text. */
    used=JS_ToString(task,argv[0],(STRPTR)text,sizeof(text));
    if(used<0 || used>=(LONG)sizeof(text)-1)
        return JS_Throw(task,(CONST_STRPTR)"defaultFont.measureString text too long");
    text[sizeof(text)-1]=0;
    if(!OOGFOSGameDispatchMeasureText(dispatch,text,&width) || !isfinite(width))
        return JS_Throw(task,(CONST_STRPTR)"defaultFont.measureString failed on game task");
    number=(DOUBLE)width;
    return JS_NewDouble(task,&number);
}
int OOGFOSJSFontBindGame(struct OOGFOSGameDispatch *dispatch)
{
    return OOGFOSGameDispatchSetMeasureText(dispatch,
                                          OOGFOSJSFontMeasureOnGameTask);
}
LONG OOGFOSJSFontInstall(JSTask task,struct OOGFOSGameDispatch *dispatch)
{
    JSValue global=NULL,fn=NULL,ignored=NULL;
    LONG rc=JSERR_BADARGS;
    const char *initialize=
        "(function(g){"
        "var f=Object.create(null);"
        "Object.defineProperty(f,'measureString',"
        "{value:g.__ooliteFontMeasureNative,writable:false,"
        " enumerable:false,configurable:false});"
        "Object.defineProperty(g,'defaultFont',"
        "{value:Object.freeze(f),writable:false,"
        " enumerable:true,configurable:false});"
        "delete g.__ooliteFontMeasureNative;"
        "})(this)";
    if(!task || !dispatch || !dispatch->gamePort || !dispatch->measureTextBackend)
        return JSERR_BADARGS;
    global=JS_Global(task);
    if(!global)return JSERR_BADARGS;
    fn=JS_NewFunction(task,(CONST_STRPTR)"measureString",measureString,dispatch);
    if(!fn){JS_Free(task,global);return JSERR_NOMEMORY;}
    rc=JS_SetProp(task,global,(CONST_STRPTR)"__ooliteFontMeasureNative",fn);
    JS_Free(task,fn);
    JS_Free(task,global);
    if(rc!=JSERR_OK)return rc;
    rc=JS_Evaluate(task,(CONST_STRPTR)initialize,-1,
                   (CONST_STRPTR)"oolite-default-font-bootstrap.js",&ignored);
    if(ignored)JS_Free(task,ignored);
    return rc;
}
