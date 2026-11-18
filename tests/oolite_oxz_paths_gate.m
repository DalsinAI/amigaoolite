/* Oolite OOOXZManager native path gate; methods reproduced from the patched pinned upstream source. */
#import <Foundation/Foundation.h>
#include <stdlib.h>
#include <stdio.h>
@interface OOOXZManager : NSObject
- (NSString*)installPath;
- (NSString*)extractAddOnsPath;
- (NSArray*)additionalAddOnsPaths;
@end
@implementation OOOXZManager
- (NSString*)installPath
{
#if OOLITE_GFOS
    const char *root = getenv("OO_MANAGEDADDONSDIR");
    return root != NULL && root[0] ? [NSString stringWithUTF8String:root] : @"SYS:Oolite/ManagedAddOns";
#else
    const char* managedAddOnsEnv = SDL_getenv("OO_MANAGEDADDONSDIR");

    if (managedAddOnsEnv) {
        return [NSString stringWithUTF8String:managedAddOnsEnv];
    } else {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        NSString* appPath = [paths objectAtIndex:0];
        if (appPath != nil) {
            appPath = [appPath stringByAppendingPathComponent:@"Oolite"];
#if OOLITE_MAC_OS_X
            appPath = [appPath stringByAppendingPathComponent:@"Managed AddOns"];
#else
            /* GNUStep uses "ApplicationSupport" rather than "Application
             * Support" so match convention by not putting a space in the
             * path either */
            appPath = [appPath stringByAppendingPathComponent:@"ManagedAddOns"];
#endif
            return appPath;
        }
    }
    return nil;
#endif
}

/* The extract path for OXZs . */
- (NSString*)extractAddOnsPath
{
#if OOLITE_GFOS
    const char *root = getenv("OO_ADDONSEXTRACTDIR");
    return root != NULL && root[0] ? [NSString stringWithUTF8String:root] : @"SYS:Oolite/AddOns";
#else
    const char* addOnsExtractEnv = SDL_getenv("OO_ADDONSEXTRACTDIR");

    if (addOnsExtractEnv) {
        return [NSString stringWithUTF8String:addOnsExtractEnv];
    } else {
#if OOLITE_WINDOWS
#if OO_GAME_DATA_TO_USER_FOLDER
        return [NSString stringWithFormat:@"%s\\Oolite\\AddOns", SDL_getenv("LOCALAPPDATA")];
#else
        return @"../AddOns";
#endif
#else
        return [[NSHomeDirectory() stringByAppendingPathComponent:@".Oolite"] stringByAppendingPathComponent:@"AddOns"];
#endif
    }
#endif
}

/* Add additional AddOns paths */
- (NSArray*)additionalAddOnsPaths
{
#if OOLITE_GFOS
    const char *dirs = getenv("OO_ADDITIONALADDONSDIRS");
    if (dirs == NULL || dirs[0] == 0) return [NSArray array];
    return [[NSString stringWithUTF8String:dirs] componentsSeparatedByString:@","];
#else
    const char* additionalAddOnsEnv = SDL_getenv("OO_ADDITIONALADDONSDIRS");

    if (additionalAddOnsEnv) {
        NSString* envStr = [NSString stringWithUTF8String:additionalAddOnsEnv];
        return [envStr componentsSeparatedByString:@","];
    }
    return [NSArray array];
#endif
}

@end
int main(int argc, char **argv) {
  char *environment[] = {NULL};
  GSInitializeProcess(argc, argv, environment);
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  OOOXZManager *manager = [[OOOXZManager alloc] init];
  NSString *managed = [manager installPath];
  NSString *addons = [manager extractAddOnsPath];
  NSArray *additional = [manager additionalAddOnsPaths];
  BOOL valid = [managed isEqualToString:@"SYS:Oolite/ManagedAddOns"] && [addons isEqualToString:@"SYS:Oolite/AddOns"] && [additional count] == 0;
  printf("OOLITE_OXZ_PATHS managed=%s addon=%s extra=%lu status=%s\n", [managed UTF8String], [addons UTF8String], (unsigned long)[additional count], valid ? "PASS" : "FAIL");
  fflush(stdout);
  [manager release]; [pool drain]; return valid ? 0 : 2;
}
