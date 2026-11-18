/* Optional Oolite Universe frame integration for shared GFOS javascript.library.
 * GPL-2.0-or-later. Owner task retains and shuts down dispatcher.
 * This header must be safe for Objective-C: never include AmigaOS exec/types.h
 * here, because it defines BOOL as 16-bit while objc/objc.h uses 8-bit BOOL.
 */
#ifndef OOLITE_GFOS_GAME_FRAME_H
#define OOLITE_GFOS_GAME_FRAME_H
struct OOGFOSGameDispatch;
/* m68k AmigaOS ULONG/LONG are 32-bit unsigned/signed int. Keep ABI stable
 * without dragging Exec's BOOL typedef into Objective-C Universe.m. */
int OOGFOSGameFrameBind(struct OOGFOSGameDispatch *dispatch, unsigned int budget);
int OOGFOSGameFrameUpdate(void);
#endif
