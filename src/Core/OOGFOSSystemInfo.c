#include "OOGFOSSystemInfo.h"
#include <exec/memory.h>
#include <proto/exec.h>
unsigned long OOGFOSMemoryFreeBytes(void)
{
    return (unsigned long)AvailMem(MEMF_ANY);
}
unsigned long OOGFOSMemoryTotalBytes(void)
{
    return (unsigned long)AvailMem(MEMF_TOTAL);
}
