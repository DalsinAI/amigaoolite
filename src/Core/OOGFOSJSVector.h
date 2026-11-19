/* Source-level OOJSVector / Vector3D compatibility on canonical
 * javascript.library. JS-owned value objects, never game-entity pointers.
 * Coordinates use Oolite's HPVector double precision.
 * GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_JS_VECTOR_H
#define OOLITE_GFOS_JS_VECTOR_H
#include <libraries/javascript.h>
/* Install a JavaScript-callable Vector3D constructor. Does not require
 * initial ShipEntity or other world object; call on the owning JSTask. */
LONG OOGFOSJSVectorInstall(JSTask task);
/* New owned vector wrapper; caller must JS_Free after publishing it.
 * Returns NULL for allocation failure; no cross-task JSValue use. */
JSValue OOGFOSJSVectorCreate(JSTask task, DOUBLE x, DOUBLE y, DOUBLE z);
/* Convert exact GFOS Vector3D wrapper to native coordinates. */
LONG OOGFOSJSVectorRead(JSTask task,JSValue value,DOUBLE *x,DOUBLE *y,DOUBLE *z);
#endif
