/* Source boundary for ORIGINAL Objective-C OOJSScript -> canonical GFOS.
 * No SpiderMonkey types may cross this interface; no second JS engine.
 * Game-owner-only functions; internally reuse OOGFOSScriptInvoke and
 * OOGFOSScriptObjects on the already running shared JSTask.
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef OOLITE_GFOS_ORIGINAL_SCRIPT_BRIDGE_H
#define OOLITE_GFOS_ORIGINAL_SCRIPT_BRIDGE_H
#include <stdint.h>
struct OOGFOSJS;
struct OOGFOSGameDispatch;
typedef void *OOGFOSOwnedJSValue;
enum { OOGFOS_PROP_STRING=1, OOGFOS_PROP_DOUBLE=2,
       OOGFOS_PROP_BOOLEAN=3 };
struct OOGFOSOriginalProperty {
    const char *name;
    const char *utf8;
    double number;
    uint32_t kind;
    uint32_t readonly;
};
int OOGFOSOriginalScriptBind(struct OOGFOSJS *js,
                           struct OOGFOSGameDispatch *dispatch);
void OOGFOSOriginalScriptUnbind(void);
/* Original InitOOJSScript installs a real Script prototype before running
 * oolite-global-prefix.js. Call once per new GFOS JSTask after reset. */
int32_t OOGFOSOriginalScriptInitClasses(void);
/* Returns canonical JSERR code (0 means success). Loads one Oolite script
 * with the exact shared JSTask and Script.prototype; optional properties
 * are set BEFORE source is executed. Identity is a validated safe JS key. */
int32_t OOGFOSOriginalScriptLoad(const char *scriptKey,
                                const uint16_t *utf16,uint32_t units,
                                const char *fileName,
                                const struct OOGFOSOriginalProperty *properties,
                                uint32_t propertyCount,
                                const char *defaultName,
                                char *name,uint32_t nameSize,
                                char *description,uint32_t descriptionSize,
                                char *version,uint32_t versionSize);
int32_t OOGFOSOriginalScriptDrop(const char *scriptKey);
int32_t OOGFOSOriginalScriptSetProperty(const char *scriptKey,
                                       const struct OOGFOSOriginalProperty *property);
int32_t OOGFOSOriginalScriptGetProperty(const char *scriptKey,
                                       const char *property,
                                       char *output,uint32_t capacity);
int32_t OOGFOSOriginalScriptCallMethod(const char *scriptKey,
                                      const char *method,
                                      uint32_t argc,
                                      OOGFOSOwnedJSValue *args,
                                      OOGFOSOwnedJSValue *result,
                                      int32_t *called);
/* Reuse owner task and existing JSValue ownership contract. */
void OOGFOSOriginalScriptReleaseValue(OOGFOSOwnedJSValue result);
#endif
