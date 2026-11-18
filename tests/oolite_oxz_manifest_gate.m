/* Native Oolite OXZ manifest gate using pinned upstream MiniZip and GFOpenGNUstep. */
#import <Foundation/Foundation.h>
#include "unzip.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int readManifest(const char *path, NSData **out) {
    unzFile zip = unzOpen64(path);
    int result = 0;
    if (zip == NULL) return 0;
    if (unzLocateFile(zip, "manifest.plist", 1) != UNZ_OK) goto done;
    unz_file_info64 info = {0};
    if (unzGetCurrentFileInfo64(zip, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) goto done;
    if (info.uncompressed_size == 0 || info.uncompressed_size > (1024 * 1024)) goto done;
    if (unzOpenCurrentFile(zip) != UNZ_OK) goto done;
    unsigned char *buffer = malloc((size_t)info.uncompressed_size);
    if (buffer != NULL) {
        size_t used = 0;
        int n;
        while (used < (size_t)info.uncompressed_size) {
            n = unzReadCurrentFile(zip, buffer + used, (unsigned)((size_t)info.uncompressed_size-used));
            if (n <= 0) break;
            used += (size_t)n;
        }
        if (used == (size_t)info.uncompressed_size && unzCloseCurrentFile(zip) == UNZ_OK) {
            *out = [NSData dataWithBytes:buffer length:used];
            result = *out != nil;
        } else { unzCloseCurrentFile(zip); }
        free(buffer);
    } else { unzCloseCurrentFile(zip); }
done:
    unzClose(zip);
    return result;
}
int main(int argc, char **argv) {
    char *env[]={NULL}; GSInitializeProcess(argc, argv, env);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSData *data=nil; NSDictionary *manifest=nil;
    NSString *err=nil; NSPropertyListFormat format=0;
    int decoded=readManifest("SYS:Oolite/fixture.oxz",&data);
    if (decoded) {
        id candidate=[NSPropertyListSerialization propertyListFromData:data mutabilityOption:NSPropertyListImmutable format:&format errorDescription:&err];
        if ([candidate isKindOfClass:[NSDictionary class]]) manifest=candidate;
    }
    NSString *identifier=[manifest objectForKey:@"identifier"];
    NSString *version=[manifest objectForKey:@"version"];
    BOOL pass=decoded && [identifier isEqualToString:@"org.oolite.test"] && [version isEqualToString:@"1.0"];
    printf("OOLITE_OXZ_MANIFEST archive=%d bytes=%lu identifier=%s version=%s status=%s\n", decoded, (unsigned long)[data length], identifier?[identifier UTF8String]:"(nil)", version?[version UTF8String]:"(nil)", pass?"PASS":"FAIL"); fflush(stdout);
    [pool drain]; return pass?0:2;
}
