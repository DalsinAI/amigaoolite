/* Oolite native AmigaOS resource root gate. The builtInPath method below is inserted verbatim from the pinned upstream ResourceManager with patch 0003. */
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <stdlib.h>
@interface ResourceManager : NSObject
+ (NSString*)builtInPath;
@end
@implementation ResourceManager
+ (NSString*)builtInPath
{
#if OOLITE_GFOS
    const char *configuredRoot = getenv("OOLITE_RESOURCE_ROOT");
    if (configuredRoot != NULL && configuredRoot[0] != 0) return [NSString stringWithUTF8String:configuredRoot];
    return @"SYS:Oolite";
#else
    NSFileManager* fileManager = [NSFileManager defaultManager];

#if OOLITE_MAC_COCOA_UI
    return [[NSBundle mainBundle] resourcePath]; // Use resourcePath directly
#else
    NSString* startingDir = [fileManager currentDirectoryPath]; // Start from cwd
#endif
    // Look for a "Resources" folder (Windows & Linux)
    NSString* primaryResourcesPath = [startingDir stringByAppendingPathComponent:@"Resources"];
    BOOL isDir = NO;
    if ([fileManager fileExistsAtPath:primaryResourcesPath isDirectory:&isDir] && isDir) {
        return primaryResourcesPath;
    }
    // Fallback: Look in startingDir/../share/oolite/Resources
    NSString* fallbackPath = [[startingDir stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"share/oolite/Resources"];
    return [fallbackPath stringByStandardizingPath];
#endif
}
@end
int main(int argc, char **argv) {
  char *env[]={NULL};
  GSInitializeProcess(argc,argv,env);
  NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
  NSString *root=[ResourceManager builtInPath];
  NSString *config=[root stringByAppendingPathComponent:@"Config"];
  NSString *ship=[config stringByAppendingPathComponent:@"shipdata.plist"];
  NSString *planet=[config stringByAppendingPathComponent:@"planetinfo.plist"];
  NSFileManager *fm=[NSFileManager defaultManager];
  BOOL isdir=NO;
  BOOL dirOK=[fm fileExistsAtPath:config isDirectory:&isdir] && isdir;
  NSData *sd=[NSData dataWithContentsOfFile:ship];
  NSData *pd=[NSData dataWithContentsOfFile:planet];
  NSString *err=nil; NSPropertyListFormat fmt=0;
  id ships=[NSPropertyListSerialization propertyListFromData:sd mutabilityOption:NSPropertyListImmutable format:&fmt errorDescription:&err];
  id planets=[NSPropertyListSerialization propertyListFromData:pd mutabilityOption:NSPropertyListImmutable format:&fmt errorDescription:&err];
  BOOL ok=dirOK && [ships isKindOfClass:[NSDictionary class]] && [ships count]==194 && [planets isKindOfClass:[NSDictionary class]] && [planets count]==2050;
  printf("OOLITE_RESOURCE_ROOT root=%s dir=%d ship=%lu planet=%lu status=%s\n",[root UTF8String],dirOK,(unsigned long)[ships count],(unsigned long)[planets count],ok?"PASS":"FAIL");fflush(stdout);
  [pool drain]; return ok?0:2;
}
