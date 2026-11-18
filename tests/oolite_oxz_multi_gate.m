#import <Foundation/Foundation.h>
#import "OOOXZGFOSExtraction.h"
#include <unistd.h>
#include <stdio.h>
int main(int argc,char **argv) {
 char *env[]={NULL}; GSInitializeProcess(argc,argv,env);
 NSAutoreleasePool *pool=[[NSAutoreleasePool alloc]init];
 NSString *dst=@"SYS:Oolite/AddOns/GFOSMultiProbe.oxp";
 BOOL extracted=OOGFOSExtractOXZ(@"SYS:Oolite/multi-valid.oxz",dst);
 NSData *a=[NSData dataWithContentsOfFile:[dst stringByAppendingPathComponent:@"manifest.plist"]];
 NSData *b=[NSData dataWithContentsOfFile:[dst stringByAppendingPathComponent:@"Config/shipdata.plist"]];
 NSData *c=[NSData dataWithContentsOfFile:[dst stringByAppendingPathComponent:@"Models/test.dat"]];
 BOOL corruptRejected=!OOGFOSExtractOXZ(@"SYS:Oolite/multi-crc-bad.oxz",@"SYS:Oolite/AddOns/GFOSBadMultiProbe.oxp");
 BOOL rollbackClean=access("SYS:Oolite/AddOns/GFOSBadMultiProbe.oxp.partial",F_OK)!=0 &&
                   access("SYS:Oolite/AddOns/GFOSBadMultiProbe.oxp",F_OK)!=0;
 BOOL pass=extracted && [a length]>0 && [b length]>0 && [c length]==12 && corruptRejected && rollbackClean;
 printf("OOLITE_OXZ_MULTI extracted=%d manifest=%lu config=%lu model=%lu corrupt_rejected=%d rollback_clean=%d status=%s\n",extracted,(unsigned long)[a length],(unsigned long)[b length],(unsigned long)[c length],corruptRejected,rollbackClean,pass?"PASS":"FAIL");fflush(stdout);
 [pool drain];return pass?0:2;
}
