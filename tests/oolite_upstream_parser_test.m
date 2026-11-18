/* Execute the original Oolite property-list API, ported for AmigaDOS. */
#import <Foundation/Foundation.h>
#import "OOPListParsing.h"
#include <stdio.h>
#include <stddef.h>
int main(int argc, char **argv)
{
  char *env[]={NULL}; unsigned failures=0;
  GSInitializeProcess(argc,argv,env);
  NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
  NSDictionary *ships=OODictionaryFromFile(@"SYS:Oolite/Config/shipdata.plist");
  NSDictionary *planet=OODictionaryFromFile(@"SYS:Oolite/Config/planetinfo.plist");
  NSDictionary *trade=OODictionaryFromFile(@"SYS:Oolite/Config/trade-goods.plist");
  NSArray *demos=OOArrayFromFile(@"SYS:Oolite/Config/demoships.plist");
  NSArray *empty=OOArrayFromFile(@"SYS:Oolite/Config/pirate-victim-roles.plist");
  printf("OOLITE_UPSTREAM_PARSER ship=%lu planet=%lu trade=%lu demo=%lu empty=%lu\n",
    (unsigned long)[ships count],(unsigned long)[planet count],
    (unsigned long)[trade count],(unsigned long)[demos count],
    (unsigned long)[empty count]); fflush(stdout);
  if([ships count]!=194)failures++;
  if([planet count]!=2050)failures++;
  if([trade count]!=17)failures++;
  if([demos count]!=27)failures++;
  if(empty==nil || [empty count]!=0)failures++;
  if(OOArrayFromFile(@"SYS:Oolite/Config/shipdata.plist")!=nil)failures++;
  if(OODictionaryFromFile(@"SYS:Oolite/Config/no-such-file.plist")!=nil)failures++;
  printf("OOLITE_UPSTREAM_PARSER_%s checks=7 failed=%u\n",failures?"FAIL":"PASS",failures);fflush(stdout);
  [pool drain];return failures?2:0;
}
