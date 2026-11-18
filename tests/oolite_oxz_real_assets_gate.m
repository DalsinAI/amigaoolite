/* Original Oolite shipdata file, compressed in a test OXZ, extracted and parsed in native 68040 guest. */
#import <Foundation/Foundation.h>
#import "OOOXZGFOSExtraction.h"
#include <stdio.h>
int main(int argc,char **argv) {
    char *env[]={NULL}; GSInitializeProcess(argc,argv,env);
    NSAutoreleasePool *pool=[[NSAutoreleasePool alloc]init];
    NSString *destination=@"SYS:Oolite/AddOns/GFOSRealAssetsRepro.oxp";
    BOOL extracted=OOGFOSExtractOXZ(@"SYS:Oolite/real-assets.oxz",destination);
    NSData *shipdata=[NSData dataWithContentsOfFile:[destination stringByAppendingPathComponent:@"Config/shipdata.plist"]];
    NSString *err=nil; NSPropertyListFormat format=0;
    id plist=[NSPropertyListSerialization propertyListFromData:shipdata mutabilityOption:NSPropertyListImmutable format:&format errorDescription:&err];
    unsigned long count=[plist isKindOfClass:[NSDictionary class]]?(unsigned long)[plist count]:0;
    BOOL ok=extracted && [shipdata length]==128966 && count==194 && format==NSPropertyListOpenStepFormat;
    printf("OOLITE_OXZ_REAL_ASSETS extracted=%d bytes=%lu ships=%lu format=%lu status=%s\n",extracted,(unsigned long)[shipdata length],count,(unsigned long)format,ok?"PASS":"FAIL");fflush(stdout);
    [pool drain];return ok?0:2;
}
