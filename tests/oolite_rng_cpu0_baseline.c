/* Deterministic, independently seeded Oolite numeric workload baseline.
 * Actual upstream RanrotWithSeed()/MakeRanrotSeed() is compiled alongside.
 * Not an implementation of GFOpenMulticore or evidence of board use. */
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "legacy_random.h"
#define OOLITE_CHUNKS 16
#define OOLITE_ITERS 524288
static uint32_t chunk(uint32_t seed) {
    RANROTSeed state=MakeRanrotSeed(seed);
    uint32_t hash=2166136261u;
    unsigned j;
    for(j=0;j<OOLITE_ITERS;++j) {
      uint32_t n=RanrotWithSeed(&state);
      hash=(hash ^ n)*16777619u;
    }
    return hash;
}
int main(void) {
    uint32_t combined=0x12345678u; unsigned i;
    clock_t begin=clock();
    for(i=0;i<OOLITE_CHUNKS;++i) {
       uint32_t h=chunk(0x20010920u+i*0x1111u);
       combined=(combined<<5)|(combined>>27);
       combined^=h;
       printf("OOLITE_CORE0_CHUNK %u %08lx\n", i,(unsigned long)h);
    }
    clock_t elapsed=clock()-begin;
    printf("OOLITE_CPU0_BASELINE chunks=%d iterations=%d checksum=%08lx clock_ticks=%ld ticks_per_sec=%ld\n",OOLITE_CHUNKS,OOLITE_ITERS,(unsigned long)combined,(long)elapsed,(long)CLOCKS_PER_SEC);
    fflush(stdout);
    return 0;
}
