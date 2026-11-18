/* Oolite real RanrotWithSeed chunk range via Team 4 GFOpenMulticore.
 * CPU0 / OMC_ANY / board-only modes must match Linux reference.
 * Actual upstream legacy_random.c is compiled alongside this test. */
#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/gfopenmulticore.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "legacy_random.h"
#define CHUNKS 16
#define ITERS 524288u
struct Library *OpenMulticoreBase;
static LONG oolite_rng_worker(ULONG index, ULONG *out) {
    RANROTSeed state=MakeRanrotSeed(0x20010920u + index * 0x1111u);
    uint32_t hash=2166136261u;
    ULONG j;
    for(j=0;j<ITERS;j++) hash=(hash ^ RanrotWithSeed(&state))*16777619u;
    out[index]=(ULONG)hash;
    return 0;
}
static uint32_t hash_results(const ULONG *a) {
    uint32_t combined=0x12345678u; unsigned i;
    for(i=0;i<CHUNKS;i++) { combined=(combined<<5)|(combined>>27); combined^=a[i]; }
    return combined;
}
static int run(struct OMCJob *j, ULONG *out, ULONG target, ULONG flags, const char *tag) {
    ULONG args[1], failed=0xffffffffUL, n=CHUNKS; LONG err, grant_err;
    OMC_JobInit(j);
    j->omj_Entry=(APTR)oolite_rng_worker;
    args[0]=(ULONG)out;
    j->omj_Args=args; j->omj_NArgs=1;
    j->omj_Target=target; j->omj_Flags=flags; j->omj_StackSize=8192;
    grant_err=OMC_AddGrant(j,out,OMC_GRANTSIZE(n*4),OMCG_READ|OMCG_WRITE);
    if(grant_err!=OMCERR_OK) {
        printf("OOLITE_OMC_%s GRANT_FAIL %ld\n", tag,(long)grant_err);return 1;
    }
    clock_t t0=clock();
    err=OMC_RunRange(j,n,4,&failed);
    clock_t ticks=clock()-t0;
    uint32_t result=hash_results(out);
    printf("OOLITE_OMC_%s call=%ld status=%ld where=%lu failed=%lu cores=%lu hash=%08lx ticks=%ld tick_hz=%ld\n",
        tag,(long)err,(long)j->omj_Status,(unsigned long)j->omj_Where,
        (unsigned long)failed,(unsigned long)OMC_CoreCount(),(unsigned long)result,(long)ticks,(long)CLOCKS_PER_SEC);
    fflush(stdout);
    if(err!=OMCERR_OK || j->omj_Status!=OMCERR_OK || failed!=n || result!=0x081dedd2u) return 2;
    return 0;
}
int main(void) {
    struct OMCJob job;
    ULONG *out; ULONG units; int bad=0;
    OpenMulticoreBase=OpenLibrary(OPENMULTICORE_NAME,0);
    if(!OpenMulticoreBase) { puts("OOLITE_OMC_NO_LIBRARY: CPU0 fallback required"); return 3; }
    printf("OOLITE_OMC_LIBRARY version=%u revision=%u\n",(unsigned)OpenMulticoreBase->lib_Version,(unsigned)OpenMulticoreBase->lib_Revision);
    if(!OMC_HAS_JOBCALLS(OpenMulticoreBase) || !OMC_HAS_RANGE(OpenMulticoreBase)) {
        puts("OOLITE_OMC_NO_RANGE");CloseLibrary(OpenMulticoreBase);return 4;
    }
    units=OMC_CoreCount();
    out=OMC_AllocGrant(OMC_PAGESIZE,OMCAF_CLEAR|OMCAF_PAGE);
    if(!out) { puts("OOLITE_OMC_ALLOC_FAIL");CloseLibrary(OpenMulticoreBase);return 5; }
    bad+=run(&job,out,OMC_CPU0,0,"CPU0");
    bad+=run(&job,out,OMC_ANY,0,"ANY");
    if(units) {
        int r=run(&job,out,OMC_ANY,OMCF_BOARD,"BOARD_ONLY");
        bad+=r;
        if(!r && job.omj_Where!=OMCW_BOARD) { puts("OOLITE_OMC_NO_BOARD_PLACEMENT"); bad++; }
    }
    OMC_FreeGrant(out);
    CloseLibrary(OpenMulticoreBase);
    printf("OOLITE_OMC_%s errors=%d\n",bad?"FAIL":"PASS",bad);
    return bad?7:0;
}
