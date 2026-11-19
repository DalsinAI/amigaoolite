/* Native javascript.library ABI v2 guard against a v1 JavaScriptEngine.
 * The old task layout contains a pointer to a smaller function table;
 * JS_ClassPrototype must NEVER index past that table. No dummy Oolite
 * game objects or engine restart are needed to exercise this guard.
 * GPL-2.0-or-later.
 */
#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/javascript.h>
#include <libraries/javascript.h>
#include <stdio.h>
struct Library *JavaScriptBase=NULL;
struct LegacyTableHeader {ULONG jt_Version,jt_Size;};
struct LegacyTaskHead {const struct LegacyTableHeader *table;};
int main(void)
{
#if defined(JAVASCRIPT_CLASS_PROTOTYPE_VERSION)
    static const struct LegacyTableHeader old={1,8};
    struct LegacyTaskHead fake={&old};
    JSValue value;
    LONG version;
    int ok;
    JavaScriptBase=OpenLibrary((CONST_STRPTR)JAVASCRIPTNAME,
                              JAVASCRIPT_CLASS_PROTOTYPE_VERSION);
    if(!JavaScriptBase) {
        puts("OOLITE_JS_ABI_V2 library=UNAVAILABLE");return 77;
    }
    version=JavaScriptBase->lib_Version;
    value=JS_ClassPrototype((JSTask)&fake,(JSClass)(APTR)0x1234);
    ok=version>=2 && value==NULL;
    printf("OOLITE_JS_ABI_V2 library_version=%ld legacy_table_bytes=%lu prototype_null=%d status=%s\n",
           (long)version,(unsigned long)old.jt_Size,value==NULL,
           ok?"PASS":"FAIL");
    CloseLibrary(JavaScriptBase);JavaScriptBase=NULL;
    return ok?0:2;
#else
    puts("OOLITE_JS_ABI_V2 sdk=UNAVAILABLE");
    return 77;
#endif
}
