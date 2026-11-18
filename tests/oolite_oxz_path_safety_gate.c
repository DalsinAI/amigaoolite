#include "OOOXZPathSafety.h"
#include <stdio.h>
#include <string.h>
struct Case { const char *s; int expected; };
int main(void) {
    const struct Case cases[] = {
        {"manifest.plist",1},{"Config/ships.plist",1},{"Models/",1},
        {"../escape",0},{"Configs/../../escape",0},{"/absolute",0},
        {"SYS:outside",0},{"a\\outside",0},{"a//b",0},
        {"./relative",0},{"a/./b",0},{"a/../b",0},
        {"..",0},{".",0},{"",0},{"a/",1},
        {"a/file.plist",1},{"a/..",0},{"a/.",0},
        {"a/\tfile",0}
    };
    unsigned fail=0; unsigned i;
    for (i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        int actual=OOGFOSSafeOXZEntry(cases[i].s, strlen(cases[i].s));
        if (actual!=cases[i].expected) { fail++; printf("BAD_CASE %u\n",i); }
    }
    char embedded[]={'a','\0','b'};
    if (OOGFOSSafeOXZEntry(embedded,sizeof(embedded))) fail++;
    char giant[512]; memset(giant,'x',sizeof(giant));
    if (OOGFOSSafeOXZEntry(giant,sizeof(giant))) fail++;
    printf("OOLITE_OXZ_SAFETY cases=22 failed=%u status=%s\n",fail,fail?"FAIL":"PASS");fflush(stdout);
    return fail?2:0;
}
