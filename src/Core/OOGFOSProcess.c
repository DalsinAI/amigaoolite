/* What GNUstep needs to know about a GFOS process before it starts.
 *
 * GNUstep finds its own executable from argv[0] (and the bundle, info
 * dictionary and user defaults from that). AmigaOS gives a bare command
 * name, and GNUstep has no launch directory to resolve it against, so
 * NSBundle asserted inside +[NSUserDefaults initialize]; the assertion's
 * NSLog then needed the time zone and the user defaults again, and the
 * process hung. OOGFOSAbsoluteArgv() gives argv[0] as the full path of the
 * program file (PROGDIR: resolved, plus the program's own name), for
 * GSInitializeProcess.
 *
 * Team 5, 19 Nov 2026. MIT.
 */
#include <exec/types.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>

#include "OOGFOSProcess.h"

static char oogfosArgv0[512];

char **OOGFOSAbsoluteArgv(int argc, char **argv)
{
    char name[256];
    BPTR home = GetProgramDir();
    if (argc < 1 || !argv || !home)
        return argv;
    if (!NameFromLock(home, (STRPTR)oogfosArgv0, sizeof oogfosArgv0))
        return argv;
    if (!GetProgramName((STRPTR)name, sizeof name) || !name[0])
        strncpy(name, argv[0], sizeof name - 1), name[sizeof name - 1] = 0;
    if (!AddPart((STRPTR)oogfosArgv0, FilePart((STRPTR)name), sizeof oogfosArgv0))
        return argv;
    argv[0] = oogfosArgv0;
    return argv;
}
