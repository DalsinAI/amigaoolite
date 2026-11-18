#include "OOOXZArchivePreflight.h"
#include <stdio.h>
int main(void) {
    unsigned count=0;
    int ok=OOGFOSPreflightOXZ("SYS:Oolite/fixture.oxz",&count) && count==1;
    int rejected=!OOGFOSPreflightOXZ("SYS:Oolite/does-not-exist.oxz",NULL);
    int escapeRejected=!OOGFOSPreflightOXZ("SYS:Oolite/escape.oxz",NULL);
    int duplicateRejected=!OOGFOSPreflightOXZ("SYS:Oolite/duplicate.oxz",NULL);
    printf("OOLITE_OXZ_PREFLIGHT valid=%d entries=%u missing_rejected=%d escape_rejected=%d duplicate_rejected=%d status=%s\n",ok,count,rejected,escapeRejected,duplicateRejected,ok&&rejected&&escapeRejected&&duplicateRejected?"PASS":"FAIL");fflush(stdout);
    return ok&&rejected&&escapeRejected&&duplicateRejected?0:2;
}
