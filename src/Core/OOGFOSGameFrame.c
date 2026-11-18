#include "OOGFOSGameFrame.h"
#include <proto/exec.h>
/* The game task alone owns this pointer. It is not accessed by JS tasks. */
static struct OOGFOSGameDispatch *sDispatch;
static ULONG sBudget;
int OOGFOSGameFrameBind(struct OOGFOSGameDispatch *d, ULONG budget)
{
    /* No change to a live binding from a foreign task. */
    if (sDispatch && FindTask(NULL) != sDispatch->gameTask) return 0;
    if (!d) { sDispatch = NULL; sBudget = 0; return 1; }
    if (!budget || !d->gamePort || FindTask(NULL) != d->gameTask)
        return 0;
    /* Do not replace a live owner's dispatch behind queued callers. */
    if (sDispatch && sDispatch != d) return 0;
    sDispatch = d;
    sBudget = budget;
    return 1;
}
LONG OOGFOSGameFrameUpdate(void)
{
    struct OOGFOSGameDispatch *d = sDispatch;
    if (!d) return 0; /* Opt-in, no extra game tick work until initialized. */
    if (FindTask(NULL) != d->gameTask) return -1;
    return OOGFOSGameDispatchPumpFrame(d, sBudget);
}
