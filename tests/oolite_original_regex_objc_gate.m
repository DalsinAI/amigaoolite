/*
 * Original OORegExpMatcher Objective-C interface on GFOS JavaScriptCore:
 * executable integration regression for real Oolite selectors.
 * GPL-2.0-or-later; original matcher remains MIT licensed.
 */
#import <Foundation/Foundation.h>
#import "OORegExpMatcher.h"
#include <stdio.h>
int main(void)
{
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    OORegExpMatcher *matcher = [OORegExpMatcher regExpMatcher];
    int good=matcher != nil,checks=0;
#define PROBE(expect,actual) do { \
    BOOL got = (actual); \
    ++checks; if ((got ? 1:0) != (expect)) {good=0;printf("objc regex fail %d\n",checks);} \
} while(0)
    if(good) {
        PROBE(1, [matcher string:@"Commander Jameson" matchesExpression:@"Jameson"]);
        PROBE(0, [matcher string:@"Commander Jameson" matchesExpression:@"jameson"]);
        PROBE(1, [matcher string:@"Commander Jameson" matchesExpression:@"jameson" flags:kOORegExpCaseInsensitive]);
        PROBE(1, [matcher string:@"alpha\nbeta" matchesExpression:@"^beta$" flags:kOORegExpMultiLine]);
        PROBE(1, [@"Planet Lave" oo_matchesRegularExpression:@"Planet"]);
        PROBE(0, [@"Planet Lave" oo_matchesRegularExpression:@"["]);
        PROBE(1, [@"Planet Lave" oo_matchesRegularExpression:@"Lave"]);
    }
    printf("OOLITE_ORIGINAL_REGEX_OBJC checks=%d status=%s\n",checks,good?"PASS":"FAIL");
    [pool drain];
    return good?0:2;
}
