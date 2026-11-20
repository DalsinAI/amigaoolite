/* Probe: NSDistributedLock (what NSUserDefaults uses) and the file calls under it. */
#import <Foundation/Foundation.h>
#include "OOGFOSProcess.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char *environment[] = { NULL };
    OOGFOSAbsoluteArgv(argc, argv);
    GSInitializeProcess(argc, argv, environment);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSFileManager *fm = [NSFileManager defaultManager];

    NSArray *dirs = [NSArray arrayWithObjects:@"SYS:OoliteT5", @"PROGDIR:", @"RAM:", @"SYS:Prefs/Env-Archive/GNUstep/Defaults", nil];
    for (NSString *d in dirs) {
        printf("%-40s writable=%d access(W_OK)=%d\n", [d UTF8String], (int)[fm isWritableFileAtPath:d],
               access([d fileSystemRepresentation], W_OK));
    }

    NSString *dir = @"SYS:OoliteT5/lockprobe-dir";
    [fm createDirectoryAtPath:dir withIntermediateDirectories:NO attributes:nil error:NULL];
    printf("removeFileAtPath empty dir: %d\n", (int)[fm removeFileAtPath:dir handler:nil]);
    printf("  still exists: %d\n", (int)[fm fileExistsAtPath:dir]);

    NSString *lp = @"SYS:Prefs/Env-Archive/GNUstep/Defaults/probe.lck";
    NSDistributedLock *lk = [NSDistributedLock lockWithPath:lp];
    printf("tryLock: %d\n", (int)[lk tryLock]); fflush(stdout);
    NSDate *mine = [lk lockDate];
    NSDate *disk = [[fm fileAttributesAtPath:lp traverseLink:YES] fileModificationDate];
    printf("lockDate %.6f, on disk %.6f, equal %d\n", [mine timeIntervalSinceReferenceDate],
           [disk timeIntervalSinceReferenceDate], (int)[mine isEqual:disk]); fflush(stdout);
    @try { [lk unlock]; printf("unlock: ok\n"); }
    @catch (NSException *e) { printf("unlock raised: %s\n", [[e reason] UTF8String]); }
    printf("second tryLock: %d\n", (int)[lk tryLock]); fflush(stdout);
    @try { [lk unlock]; printf("second unlock: ok\n"); }
    @catch (NSException *e) { printf("unlock raised: %s\n", [[e reason] UTF8String]); }
    printf("lock dir still there: %d\n", (int)[fm fileExistsAtPath:lp]);
    fflush(stdout);
    [pool release];
    return 0;
}
