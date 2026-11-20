/* gfosjsapi: XDR (script serialisation). javascript.library keeps compiled
 * scripts itself (JSA_CacheDir), so these exist only so Oolite's cache code
 * compiles: JS_XDRNewMem returns NULL and Oolite falls back to compiling. */
#ifndef GFOSJSAPI_JSXDRAPI_H
#define GFOSJSAPI_JSXDRAPI_H
#include "jsapi.h"
#ifdef __cplusplus
extern "C" {
#endif
JSXDRState *JS_XDRNewMem(JSContext *cx, JSXDRMode mode);
void *JS_XDRMemGetData(JSXDRState *xdr, uint32 *lp);
void JS_XDRMemSetData(JSXDRState *xdr, void *data, uint32 len);
void JS_XDRDestroy(JSXDRState *xdr);
JSBool JS_XDRScript(JSXDRState *xdr, JSScript **scriptp);
#ifdef __cplusplus
}
#endif
#endif
