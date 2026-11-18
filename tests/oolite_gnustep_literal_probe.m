#import <Foundation/Foundation.h>
#include <stdio.h>
int main(void)
{
    printf("OOLITE_OBJC_PROBE entering\n"); fflush(stdout);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *name = @"Oolite";
    printf("OOLITE_OBJC_PROBE literal=%s length=%lu\n", [name UTF8String], (unsigned long)[name length]);
    fflush(stdout);
    [pool drain];
    return 0;
}
