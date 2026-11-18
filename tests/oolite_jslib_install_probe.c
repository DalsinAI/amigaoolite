/* Diagnose canonical GFOS shared javascript.library installation only.
 * Does NOT start JavaScriptEngine or wait on its Exec message port.
 */
#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <stdio.h>
int main(void)
{
    struct Library *lib = OpenLibrary((CONST_STRPTR)"javascript.library", 1);
    BPTR engine = Lock((CONST_STRPTR)"LIBS:JavaScript/JavaScriptEngine", ACCESS_READ);
    BPTR engineSYS = Lock((CONST_STRPTR)"SYS:Libs/JavaScript/JavaScriptEngine", ACCESS_READ);
    BPTR json = Lock((CONST_STRPTR)"LIBS:json.library", ACCESS_READ);
    printf("OOLITE_JSLIB_DEPLOY lib=%d engine_libs=%d engine_sys=%d json=%d status=%s\n",
        lib != NULL, engine != 0, engineSYS != 0, json != 0,
        lib && engine && engineSYS && json ? "PASS" : "FAIL");
    if (engine) UnLock(engine);
    if (engineSYS) UnLock(engineSYS);
    if (json) UnLock(json);
    if (lib) CloseLibrary(lib);
    return 0; /* diagnostic, non-blocking even on missing components */
}
