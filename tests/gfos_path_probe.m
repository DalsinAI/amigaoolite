/* Probe: Amiga paths through libnix and through GNUstep's NSFileManager. */
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define STEP(s) do { fputs("STEP " s "\n", stdout); fflush(stdout); } while (0)

static void tryPath(NSFileManager *fm, NSString *p)
{
    const char *fs = [p fileSystemRepresentation];
    const char *fmfs = [fm fileSystemRepresentationWithPath:p];
    printf("PATH %s -> fs \"%s\", fm fs \"%s\", standardized \"%s\"\n", [p UTF8String], fs,
           fmfs ? fmfs : "(null)", [[p stringByStandardizingPath] UTF8String]);
    fflush(stdout);
    int rc = mkdir(fs, 0777);
    printf("  libnix mkdir rc=%d errno=%d\n", rc, rc ? errno : 0); fflush(stdout);
    struct stat st;
    rc = stat(fs, &st);
    printf("  libnix stat rc=%d dir=%d mode=0%lo size(struct stat)=%u\n", rc, rc == 0 && S_ISDIR(st.st_mode),
           (unsigned long)st.st_mode, (unsigned)sizeof st); fflush(stdout);
    printf("  NSFileManager fileType %s\n",
           [[[fm attributesOfItemAtPath:p error:NULL] fileType] UTF8String]); fflush(stdout);
    BOOL isDir0 = NO;
    printf("  NSFileManager sees libnix's dir: exists %d dir %d\n",
           (int)[fm fileExistsAtPath:p isDirectory:&isDir0], (int)isDir0); fflush(stdout);
    rc = rmdir(fs);
    printf("  libnix rmdir rc=%d, stat after %d\n", rc, stat(fs, &st)); fflush(stdout);
    STEP("createDirectoryAtPath");
    BOOL ok = [fm createDirectoryAtPath:p withIntermediateDirectories:NO attributes:nil error:NULL];
    printf("  NSFileManager create %d\n", (int)ok);
    fflush(stdout);
    BOOL isDir = NO;
    printf("  NSFileManager exists %d dir %d\n", (int)[fm fileExistsAtPath:p isDirectory:&isDir], (int)isDir);
    fflush(stdout);
    STEP("attributesOfItemAtPath");
    NSDictionary *a = [fm attributesOfItemAtPath:p error:NULL];
    printf("  attributes %s\n", a ? "yes" : "no"); fflush(stdout);
    [fm removeItemAtPath:p error:NULL];
}

int main(int argc, char **argv)
{
    char *environment[] = { NULL };
    GSInitializeProcess(argc, argv, environment);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSFileManager *fm = [NSFileManager defaultManager];
    printf("INFO currentDirectoryPath %s\n", [[fm currentDirectoryPath] UTF8String]); fflush(stdout);
    tryPath(fm, @"RAM:gfos-pp1");
    tryPath(fm, @"SYS:OoliteT5/gfos-pp2");
    tryPath(fm, @"PROGDIR:gfos-pp3");
    tryPath(fm, @"gfos-pp4");
    [pool release];
    STEP("done");
    return 0;
}
