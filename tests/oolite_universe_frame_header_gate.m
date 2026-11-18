/* Compile-only Objective-C ABI gate: original GNUstep Foundation/ObjC BOOL
 * must not conflict with Exec BOOL from the C-side dispatcher headers. */
#import <Foundation/Foundation.h>
#include "OOGFOSGameFrame.h"
_Static_assert(sizeof(BOOL)==1, "ObjC BOOL must stay one byte");
_Static_assert(sizeof(int)==4, "AmigaOS LONG must stay 32-bit");
_Static_assert(sizeof(unsigned int)==4, "AmigaOS ULONG must stay 32-bit");
int OOGFOSUniverseFrameHeaderGate(void)
{
    /* These are the exact function types imported by patched Universe.m. */
    int (*bind)(struct OOGFOSGameDispatch *, unsigned int)=OOGFOSGameFrameBind;
    int (*update)(void)=OOGFOSGameFrameUpdate;
    return bind != 0 && update != 0;
}
