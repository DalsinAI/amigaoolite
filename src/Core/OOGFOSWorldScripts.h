/* Original Oolite OOJSWorldScripts global registry on canonical
 * javascript.library. World script entries are enumerable, read-only
 * references to per-script JS objects. GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_WORLD_SCRIPTS_H
#define OOLITE_GFOS_WORLD_SCRIPTS_H
#include <libraries/javascript.h>
/* Both functions execute on the owning JSTask, normally from JS_Post.
 * One global worldScripts object is created before oolite-global-prefix.js
 * and made read-only; scripts are added under Oolite world-script names,
 * including hyphens. Original Oolite does not delete world script entries. */
LONG OOGFOSWorldScriptsInit(JSTask task);
LONG OOGFOSWorldScriptsRegister(JSTask task, CONST_STRPTR scriptGlobal,
                               CONST_STRPTR worldName);
#endif
