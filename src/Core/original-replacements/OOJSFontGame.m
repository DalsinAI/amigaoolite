/* Oolite's original OOJSFont measureString calls OOStringWidthInEm,
 * on the owner game task. Do not import NDK Exec BOOL into Objective-C.
 * Exact original font function is implemented by upstream HeadUpDisplay.m.
 * SPDX-License-Identifier: MIT
 */
#import <Foundation/Foundation.h>
#include <math.h>
extern CGFloat OOStringWidthInEm(NSString *text);
int OOGFOSJSFontMeasureOnGameTask(void *owner,const char *utf8,float *out)
{
    NSString *string;
    CGFloat width;
    (void)owner;
    if(!utf8 || !out)return 0;
    string=[NSString stringWithUTF8String:utf8];
    if(!string)return 0;
    width=OOStringWidthInEm(string);
    if(!isfinite(width))return 0;
    *out=(float)width;
    return 1;
}
