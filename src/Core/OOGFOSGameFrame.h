/* Optional Oolite Universe frame integration for shared GFOS javascript.library.
 * GPL-2.0-or-later. Owner task retains and shuts down dispatcher. */
#ifndef OOLITE_GFOS_GAME_FRAME_H
#define OOLITE_GFOS_GAME_FRAME_H
#include "OOGFOSGameDispatch.h"
/* Must be invoked on the owner task after the dispatch has been initialized.
 * A NULL dispatch uninstalls. Never free a dispatch until uninstalled and
 * all worker callbacks have joined, and the dispatch is safely closed. */
int OOGFOSGameFrameBind(struct OOGFOSGameDispatch *dispatch, ULONG budget);
/* Called at the start of -[Universe update:]. Returns -1 if wrong task. */
LONG OOGFOSGameFrameUpdate(void);
#endif
