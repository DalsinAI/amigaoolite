/*
 * Native exercise of the replacement OORegExpMatcher JavaScriptCore bridge.
 * Tested against actual javascript.library v2 and the 68040 JSC engine.
 * GPL-2.0-or-later.
 */
#include "OOGFOSRegExp.h"
#include <stdio.h>
int main(void)
{
    int n=0,good=1,opened;
#define TEST(expect,source,pattern,flags) do { \
    int got=OOGFOSRegExpMatch(source,pattern,flags); \
    ++n; if(got!=(expect)){printf("regex fail %d: %s/%s got=%d\n",n,source,pattern,got);good=0;} \
} while(0)
    opened=OOGFOSRegExpOpen();
    if(!opened){puts("OOLITE_ORIGINAL_REGEX status=NO_ENGINE");return 77;}
    TEST(1,"Commander Jameson","Jameson",0);
    TEST(0,"Commander Jameson","jameson",0);
    TEST(1,"Commander Jameson","jameson",0x01);
    TEST(1,"alpha\nbeta","^beta$",0x04);
    TEST(0,"alpha\nbeta","^beta$",0);
    TEST(1,"a+b?","a\\+b\\?",0);
    TEST(1,"café","caf.",0);
    TEST(0,"Planet Lave","[",0); /* malformed pattern must fail safely */
    TEST(1,"Planet Lave","^Planet",0); /* recover after exception */
    TEST(0,"Planet Lave","Planet",0x02); /* unsupported legacy global mask */
    OOGFOSRegExpClose();
    printf("OOLITE_ORIGINAL_REGEX checks=%d all=%d status=%s\n",
           n,good,good?"PASS":"FAIL");
    return good?0:2;
}
