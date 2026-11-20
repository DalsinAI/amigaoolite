/* Gate: can Oolite's game code run ON javascript.library's script task?
 *
 * The program's main() (the launching process) makes one JSTask and posts
 * the "game" to it as a single job (JS_Post, wait). Everything the game does
 * then runs on the script task: GNUstep Foundation, the C library, files,
 * pthreads, and JavaScript calls in both directions, which are direct C
 * calls there. For comparison, the same JS call is timed from the launching
 * task (each call handed over and waited for).
 *
 * Prints one PASS/FAIL line per check and a TIMING line per measurement.
 * No SpiderMonkey anywhere. Team 5, 19 Nov 2026.
 */
/* Amiga headers first, with their BOOL renamed: Objective-C has its own. */
#define BOOL AMIGA_BOOL
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <dos/dosextens.h>
#include <devices/timer.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <proto/javascript.h>
#undef BOOL
#import <Foundation/Foundation.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Library *JavaScriptBase = NULL;
struct Device *TimerBase = NULL;
static struct timerequest timerReq;
static ULONG eclockHz = 1;
static int failures = 0;

static void check(const char *what, int ok)
{
    printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    fflush(stdout);
    if (!ok) failures++;
}

static unsigned long long now(void)
{
    struct EClockVal e;
    eclockHz = ReadEClock(&e);
    return ((unsigned long long)e.ev_hi << 32) | e.ev_lo;
}

static void timing(const char *what, unsigned long long t0, unsigned long long t1, long n)
{
    double us = (double)(t1 - t0) * 1e6 / (double)eclockHz;
    printf("TIMING %s: %ld calls, %.0f us total, %.2f us per call\n", what, n, us, us / (double)n);
    fflush(stdout);
}

/* A native function scripts call: does Objective-C work on the script task. */
static JSValue nativeObjC(JSTask task, APTR self, LONG argc, JSValue *argv, APTR data)
{
    LONG i = 0;
    (void)self; (void)data;
    if (argc > 0) JS_ToLong(task, argv[0], &i);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *s = [NSString stringWithFormat:@"%ld", (long)i];
    LONG back = (LONG)[s intValue];
    [pool release];
    return JS_NewLong(task, back);
}

/* A native function with no work: the cost of the JS -> C call alone. */
static JSValue nativeEmpty(JSTask task, APTR self, LONG argc, JSValue *argv, APTR data)
{
    (void)self; (void)argc; (void)argv; (void)data;
    return JS_NewLong(task, 1);
}

static LONG emptyJob(JSTask task, APTR data) { (void)task; (void)data; return 0; }

static void *threadBody(void *arg) { *(int *)arg = 42; return NULL; }

/* libnix pthreads keeps its threads in a private table (pthread.c, threads[],
 * global in our copy of libpthread.a: objcopy --globalize-symbol=__threads).
 * Slot 0 is the main thread; its Task pointer is at offset 20. pthread_self()
 * answers 0 for any task not in the table, so the main slot must name the
 * task that really runs the game. */
extern void **_threads;
#define OOGFOS_PTHREAD_TASK_OFFSET 20
static struct Task *OOGFOSAdoptMainThread(void)
{
    struct Task **slot = (struct Task **)((char *)_threads[0] + OOGFOS_PTHREAD_TASK_OFFSET);
    struct Task *previous;
    Forbid();
    previous = *slot;
    *slot = FindTask(NULL);
    Permit();
    return previous;
}
static void OOGFOSReleaseMainThread(struct Task *previous)
{
    struct Task **slot = (struct Task **)((char *)_threads[0] + OOGFOS_PTHREAD_TASK_OFFSET);
    Forbid();
    *slot = previous;
    Permit();
}

struct GameArgs { BPTR homeDir; BPTR currentDir; struct Task *launcher; };

static LONG gameMain(JSTask task, APTR data)
{
    struct GameArgs *g = (struct GameArgs *)data;
    struct Task *me = FindTask(NULL);
    check("game job runs on a different task from the launcher", me != g->launcher);
    check("script task is a DOS process", me->tc_Node.ln_Type == NT_PROCESS);
    printf("INFO script task \"%s\", stack %lu bytes\n", me->tc_Node.ln_Name ? me->tc_Node.ln_Name : "?",
           (unsigned long)((char *)me->tc_SPUpper - (char *)me->tc_SPLower));

    /* Make it look like the game's own process for DOS. */
    BPTR oldDir = CurrentDir(g->currentDir);
    BPTR oldHome = 0;
    if (me->tc_Node.ln_Type == NT_PROCESS) oldHome = SetProgramDir(g->homeDir);

    /* C library */
    char *m = malloc(100000);
    check("malloc/free", m != NULL);
    if (m) { memset(m, 1, 100000); free(m); }
    char buf[64];
    snprintf(buf, sizeof buf, "%d %.3f", 7, 2.5);
    check("snprintf with float", strcmp(buf, "7 2.500") == 0);
    FILE *f = fopen("PROGDIR:oolite_js_game_on_script_task.tmp", "w");
    check("fopen PROGDIR: for writing", f != NULL);
    if (f) { fputs("ok\n", f); fclose(f); }
    f = fopen("PROGDIR:oolite_js_game_on_script_task.tmp", "r");
    check("fopen PROGDIR: for reading", f != NULL && fgets(buf, sizeof buf, f) && strcmp(buf, "ok\n") == 0);
    if (f) fclose(f);
    remove("PROGDIR:oolite_js_game_on_script_task.tmp");

    /* pthreads from the script task */
    pthread_t th;
    int out = 0;
    int prc = pthread_create(&th, NULL, threadBody, &out);
    if (prc == 0) pthread_join(th, NULL);
    check("pthread_create/join", prc == 0 && out == 42);

    /* Foundation: the script task takes over pthread slot 0 (the main thread)
     * while the launcher is parked in JS_Post, so GNUstep sees one main thread. */
    struct Task *mainOwner = OOGFOSAdoptMainThread();
    check("script task adopted as the main pthread", mainOwner == g->launcher && pthread_self() == 0);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    check("NSThread currentThread is the main thread", [NSThread currentThread] != nil && [NSThread isMainThread]);
    NSString *s = [NSString stringWithFormat:@"%@-%d-%.1f", @"oolite", 3, 1.5];
    check("NSString stringWithFormat", [s isEqualToString:@"oolite-3-1.5"]);
    NSMutableDictionary *d = [NSMutableDictionary dictionary];
    [d setObject:[NSNumber numberWithInt:5] forKey:@"five"];
    NSArray *a = [NSArray arrayWithObjects:@"a", @"b", nil];
    check("NSDictionary/NSArray", [[d objectForKey:@"five"] intValue] == 5 && [a count] == 2);
#if OOGFOS_GATE_EXCEPTIONS   /* needs -fexceptions at compile AND link (build/gfos-runtime.mk) */
    int caught = 0;
    @try { @throw [NSException exceptionWithName:@"OOTest" reason:@"plain" userInfo:nil]; }
    @catch (NSException *e) { caught = [[e name] isEqualToString:@"OOTest"]; }
    check("@throw/@catch on the script task", caught);
#endif
    check("NSFileManager PROGDIR:", [[NSFileManager defaultManager] fileExistsAtPath:@"PROGDIR:"]);
    NSDictionary *plist = [@"{ name = Cobra; speed = 0.35; }" propertyList];
    check("property list parse", [[plist objectForKey:@"name"] isEqualToString:@"Cobra"]);
    [pool release];

    /* JavaScript on the script task: every call is direct */
    JSValue fn = NULL, r = NULL;
    LONG rc = JS_Evaluate(task, (CONST_STRPTR)"(function(x){return x+1;})", -1, (CONST_STRPTR)"gate.js", &fn);
    check("JS_Evaluate on the script task", rc == JSERR_OK && fn);
    JSValue global = JS_Global(task);
    JSValue nObjC = JS_NewFunction(task, (CONST_STRPTR)"nativeObjC", nativeObjC, NULL);
    JSValue nEmpty = JS_NewFunction(task, (CONST_STRPTR)"nativeEmpty", nativeEmpty, NULL);
    JS_SetProp(task, global, (CONST_STRPTR)"nativeObjC", nObjC);
    JS_SetProp(task, global, (CONST_STRPTR)"nativeEmpty", nEmpty);

    const long N = 10000;
    unsigned long long t0 = now();
    long okCount = 0;
    for (long i = 0; i < N; i++) {
        JSValue arg = JS_NewLong(task, i);
        r = NULL;
        if (JS_Call(task, fn, NULL, 1, &arg, &r) == JSERR_OK) {
            LONG v = 0;
            JS_ToLong(task, r, &v);
            if (v == i + 1) okCount++;
        }
        JS_Free(task, arg);
        if (r) JS_Free(task, r);
    }
    unsigned long long t1 = now();
    check("C -> JS calls on the script task give right answers", okCount == N);
    timing("C -> JS call on the script task (JS_NewLong + JS_Call + JS_ToLong + 2 JS_Free)", t0, t1, N);

    t0 = now();
    rc = JS_Evaluate(task, (CONST_STRPTR)"var s=0; for (var i=0;i<10000;i++) s+=nativeEmpty(); s", -1,
                     (CONST_STRPTR)"gate2.js", &r);
    t1 = now();
    LONG sum = 0;
    if (r) { JS_ToLong(task, r, &sum); JS_Free(task, r); }
    check("JS -> C empty native 10000 times", rc == JSERR_OK && sum == 10000);
    timing("JS -> C call, empty native (includes the JS loop)", t0, t1, 10000);

    t0 = now();
    rc = JS_Evaluate(task, (CONST_STRPTR)"var t=0; for (var i=0;i<10000;i++) t+=nativeObjC(i); t", -1,
                     (CONST_STRPTR)"gate3.js", &r);
    t1 = now();
    sum = 0;
    if (r) { JS_ToLong(task, r, &sum); JS_Free(task, r); }
    check("JS -> Objective-C native 10000 times", rc == JSERR_OK && sum == 49995000);
    timing("JS -> C call doing NSString work", t0, t1, 10000);

    JS_Free(task, nObjC);
    JS_Free(task, nEmpty);
    JS_Free(task, global);
    JS_Free(task, fn);
    OOGFOSReleaseMainThread(mainOwner);

    if (oldHome) SetProgramDir(oldHome);
    CurrentDir(oldDir);
    return failures;
}

int main(int argc, char **argv)
{
    char *environment[] = { NULL };
    GSInitializeProcess(argc, argv, environment);   /* as Oolite's GFOS main.m does */
    printf("oolite_js_game_on_script_task_gate\n");
    /* Start Foundation on the launching process (it has the CLI and is the main thread). */
    NSAutoreleasePool *mainPool = [[NSAutoreleasePool alloc] init];
    printf("STEP main Foundation: %s, main thread %d, process %s\n",
           [[NSString stringWithFormat:@"%d", 1] UTF8String], (int)[NSThread isMainThread],
           [[[NSProcessInfo processInfo] processName] UTF8String]);
    fflush(stdout);
    /* NSUserDefaults/NSLog are covered by tests/gfos_defaults_probe.m. */

    if (OpenDevice((CONST_STRPTR)TIMERNAME, UNIT_ECLOCK, (struct IORequest *)&timerReq, 0) != 0) {
        printf("FAIL timer.device\n");
        return 20;
    }
    TimerBase = timerReq.tr_node.io_Device;
    JavaScriptBase = OpenLibrary((CONST_STRPTR)JAVASCRIPTNAME, 2);
    if (!JavaScriptBase) {
        printf("FAIL javascript.library v2\n");
        CloseDevice((struct IORequest *)&timerReq);
        return 20;
    }
    struct TagItem tags[] = { { JSA_StackSize, 1024 * 1024 }, { JSA_Name, (ULONG)"Oolite" }, { TAG_DONE, 0 } };
    JSTask task = JS_NewTask(tags);
    check("JS_NewTask with a 1 MB stack", task != NULL);
    if (!task) { CloseLibrary(JavaScriptBase); CloseDevice((struct IORequest *)&timerReq); return 20; }

    /* Baseline: the same JS call made from the launching task (handed over each time). */
    JSValue fn = NULL;
    JS_Evaluate(task, (CONST_STRPTR)"(function(x){return x+1;})", -1, (CONST_STRPTR)"base.js", &fn);
    const long M = 1000;
    long okCount = 0;
    unsigned long long t0 = now();
    for (long i = 0; i < M; i++) {
        JSValue arg = JS_NewLong(task, i), r = NULL;
        if (JS_Call(task, fn, NULL, 1, &arg, &r) == JSERR_OK) {
            LONG v = 0;
            JS_ToLong(task, r, &v);
            if (v == i + 1) okCount++;
        }
        JS_Free(task, arg);
        if (r) JS_Free(task, r);
    }
    unsigned long long t1 = now();
    check("C -> JS calls from the launching task", okCount == M);
    timing("C -> JS call from the launching task (5 handovers each)", t0, t1, M);
    JS_Free(task, fn);

    t0 = now();
    for (long i = 0; i < M; i++) JS_Post(task, emptyJob, NULL, TRUE);
    t1 = now();
    timing("one handover: empty JS_Post, wait", t0, t1, M);

    struct Process *pr = (struct Process *)FindTask(NULL);
    struct GameArgs g;
    g.launcher = &pr->pr_Task;
    g.homeDir = GetProgramDir();
    g.currentDir = pr->pr_CurrentDir;
    LONG gameRc = JS_Post(task, gameMain, &g, TRUE);
    printf("INFO game job returned %ld\n", (long)gameRc);

    JS_EndTask(task);
    [mainPool release];
    CloseLibrary(JavaScriptBase);
    CloseDevice((struct IORequest *)&timerReq);
    printf("%s oolite_js_game_on_script_task_gate (%d failures)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 10 : 0;
}
