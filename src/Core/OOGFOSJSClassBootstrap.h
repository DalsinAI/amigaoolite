/* Native Oolite JS_InitClass bootstrap using javascript.library ABI v2.
 * Produces the real JSHostClass prototype BEFORE a ship exists.
 * GPL-2.0-or-later. No SpiderMonkey jsval/JSContext binary emulation.
 */
#ifndef OOLITE_GFOS_JS_CLASS_BOOTSTRAP_H
#define OOLITE_GFOS_JS_CLASS_BOOTSTRAP_H
#include "OOGFOSJavaScript.h"
/* Return 1 when a native class is published globally; 0 for unsupported
 * library version, failed registration or name conflict.
 * hostClass and all slot data must remain valid until JSTask shutdown.
 * Oolite's script-defined prototype extensions then modify the same
 * prototype used by all subsequent JS_Wrap calls on this class. */
int OOGFOSJSBootstrapClass(struct OOGFOSJS *js,
                           const struct JSHostClass *hostClass,
                           CONST_STRPTR constructorName);
#endif
