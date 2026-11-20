/* gfosjsapi gate, launcher half: opens javascript.library v2, makes the
 * script task and runs the gate body on it as one job, as the game will run.
 * Kept apart from the body because the library's own calls are named JS_*. 
 * Team 5, 19 Nov 2026. */
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/javascript.h>
#include <stdio.h>
#include "oojsl.h"

struct Library *JavaScriptBase = NULL;

int gfosjsapiGateBody(void);

static LONG job(JSTask task, APTR data)
{
    (void)task;
    *(int *)data = gfosjsapiGateBody();
    return 0;
}

int main(void)
{
    struct TagItem tags[] = { { JSA_StackSize, 1024 * 1024 }, { JSA_Name, (ULONG)"OoliteGate" }, { TAG_DONE, 0 } };
    JSTask task;
    int failures = -1;
    JavaScriptBase = OpenLibrary((CONST_STRPTR)JAVASCRIPTNAME, 2);
    if (!JavaScriptBase) { printf("FAIL javascript.library v2\n"); return 20; }
    task = JS_NewTask(tags);
    if (!task) { printf("FAIL JS_NewTask\n"); CloseLibrary(JavaScriptBase); return 20; }
    ooL_SetTask(task);
    JS_Post(task, job, &failures, TRUE);
    printf("%s gfosjsapi gate: %d failure(s)\n", failures == 0 ? "PASS" : "FAIL", failures);
    JS_EndTask(task);
    CloseLibrary(JavaScriptBase);
    return failures == 0 ? 0 : 10;
}
