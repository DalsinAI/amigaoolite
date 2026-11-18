/* Native GFOpenGNUstep parser test using original Oolite OpenStep .plist data. */
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPropertyList.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSData.h>
#include <stdio.h>
#include <stddef.h>
int main(int argc, char **argv) {
  char *env[]={NULL}; int fails=0;
  const char *filenames[]={"demoships.plist","global-settings.plist","pirate-victim-roles.plist","role-categories.plist","trade-goods.plist","shipdata.plist","descriptions.plist","planetinfo.plist"};
  const unsigned counts[]={27,3,0,16,17,0,0,0};
  const BOOL arrays[]={YES,NO,YES,NO,NO,NO,NO,NO};
  GSInitializeProcess(argc,argv,env);
  NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
  NSError *dirError=nil;
  NSArray *listing=[[NSFileManager defaultManager] contentsOfDirectoryAtPath:@"SYS:Oolite/Config" error:&dirError];
  BOOL listed=(listing!=nil && [listing count]>=8);
  printf("OOLITE_CONFIG_ENUMERATE files=%lu status=%s\n",(unsigned long)[listing count],listed?"PASS":"FAIL"); fflush(stdout);
  if (!listed)fails++;
  for(unsigned i=0;i<8;i++) {
    NSString *file=[NSString stringWithFormat:@"SYS:Oolite/Config/%s",filenames[i]];
    BOOL exists=[[NSFileManager defaultManager] fileExistsAtPath:file];
    NSData *data=[NSData dataWithContentsOfFile:file];
    NSString *error=nil;
    NSPropertyListFormat format=0;
    id plist=[NSPropertyListSerialization propertyListFromData:data mutabilityOption:NSPropertyListImmutable format:&format errorDescription:&error];
    unsigned long entries= [plist respondsToSelector:@selector(count)] ? (unsigned long)[plist count]:0;
    BOOL typeOK=arrays[i] ? [plist isKindOfClass:[NSArray class]] : [plist isKindOfClass:[NSDictionary class]];
    BOOL ok=exists && [data length]>0 && plist!=nil && typeOK && (i<5 ? entries==counts[i] : entries>20) && format==NSPropertyListOpenStepFormat;
    printf("OOLITE_PLIST_%s exists=%d bytes=%lu entries=%lu format=%lu status=%s\n",filenames[i],exists,(unsigned long)[data length],entries,(unsigned long)format,ok?"PASS":"FAIL");fflush(stdout);
    if(!ok)fails++;
  }
  printf("OOLITE_REAL_ASSETS_%s files=8 failed=%d\n",fails?"FAIL":"PASS",fails);fflush(stdout);
  [pool drain]; return fails?2:0;
}
