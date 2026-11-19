/*
 * Oolite OOJSScript.m (source migration, OoliteProject/oolite pinned commit
 * 21978c37b2f11c8664adf3e2062efc1d0c875979)
 *
 * Preserves original Objective-C script class, OOScript hierarchy,
 * name/description/version, original Unicode file loading, weak references,
 * per-script identity, manifest defaults, readonly properties and the
 * running-script stack. JavaScript executes exclusively on canonical
 * GFOpenJavaScript v2's existing SHARED JSTask through the existing
 * OOGFOSScriptInvoke/Exec owner-task RPC; no legacy JSAPI, JSRuntime,
 * JSContext, JSClass, GC-root address or separate compiler.
 *
 * Oolite contributors, GPL-2.0-or-later.
 */
#import "OOJSScriptGFOS.h"
#import "NSStringOOExtensions.h"
#import "OOPListParsing.h"
#import "OOManifestProperties.h"
#include "OOGFOSOriginalScriptBridge.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static uint32_t sNextScriptId=0;
typedef struct OOOriginalScriptStack OOOriginalScriptStack;
struct OOOriginalScriptStack {
    OOOriginalScriptStack *back;
    OOJSScript *script; /* Borrowed, just like pinned original stack */
};
static OOOriginalScriptStack *sRunningStack;
static NSString * const kLocalManifestProperty=@"oolite_manifest_identifier";

static NSString *StrippedName(NSString *string)
{
    static NSCharacterSet *invalidSet=nil;
    if(!invalidSet)
        invalidSet=[[NSCharacterSet characterSetWithCharactersInString:
                     @"_ \t\n\r\v"] retain];
    return [string stringByTrimmingCharactersInSet:invalidSet];
}
static NSString *scriptNameFromPath(NSString *path)
{
    NSString *last=nil,*name=nil,*folder;
    if(!path)return @"unidentified.anon-script";
    last=[path lastPathComponent];
    if(![last hasPrefix:@"script."])name=last;
    else {
        folder=[path stringByDeletingLastPathComponent];
        if([[folder lastPathComponent] caseInsensitiveCompare:@"Config"]==NSOrderedSame)
            folder=[folder stringByDeletingLastPathComponent];
        if([[folder pathExtension] caseInsensitiveCompare:@"oxp"]==NSOrderedSame)
            folder=[folder stringByDeletingPathExtension];
        name=[folder lastPathComponent];
    }
    if([name length]==0)name=path;
    return StrippedName([name stringByAppendingString:@".anon-script"]);
}
static NSDictionary *defaultPropertiesFromPath(NSString *path)
{
    NSString *manifestPath=[[[path stringByDeletingLastPathComponent]
                              stringByDeletingLastPathComponent]
                             stringByAppendingPathComponent:@"manifest.plist"];
    NSDictionary *manifest=OODictionaryFromFile(manifestPath);
    NSMutableDictionary *result=[NSMutableDictionary dictionaryWithCapacity:4];
    id identifier=[manifest objectForKey:kOOManifestIdentifier];
    if([identifier isKindOfClass:[NSString class]] &&
       ![identifier hasPrefix:@"__oolite.tmp."]){
        id value;
        if((value=[manifest objectForKey:kOOManifestVersion]))
            [result setObject:value forKey:@"version"];
        [result setObject:identifier forKey:kLocalManifestProperty];
        if((value=[manifest objectForKey:kOOManifestAuthor]))
            [result setObject:value forKey:@"author"];
        if((value=[manifest objectForKey:kOOManifestLicense]))
            [result setObject:value forKey:@"license"];
    }
    return result;
}
static int nativeProperty(NSString *key,id value,unsigned readonly,
                          struct OOGFOSOriginalProperty *out)
{
    memset(out,0,sizeof(*out));
    if(![key isKindOfClass:[NSString class]] || !value)return 0;
    out->name=[key UTF8String];
    if(!out->name || !out->name[0])return 0;
    out->readonly=readonly;
    if([value isKindOfClass:[NSString class]]){
        out->kind=OOGFOS_PROP_STRING;
        out->utf8=[value UTF8String];
        return out->utf8!=NULL;
    }
    if([value isKindOfClass:[NSNumber class]]){
        out->kind=OOGFOS_PROP_DOUBLE;
        out->number=[value doubleValue];
        return 1;
    }
    /* Never silently stringify arbitrary game-owned ObjC objects or
     * attempt cross-task dereferencing of OOJSValue, ShipEntity etc. */
    return 0;
}
@implementation OOJSScript
+ (id)scriptWithPath:(NSString *)path properties:(NSDictionary *)properties
{
    return [[[self alloc] initWithPath:path properties:properties] autorelease];
}
- (id)initWithPath:(NSString *)path properties:(NSDictionary *)properties
{
    NSString *fileContents,*fallback,*cachedName;
    NSDictionary *defaults;
    NSData *utf16;
    NSArray *defaultKeys,*customKeys;
    struct OOGFOSOriginalProperty list[64];
    uint32_t count=0,sequence;
    char nameText[512],descriptionText[2048],versionText[512];
    uint32_t i;
    int32_t rc;
    if(!(self=[super init]))return nil;
    validScript=NO;
    filePath=[path copy];
    if(!filePath)goto fail;
    fileContents=[NSString stringWithContentsOfUnicodeFile:path];
    if(!fileContents)goto fail;
    utf16=[fileContents utf16DataWithBOM:NO];
    if(!utf16 || [utf16 length]==0 ||
       ([utf16 length]%sizeof(unichar))!=0 ||
       [utf16 length]/sizeof(unichar)>262144)goto fail;
    sequence=++sNextScriptId;
    if(sequence==0)goto fail;
    if(snprintf(scriptKey,sizeof(scriptKey),"__ooliteScript%lu",
                (unsigned long)sequence)<0)goto fail;
    defaults=defaultPropertiesFromPath(path);
    defaultKeys=[defaults allKeys];
    customKeys=[properties allKeys];
    if([defaultKeys count]+[customKeys count]>64)goto fail;
    for(i=0;i<[defaultKeys count];i++){
        NSString *key=[defaultKeys objectAtIndex:i];
        if(!nativeProperty(key,[defaults objectForKey:key],
                           [key isEqualToString:kLocalManifestProperty],&list[count]))
            goto fail;
        count++;
    }
    for(i=0;i<[customKeys count];i++){
        NSString *key=[customKeys objectAtIndex:i];
        if(!nativeProperty(key,[properties objectForKey:key],1,&list[count]))
            goto fail;
        count++;
    }
    fallback=scriptNameFromPath(path);
    rc=OOGFOSOriginalScriptLoad(scriptKey,
        (const uint16_t *)[utf16 bytes],
        (uint32_t)([utf16 length]/sizeof(unichar)),
        [path UTF8String],list,count,[fallback UTF8String],
        nameText,sizeof(nameText),descriptionText,sizeof(descriptionText),
        versionText,sizeof(versionText));
    if(rc!=0)goto fail;
    cachedName=([NSString stringWithUTF8String:nameText]);
    if(!cachedName || [cachedName length]==0)cachedName=fallback;
    name=[StrippedName(cachedName) copy];
    description=descriptionText[0]?
        [[NSString stringWithUTF8String:descriptionText] copy]:nil;
    version=versionText[0]?
        [[NSString stringWithUTF8String:versionText] copy]:nil;
    validScript=YES;
    [filePath release];
    filePath=nil;
    return self;
fail:
    /* Bridge rollback drops failed candidate on JSTask. */
    if(scriptKey[0])OOGFOSOriginalScriptDrop(scriptKey);
    [self release];
    return nil;
}
- (void)dealloc
{
    if(validScript && scriptKey[0])
        OOGFOSOriginalScriptDrop(scriptKey);
    if(weakSelf)[weakSelf weakRefDrop];
    [name release];[description release];[version release];
    [filePath release];
    [super dealloc];
}
- (id)weakRetain
{
    if(!weakSelf)weakSelf=[OOWeakReference weakRefWithObject:self];
    return [weakSelf retain];
}
- (void)weakRefDied:(OOWeakReference *)ref
{
    if(ref==weakSelf)weakSelf=nil;
}
- (NSString *)name {return name?name:@"invalid script";}
- (NSString *)scriptDescription{return description;}
- (NSString *)version{return version;}
- (void)runWithTarget:(id)target {(void)target;}
+ (OOJSScript *)currentlyRunningScript
{return sRunningStack?sRunningStack->script:nil;}
+ (NSArray *)scriptStack
{
    OOOriginalScriptStack *current;
    NSMutableArray *result=[NSMutableArray array];
    for(current=sRunningStack;current;current=current->back)
        [result insertObject:current->script atIndex:0];
    return result;
}
+ (void)pushScript:(OOJSScript *)script
{
    OOOriginalScriptStack *node=(OOOriginalScriptStack *)malloc(sizeof(*node));
    if(!node)abort(); /* never continue with inconsistent nested script stack */
    node->back=sRunningStack;node->script=script;
    sRunningStack=node;
}
+ (void)popScript:(OOJSScript *)script
{
    OOOriginalScriptStack *node=sRunningStack;
    if(!node || node->script!=script)abort();
    sRunningStack=node->back;
    free(node);
}
- (BOOL)callMethodNamed:(NSString *)method arguments:(void *const *)argv
                   count:(uint32_t)argc result:(void **)outResult
{
    OOGFOSOwnedJSValue result=NULL;
    int32_t called=0,rc;
    if(outResult)*outResult=NULL;
    if(!validScript || !method || !outResult)return NO;
    [[self class] pushScript:self];
    rc=OOGFOSOriginalScriptCallMethod(scriptKey,[method UTF8String],argc,
                                     (OOGFOSOwnedJSValue *)argv,
                                     &result,&called);
    [[self class] popScript:self];
    if(rc!=0)return NO;
    *outResult=result; /* task-owned caller must release before reset */
    return YES; /* Missing handler is success, original OOJSScript contract. */
}
- (id)propertyNamed:(NSString *)property
{
    char output[2048];
    if(!validScript || !property ||
       OOGFOSOriginalScriptGetProperty(scriptKey,[property UTF8String],
                                     output,sizeof(output))!=0)
        return nil;
    return [NSString stringWithUTF8String:output];
}
- (BOOL)setProperty:(id)value named:(NSString *)property
{
    struct OOGFOSOriginalProperty p;
    if(!validScript || !nativeProperty(property,value,0,&p))return NO;
    return OOGFOSOriginalScriptSetProperty(scriptKey,&p)==0;
}
- (BOOL)defineProperty:(id)value named:(NSString *)property
{
    struct OOGFOSOriginalProperty p;
    if(!validScript || !nativeProperty(property,value,1,&p))return NO;
    return OOGFOSOriginalScriptSetProperty(scriptKey,&p)==0;
}
- (void)javaScriptEngineWillReset:(NSNotification *)notification
{
    (void)notification;
    /* Reset has retired this task's JSValue handles; no GC root address. */
    validScript=NO;
    scriptKey[0]=0;
}
@end
