/* Actual AmigaOS Exec memory availability for Oolite OOCPUInfo.m port. */
#include "OOGFOSSystemInfo.h"
#include <stdio.h>
int main(void)
{
    unsigned long total=OOGFOSMemoryTotalBytes();
    unsigned long freeBytes=OOGFOSMemoryFreeBytes();
    int ok=total>0 && freeBytes>0 && total>=freeBytes;
    printf("OOLITE_GFOS_SYSTEM_INFO total=%lu free=%lu total_mb=%lu free_mb=%lu status=%s\n",
           total,freeBytes,total/(1024UL*1024UL),freeBytes/(1024UL*1024UL),ok?"PASS":"FAIL");
    fflush(stdout);
    return ok?0:2;
}
