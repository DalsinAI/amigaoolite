#import <Foundation/Foundation.h>
#import "OOOXZGFOSExtraction.h"
#include <unistd.h>
#include <stdio.h>
int main(int argc, char **argv) {
    char *env[]={NULL};GSInitializeProcess(argc,argv,env);
    NSAutoreleasePool *pool=[[NSAutoreleasePool alloc]init];
    NSString *good=@"SYS:Oolite/AddOns/GFOSRollbackProbe.oxp";
    BOOL installed=OOGFOSExtractOXZ(@"SYS:Oolite/fixture.oxz",good);
    NSData *data=[NSData dataWithContentsOfFile:[good stringByAppendingPathComponent:@"manifest.plist"]];
    BOOL crcRejected=!OOGFOSExtractOXZ(@"SYS:Oolite/crc-bad.oxz",@"SYS:Oolite/AddOns/BadCRCProbe.oxp");
    BOOL crcClean=access("SYS:Oolite/AddOns/BadCRCProbe.oxp.partial",F_OK)!=0 &&
        access("SYS:Oolite/AddOns/BadCRCProbe.oxp",F_OK)!=0;
    BOOL traversalRejected=!OOGFOSExtractOXZ(@"SYS:Oolite/escape.oxz",@"SYS:Oolite/AddOns/TraversalProbe.oxp");
    BOOL traversalClean=access("SYS:Oolite/AddOns/TraversalProbe.oxp.partial",F_OK)!=0 &&
        access("SYS:Oolite/AddOns/TraversalProbe.oxp",F_OK)!=0;
    BOOL pass=installed && [data length]==53 && crcRejected && crcClean && traversalRejected && traversalClean;
    printf("OOLITE_OXZ_ROLLBACK installed=%d bytes=%lu crc_rejected=%d crc_clean=%d traversal_rejected=%d traversal_clean=%d status=%s\n",installed,(unsigned long)[data length],crcRejected,crcClean,traversalRejected,traversalClean,pass?"PASS":"FAIL");fflush(stdout);
    [pool drain];return pass?0:2;
}
