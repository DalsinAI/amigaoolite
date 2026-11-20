/* Probe: Objective-C exceptions with the plain GCC libobjc, no Foundation. */
#include <objc/runtime.h>
#include <stdio.h>

__attribute__((objc_root_class))
@interface OOProbeRoot { Class isa; }
+ (id) make;
@end
@implementation OOProbeRoot
+ (id) make { return class_createInstance(self, 0); }
@end

static void thrower(int x) { if (x) @throw [OOProbeRoot make]; }

int main(int argc, char **argv)
{
    (void)argv;
    int caughtAny = 0, caughtClass = 0;
    @try { thrower(argc); } @catch (id e) { caughtAny = 1; }
    printf("%s catch(id)\n", caughtAny ? "PASS" : "FAIL"); fflush(stdout);
    @try { thrower(argc); } @catch (OOProbeRoot *e) { caughtClass = 1; }
    printf("%s catch(OOProbeRoot *)\n", caughtClass ? "PASS" : "FAIL"); fflush(stdout);
    return caughtAny && caughtClass ? 0 : 10;
}
