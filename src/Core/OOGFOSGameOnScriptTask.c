/*
 * The game on javascript.library's script task (Team 5, 19 Nov 2026).
 *
 * main() opens javascript.library v2, makes one JSTask and posts the whole of
 * Oolite's own main() (renamed OOGameMain by patches/oolite/0016) to it as one
 * job, waiting for it. Everything the game does then runs on the script task,
 * so every JSAPI call through gfosjsapi is a direct call there
 * (tests/oolite_js_game_on_script_task_gate.m proved the pieces on AC090).
 *
 * - The script task takes over libnix's pthread slot 0 (the main thread) for
 *   the length of the job: pthread_self() is 0 only for the task named there,
 *   and GNUstep must see one main thread. Needs libpthread-gfos.a
 *   (build/gfos-runtime.mk: __threads made global).
 * - It gets the launcher's current and program drawers, so PROGDIR: and
 *   relative paths mean the same as in the launcher.
 * - exit() is linked with -Wl,--wrap=exit. Called on the script task (Oolite
 *   quits with exit(0) from GameController), it returns to the launcher
 *   through the job instead of running the C library's exit on the wrong
 *   task; the launcher then exits with that code.
 * - getcwd() is linked with -Wl,--wrap=getcwd: libnix answers it from the
 *   Shell's (CLI's) current drawer name, and the script task has no CLI, so
 *   NSFileManager's currentDirectoryPath was nil there and Oolite looked for
 *   its Resources at "(null)". Here it is the name of the task's current
 *   drawer lock (NameFromLock), which is right on any process.
 *
 * MIT, AmiCradle Team.
 */
#include <exec/types.h>
#include <exec/tasks.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/javascript.h>
#include <errno.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "OOGFOSProcess.h"
#include "gfosjsapi/oojsl.h"

struct Library *JavaScriptBase = NULL;

/* Oolite's own main(), renamed by patches/oolite/0016-gfos-main-on-script-task. */
extern int OOGameMain(int argc, char *argv[]);

/* libnix pthreads (see the gate): slot 0's Task pointer is at offset 20. */
extern void **_threads;
#define OOGFOS_PTHREAD_TASK_OFFSET 20

#ifndef OOGFOS_SCRIPT_STACK
#define OOGFOS_SCRIPT_STACK (2 * 1024 * 1024)
#endif

struct OOGFOSGameJob {
    int argc;
    char **argv;
    BPTR homeDir;
    BPTR currentDir;
    int result;
};

static struct Task *sScriptTask;
static jmp_buf sExitJump;
static int sExitCode;

void __real_exit(int code) __attribute__((noreturn));

void __wrap_exit(int code)
{
    if (sScriptTask && FindTask(NULL) == sScriptTask) {
        sExitCode = code;
        fflush(NULL);
        longjmp(sExitJump, 1);
    }
    __real_exit(code);
}

char *__wrap_getcwd(char *buf, size_t size)
{
    struct Process *me = (struct Process *)FindTask(NULL);
    BPTR lock = me->pr_Task.tc_Node.ln_Type == NT_PROCESS ? me->pr_CurrentDir : 0;
    int mine = 0;
    if (!buf) {
        if (size == 0) size = 1024;
        buf = (char *)malloc(size);
        if (!buf) { errno = ENOMEM; return NULL; }
        mine = 1;
    }
    if (size == 0) { errno = EINVAL; return NULL; }
    if (!lock) {
        if (size < 5) { if (mine) free(buf); errno = ERANGE; return NULL; }
        strcpy(buf, "SYS:");
        return buf;
    }
    if (!NameFromLock(lock, (STRPTR)buf, (LONG)size)) {
        if (mine) free(buf);
        errno = IoErr() == ERROR_LINE_TOO_LONG ? ERANGE : EACCES;
        return NULL;
    }
    return buf;
}

static struct Task **mainThreadSlot(void)
{
    return (struct Task **)((char *)_threads[0] + OOGFOS_PTHREAD_TASK_OFFSET);
}

static LONG gameJob(JSTask task, APTR data)
{
    struct OOGFOSGameJob *job = (struct OOGFOSGameJob *)data;
    struct Task *me = FindTask(NULL);
    struct Task *previousMain;
    BPTR oldDir, oldHome = 0;
    (void)task;

    oldDir = CurrentDir(job->currentDir);
    if (me->tc_Node.ln_Type == NT_PROCESS)
        oldHome = SetProgramDir(job->homeDir);

    Forbid();
    previousMain = *mainThreadSlot();
    *mainThreadSlot() = me;
    Permit();

    sScriptTask = me;
    if (setjmp(sExitJump) == 0)
        job->result = OOGameMain(job->argc, job->argv);
    else
        job->result = sExitCode;
    sScriptTask = NULL;

    Forbid();
    *mainThreadSlot() = previousMain;
    Permit();

    if (oldHome)
        SetProgramDir(oldHome);
    CurrentDir(oldDir);
    return 0;
}

int main(int argc, char *argv[])
{
    struct TagItem tags[] = {
        { JSA_StackSize, OOGFOS_SCRIPT_STACK },
        { JSA_Name, (ULONG)"Oolite" },
        { TAG_DONE, 0 }
    };
    struct OOGFOSGameJob job;
    JSTask task;

    argv = OOGFOSAbsoluteArgv(argc, argv);

    JavaScriptBase = OpenLibrary((CONST_STRPTR)JAVASCRIPTNAME, 2);
    if (!JavaScriptBase) {
        fprintf(stderr, "Oolite needs javascript.library 2 or newer in LIBS:\n");
        return 20;
    }
    task = JS_NewTask(tags);
    if (!task) {
        fprintf(stderr, "Oolite: javascript.library could not start its script task\n");
        CloseLibrary(JavaScriptBase);
        return 20;
    }
    ooL_SetTask(task);

    job.argc = argc;
    job.argv = argv;
    job.homeDir = GetProgramDir();
    job.currentDir = ((struct Process *)FindTask(NULL))->pr_CurrentDir;
    job.result = EXIT_FAILURE;
    JS_Post(task, gameJob, &job, TRUE);

    JS_EndTask(task);
    CloseLibrary(JavaScriptBase);
    JavaScriptBase = NULL;
    fflush(NULL);
    return job.result;
}
