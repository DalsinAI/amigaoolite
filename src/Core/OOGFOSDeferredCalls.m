/*
 * OOGFOSDeferredCalls: see OOGFOSDeferredCalls.h. Team 5, 20 Nov 2026. MIT.
 */
#import "OOGFOSDeferredCalls.h"

@interface OOGFOSDeferredCall : NSObject
{
@public
    id target;
    SEL selector;
    id object;
    NSTimeInterval due;
}
@end

@implementation OOGFOSDeferredCall
- (void)dealloc
{
    [target release];
    [object release];
    [super dealloc];
}
@end

static NSMutableArray *sCalls;

void OOGFOSPerformAfterDelay(id target, SEL selector, id object, NSTimeInterval delay)
{
    OOGFOSDeferredCall *call;
    if (target == nil || selector == NULL) return;
    if (sCalls == nil) sCalls = [[NSMutableArray alloc] init];
    call = [[OOGFOSDeferredCall alloc] init];
    call->target = [target retain];
    call->selector = selector;
    call->object = [object retain];
    call->due = [NSDate timeIntervalSinceReferenceDate] + (delay > 0 ? delay : 0);
    [sCalls addObject:call];
    [call release];
}

void OOGFOSRunDeferredCalls(void)
{
    NSTimeInterval now;
    NSMutableArray *due = nil;
    NSUInteger i, n;
    if ([sCalls count] == 0) return;
    now = [NSDate timeIntervalSinceReferenceDate];
    n = [sCalls count];
    for (i = 0; i < n; ) {
        OOGFOSDeferredCall *call = [sCalls objectAtIndex:i];
        if (call->due <= now) {
            if (due == nil) due = [NSMutableArray array];
            [due addObject:call];
            [sCalls removeObjectAtIndex:i];
            n--;
        } else {
            i++;
        }
    }
    for (i = 0; i < [due count]; i++) {
        OOGFOSDeferredCall *call = [due objectAtIndex:i];
        @try {
            [call->target performSelector:call->selector withObject:call->object];
        }
        @catch (id e) {
            NSLog(@"OOGFOSDeferredCalls: %@ from -%@ on %@", e, NSStringFromSelector(call->selector), call->target);
        }
    }
}
