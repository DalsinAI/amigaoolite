/* GFOS source port of Oolite OOJSFont's defaultFont.measureString.
 * The font belongs to the game task and is accessed with bounded Exec RPC.
 * The shared javascript.library JSTask installs the original JS contract.
 * Upstream Oolite OOJSFont.m copyright/licence retained at origin.
 * SPDX-License-Identifier: MIT
 */
#ifndef OOJSFONT_GFOS_H
#define OOJSFONT_GFOS_H
#include <libraries/javascript.h>
struct OOGFOSGameDispatch;
/* From game task BEFORE JS scripts start. The callback uses the ORIGINAL
 * OOStringWidthInEm() function and Foundation only on the owner. */
int OOGFOSJSFontBindGame(struct OOGFOSGameDispatch *dispatch);
/* Call ON the already-existing shared JSTask during original global setup.
 * Installs read-only global defaultFont.measureString(text) returning ems.
 * The dispatcher must remain open until all scripts have exited. */
LONG OOGFOSJSFontInstall(JSTask task, struct OOGFOSGameDispatch *dispatch);
int OOGFOSJSFontMeasureOnGameTask(void *owner,const char *utf8,float *out);
#endif
