/* Actual upstream Oolite NSNumber category under GFOpenGNUstep. */
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSValue.h>
#import "NSNumberOOExtensions.h"
#include <stdio.h>
#include <stddef.h>
int main(int argc,char **argv) {
  char *env[]={NULL}; unsigned failures=0;
  GSInitializeProcess(argc,argv,env);
  NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
  NSNumber *f=[NSNumber numberWithFloat:1.25f];
  NSNumber *d=[NSNumber numberWithDouble:2.5];
  NSNumber *i=[NSNumber numberWithInt:42];
  NSNumber *yes=[NSNumber numberWithBool:YES];
  if (![f oo_isFloatingPointNumber]) failures++;
  if (![d oo_isFloatingPointNumber]) failures++;
  if ([i oo_isFloatingPointNumber]) failures++;
  if (![yes oo_isBoolean]) failures++;
  printf("OOLITE_NUMBER_EXT_%s tests=4 failed=%u\n",failures?"FAIL":"PASS",failures);
  [pool drain];
  return failures?2:0;
}
