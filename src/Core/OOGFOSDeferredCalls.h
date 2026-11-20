/*
 * OOGFOSDeferredCalls: -performSelector:withObject:afterDelay: for GFOS.
 *
 * The game runs on javascript.library's script task with no NSRunLoop (patch
 * 0019), so GNUstep's delayed performs never fire there and fail with "Failed to
 * get non block flag for perform in thread". Oolite's delayed calls (AI
 * reactions, delayed comms messages, cargo dumps, the afterburner sound) go
 * here instead; the game loop runs the due ones after every tick.
 *
 * Team 5, 20 Nov 2026. MIT.
 */
#import <Foundation/Foundation.h>

/* Retains target and object until the call has run, as NSObject's does. */
void OOGFOSPerformAfterDelay(id target, SEL selector, id object, NSTimeInterval delay);

/* Runs every call that is due. Calls queued meanwhile wait for the next one. */
void OOGFOSRunDeferredCalls(void);
