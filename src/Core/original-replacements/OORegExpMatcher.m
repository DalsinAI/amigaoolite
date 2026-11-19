/*
 * Source-level port of Oolite's original OORegExpMatcher.m.
 * Keeps the Objective-C class, public selectors and NSString category.
 * The JSAPI-backed implementation is replaced by the real JavaScriptCore
 * RegExp through OOGFOSRegExp's narrow C ABI.
 *
 * Original copyright 2010–2013 Jens Ayton, MIT.
 * New bridge 2026 AmiCradle Team. Original MIT license preserved.
 */
#import "OORegExpMatcher.h"
#import "OOGFOSRegExp.h"

static OORegExpMatcher *sActiveInstance;

@implementation OORegExpMatcher
+ (instancetype)regExpMatcher
{
    NSAssert(![NSThread respondsToSelector:@selector(isMainThread)] ||
             [[NSThread currentThread] isMainThread],
             @"OORegExpMatcher must run on Oolite's game thread.");
    if(sActiveInstance==nil)
        sActiveInstance=[[[self alloc] init] autorelease];
    return sActiveInstance;
}
- (id)init
{
    self=[super init];
    if(!self)return nil;
    if(!OOGFOSRegExpOpen()){[self release];return nil;}
    return self;
}
- (void)dealloc
{
    if(sActiveInstance==self)sActiveInstance=nil;
    OOGFOSRegExpClose();
    [_cachedRegExpString release];
    [super dealloc];
}
- (BOOL)string:(NSString*)string matchesExpression:(NSString*)regexp
{
    return [self string:string matchesExpression:regexp flags:0];
}
- (BOOL)string:(NSString*)string matchesExpression:(NSString*)regexp
       flags:(NSUInteger)flags
{
    const char *source,*pattern;
    NSAssert(![NSThread respondsToSelector:@selector(isMainThread)] ||
             [[NSThread currentThread] isMainThread],
             @"OORegExpMatcher must run on Oolite's game thread.");
    if(!string||!regexp||![regexp length])return NO;
    source=[string UTF8String];pattern=[regexp UTF8String];
    if(!source||!pattern)return NO;
    return OOGFOSRegExpMatch(source,pattern,(unsigned long)flags)?YES:NO;
}
@end

@implementation NSString (OORegExpMatcher)
- (BOOL)oo_matchesRegularExpression:(NSString*)regexp
{
    return [[OORegExpMatcher regExpMatcher] string:self matchesExpression:regexp];
}
@end
