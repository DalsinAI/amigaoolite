/* ABI groundwork for pinned SpiderMonkey 1.8.5, no fake JS implementation. */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
int main(void) {
    union { uint32_t n; unsigned char byte[4]; } e = { 0x01020304u };
    int big = e.byte[0] == 1 && e.byte[1] == 2 && e.byte[2] == 3 && e.byte[3] == 4;
    int supported = big && sizeof(void*) == 4 && sizeof(long) == 4 &&
        sizeof(size_t) == 4 && sizeof(uint64_t) == 8 && sizeof(double) == 8 &&
        CHAR_BIT == 8;
    printf("OOLITE_JS_ABI ptr=%lu long=%lu size_t=%lu u64=%lu double=%lu long_double=%lu big_endian=%d status=%s\n",(unsigned long)sizeof(void*),(unsigned long)sizeof(long),(unsigned long)sizeof(size_t),(unsigned long)sizeof(uint64_t),(unsigned long)sizeof(double),(unsigned long)sizeof(long double),big,supported?"PASS":"FAIL");fflush(stdout);
    return supported?0:2;
}
