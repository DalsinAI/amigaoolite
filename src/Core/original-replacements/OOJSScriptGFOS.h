/* Original Oolite OOJSScript Objective-C class ported to canonical
 * GFOpenJavaScript. This SOURCE interface intentionally replaces all
 * legacy SpiderMonkey JSContext/jsid/jsval signatures with task-owned
 * opaque handles; callers must be migrated, not ABI-shimmed.
 * Original OOJSScript: Oolite contributors, GPL-2.0-or-later.
 */
#ifndef OOJSSCRIPT_GFOS_H
#define OOJSSCRIPT_GFOS_H
#import "OOScript.h"
#import "OOWeakReference.h"
#import <Foundation/Foundation.h>
#include <stdint.h>
@interface OOJSScript : OOScript <OOWeakReferenceSupport>
{
@private
    char scriptKey[64];
    NSString *name,*description,*version,*filePath;
    OOWeakReference *weakSelf;
    BOOL validScript;
}
+ (id)scriptWithPath:(NSString *)path properties:(NSDictionary *)props;
- (id)initWithPath:(NSString *)path properties:(NSDictionary *)props;
+ (OOJSScript *)currentlyRunningScript;
+ (NSArray *)scriptStack;
+ (void)pushScript:(OOJSScript *)script;
+ (void)popScript:(OOJSScript *)script;
- (NSString *)name;
- (NSString *)scriptDescription;
- (NSString *)version;
- (void)runWithTarget:(id)target;
/* JSValue result belongs to the CURRENT JSTask. Caller releases it
 * before reset. No borrowed SpiderMonkey JSID or JSVAL crosses here. */
- (BOOL)callMethodNamed:(NSString *)method
               arguments:(void *const *)args
                   count:(uint32_t)argc
                  result:(void **)outResult;
- (id)propertyNamed:(NSString *)property;
- (BOOL)setProperty:(id)value named:(NSString *)property;
- (BOOL)defineProperty:(id)value named:(NSString *)property;
- (void)javaScriptEngineWillReset:(NSNotification *)notification;
@end
#endif
