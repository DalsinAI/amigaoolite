/* Native Oolite Quaternion host class for the canonical GFOS javascript.library.
 * Task-owned JS value snapshots (not game-owned Entity objects).
 * GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_JS_QUATERNION_H
#define OOLITE_GFOS_JS_QUATERNION_H
#include <libraries/javascript.h>
LONG OOGFOSJSQuaternionInstall(JSTask task);
JSValue OOGFOSJSQuaternionCreate(JSTask task,DOUBLE w,DOUBLE x,DOUBLE y,DOUBLE z);
LONG OOGFOSJSQuaternionRead(JSTask task,JSValue value,
                           DOUBLE *w,DOUBLE *x,DOUBLE *y,DOUBLE *z);
#endif
