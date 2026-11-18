/* Native Oolite OXZ manifest gate using pinned upstream MiniZip and GFOpenGNUstep. */
#import <Foundation/Foundation.h>
#include "unzip.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#import "OOManifestProperties.h"

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
static int schemaValid(NSDictionary *manifest) {
    if (![manifest isKindOfClass:[NSDictionary class]]) return 0;
    NSArray *keys = [NSArray arrayWithObjects:kOOManifestIdentifier,kOOManifestVersion,kOOManifestRequiredOoliteVersion,kOOManifestTitle,nil];
    for (NSString *key in keys) {
        id value = [manifest objectForKey:key];
        if (![value isKindOfClass:[NSString class]] || [value length]==0) return 0;
    }
    return 1;
}
static int loadSchema(const char *path, int *parsed) {
    NSData *data=nil;
    NSString *error=nil;
    NSPropertyListFormat format=0;
    *parsed=0;
    if (!readManifest(path,&data)) return 0;
    id result=[NSPropertyListSerialization propertyListFromData:data mutabilityOption:NSPropertyListImmutable format:&format errorDescription:&error];
    *parsed=(result!=nil && [result isKindOfClass:[NSDictionary class]]);
    return *parsed && schemaValid((NSDictionary *)result);
}
int main(int argc, char **argv) {
    char *env[]={NULL}; GSInitializeProcess(argc,argv,env);
    NSAutoreleasePool *pool=[[NSAutoreleasePool alloc]init];
    int parsedGood=0,parsedBad=0;
    int accepted=loadSchema("SYS:Oolite/schema-valid.oxz",&parsedGood);
    int rejected=!loadSchema("SYS:Oolite/schema-invalid.oxz",&parsedBad);
    int ok=parsedGood && parsedBad && accepted && rejected;
    printf("OOLITE_OXZ_SCHEMA good_parsed=%d good_accepted=%d bad_parsed=%d missing_title_rejected=%d status=%s\n",parsedGood,accepted,parsedBad,rejected,ok?"PASS":"FAIL");fflush(stdout);
    [pool drain];return ok?0:2;
}
