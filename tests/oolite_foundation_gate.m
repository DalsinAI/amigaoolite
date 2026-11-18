/* Links Oolite's upstream OOIsNumberLiteral.m against GFOS GNUstep Base.
 * Test harness only; upstream source retained unmodified. */
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import "OOIsNumberLiteral.h"
#include <stdio.h>
#include <stddef.h>
int main(int argc, char **argv)
{
    char *env[] = { NULL };
    GSInitializeProcess(argc, argv, env);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    struct sample { const char *value; BOOL spaces; BOOL expected; } cases[] = {
        {"0", NO, YES}, {"-42", NO, YES}, {"3.14159", NO, YES},
        {"1e+3", NO, YES}, {".5", NO, YES}, {" 12 ", YES, YES},
        {" 12 ", NO, NO}, {"1e", NO, NO}, {"++4", NO, NO},
        {"", NO, NO}, {"2.1.4", NO, NO}, {"abc", NO, NO}
    };
    unsigned i, failures = 0;
    for (i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
        NSString *text = [NSString stringWithUTF8String:cases[i].value];
        BOOL actual = OOIsNumberLiteral(text, cases[i].spaces);
        if (actual != cases[i].expected) {
            printf("OOLITE_NUMBER_CASE_FAIL %u %s got=%d expected=%d\n",i,cases[i].value,actual,cases[i].expected);
            failures++;
        }
    }
    printf("OOLITE_FOUNDATION_%s cases=%u failed=%u\n",failures?"FAIL":"PASS",i,failures);
    [pool drain];
    return failures ? 2 : 0;
}
