/* Probe: where GFOS Foundation looks for user defaults, and whether
 * NSUserDefaults and NSLog work once HOME names a real Amiga directory.
 * argv[1], if given, is used as HOME (default PROGDIR:). */
#import <Foundation/Foundation.h>
#include "OOGFOSProcess.h"
#include <stdio.h>
#include <stdlib.h>

#define STEP(s) do { fputs("STEP " s "\n", stdout); fflush(stdout); } while (0)

int main(int argc, char **argv)
{
    static char home[256];
    snprintf(home, sizeof home, "HOME=%s", argc > 1 ? argv[1] : "PROGDIR:");
    char *environment[] = { home, "GNUSTEP_USER_DEFAULTS_DIR=Defaults", NULL };
    OOGFOSAbsoluteArgv(argc, argv);
    printf("INFO argv[0] %s\n", argv[0]);
    GSInitializeProcess(argc, argv, environment);
    STEP("initialized");
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    printf("INFO user %s\n", [NSUserName() UTF8String]); fflush(stdout);
    printf("INFO home %s\n", [NSHomeDirectory() UTF8String]); fflush(stdout);
    printf("INFO defaults root %s\n", [GSDefaultsRootForUser(NSUserName()) UTF8String]); fflush(stdout);
    NSFileManager *fm = [NSFileManager defaultManager];
    STEP("NSThread sleepForTimeInterval 0.1");
    [NSThread sleepForTimeInterval:0.1];
    STEP("NSDate");
    NSDate *t0 = [NSDate date];
    printf("INFO interval %f\n", [t0 timeIntervalSinceNow]); fflush(stdout);
    STEP("createDirectoryAtPath PROGDIR:probe-dir");
    printf("%s createDirectory\n", [fm createDirectoryAtPath:@"PROGDIR:probe-dir" attributes:nil] ? "PASS" : "FAIL");
    fflush(stdout);
    STEP("createDirectoryAtPath with intermediates");
    printf("%s createDirectory intermediates\n",
           [fm createDirectoryAtPath:@"PROGDIR:probe-dir2/a/b" withIntermediateDirectories:YES attributes:nil error:NULL]
           ? "PASS" : "FAIL");
    fflush(stdout);
    STEP("standardUserDefaults (before any time zone)");
    NSUserDefaults *early = [NSUserDefaults standardUserDefaults];
    printf("%s early standardUserDefaults\n", early ? "PASS" : "FAIL"); fflush(stdout);
    STEP("NSTimeZone systemTimeZone");
    NSTimeZone *tz = [NSTimeZone systemTimeZone];
    printf("INFO system time zone %s\n", [[tz name] UTF8String]); fflush(stdout);
    STEP("NSTimeZone localTimeZone");
    printf("INFO local time zone %s\n", [[[NSTimeZone localTimeZone] name] UTF8String]); fflush(stdout);
    STEP("NSDate description");
    printf("INFO now %s\n", [[[NSDate date] description] UTF8String]); fflush(stdout);
    STEP("fileAttributesAtPath");
    NSDictionary *attrs = [fm fileAttributesAtPath:@"PROGDIR:probe-dir" traverseLink:YES];
    printf("%s attributes, date %s\n", attrs ? "PASS" : "FAIL",
           [[[attrs fileModificationDate] description] UTF8String]); fflush(stdout);
    STEP("NSDistributedLock");
    NSDistributedLock *lk = [NSDistributedLock lockWithPath:@"PROGDIR:probe.lck"];
    STEP("NSDistributedLock tryLock");
    printf("%s tryLock\n", [lk tryLock] ? "PASS" : "FAIL"); fflush(stdout);
    STEP("NSDistributedLock unlock");
    [lk unlock];
    STEP("[NSUserDefaults class]");
    [NSUserDefaults class];
    STEP("standardUserDefaults");
    NSUserDefaults *d = [NSUserDefaults standardUserDefaults];
    printf("%s standardUserDefaults\n", d ? "PASS" : "FAIL"); fflush(stdout);
    [d setObject:@"yes" forKey:@"GFOSProbe"];
    printf("%s defaults set/get\n", [[d stringForKey:@"GFOSProbe"] isEqualToString:@"yes"] ? "PASS" : "FAIL");
    fflush(stdout);
    STEP("synchronize");
    printf("%s synchronize\n", [d synchronize] ? "PASS" : "FAIL"); fflush(stdout);
    STEP("NSLog");
    NSLog(@"NSLog works");
    STEP("exception: @throw an NSException directly");
    int caughtDirect = 0;
    @try { @throw [NSException exceptionWithName:@"GFOSProbe" reason:@"direct" userInfo:nil]; }
    @catch (NSException *e) { caughtDirect = 1; }
    printf("%s direct @throw caught\n", caughtDirect ? "PASS" : "FAIL"); fflush(stdout);
    STEP("exception: stack trace alone");
    printf("INFO %u return addresses\n", (unsigned)[[NSThread callStackReturnAddresses] count]); fflush(stdout);
    STEP("exception: +raise:format:");
    int caught = 0;
    @try { [NSException raise:@"GFOSProbe" format:@"%d", 1]; }
    @catch (NSException *e) { caught = 1; }
    printf("%s +raise:format: caught\n", caught ? "PASS" : "FAIL"); fflush(stdout);
    [pool release];
    STEP("done");
    return 0;
}
