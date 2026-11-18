/* GFOS Oolite data-path smoke, native AmigaDOS path semantics. */
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSData.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
int main(int argc,char **argv)
{
  char *env[]={NULL};
  GSInitializeProcess(argc,argv,env);
  printf("RESOURCE_00_START\n"); fflush(stdout);
  NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
  NSString *path=[NSString stringWithUTF8String:"SYS:ACAgent/oolite-probe.txt"];
  printf("RESOURCE_01_STRING\n"); fflush(stdout);
  BOOL found=[[NSFileManager defaultManager] fileExistsAtPath:path];
  printf("RESOURCE_02_FOUND %d\n",found); fflush(stdout);
  NSData *data=[NSData dataWithContentsOfFile:path];
  printf("RESOURCE_03_DATA %lu\n",(unsigned long)[data length]); fflush(stdout);
  int ok=found && [data length]==strlen("oolite-resource-smoke-v1\n") && memcmp([data bytes],"oolite-resource-smoke-v1\n",strlen("oolite-resource-smoke-v1\n"))==0;
  printf("OOLITE_RESOURCE_%s\n",ok?"PASS":"FAIL"); fflush(stdout);
  [pool drain];
  return ok?0:2;
}
