/* The one user GFOS has, for GNUstep Foundation.
 *
 * GNUstep finds the user name and home directory (and so the user defaults
 * database) through getpwuid()/getpwnam(). In libnix those come from
 * libsocket, which asks the TCP/IP stack's user database; GFOS has none, so
 * GNUstep got no user, no home, and NSUserDefaults, NSLog and NSException
 * stalled. Linked ahead of libsocket, these answer for the single local user:
 * the name from $USER (default "amiga"), the home from $HOME (default
 * PROGDIR:, the game's own drawer).
 *
 * Team 5, 19 Nov 2026. MIT.
 */
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* Not 0: libnix says every program is uid 0, and GNUstep then takes any user
 * not named "root" for a setuid program and chowns each file and directory it
 * makes, which fails on AmigaOS (createDirectoryAtPath: returned NO). */
#define OOGFOS_UID 1

uid_t getuid(void) { return OOGFOS_UID; }
uid_t geteuid(void) { return OOGFOS_UID; }
gid_t getgid(void) { return OOGFOS_UID; }
gid_t getegid(void) { return OOGFOS_UID; }

static struct passwd oogfosUser;
static char oogfosName[64];
static char oogfosHome[256];

static struct passwd *oogfosFill(void)
{
    const char *name = getenv("USER");
    const char *home = getenv("HOME");
    strncpy(oogfosName, name && *name ? name : "amiga", sizeof oogfosName - 1);
    strncpy(oogfosHome, home && *home ? home : "PROGDIR:", sizeof oogfosHome - 1);
    memset(&oogfosUser, 0, sizeof oogfosUser);
    oogfosUser.pw_name = oogfosName;
    oogfosUser.pw_passwd = (char *)"";
    oogfosUser.pw_uid = OOGFOS_UID;
    oogfosUser.pw_gid = OOGFOS_UID;
    oogfosUser.pw_gecos = oogfosName;
    oogfosUser.pw_dir = oogfosHome;
    oogfosUser.pw_shell = (char *)"";
    return &oogfosUser;
}

struct passwd *getpwuid(uid_t uid)
{
    (void)uid;
    return oogfosFill();
}

struct passwd *getpwnam(const char *name)
{
    struct passwd *p = oogfosFill();
    if (name && *name && strcmp(name, p->pw_name) != 0)
        return NULL;
    return p;
}

/* The group database has the same problem (libsocket's getgrent.o):
 * GNUstep asks for the group name of every file whose attributes it
 * copies, so NSUserDefaults hung after writing its plist. One group,
 * named like the user. */
static struct group oogfosGroup;
static char *oogfosGroupMembers[2];

static struct group *oogfosGroupFill(void)
{
    struct passwd *p = oogfosFill();
    memset(&oogfosGroup, 0, sizeof oogfosGroup);
    oogfosGroupMembers[0] = p->pw_name;
    oogfosGroupMembers[1] = NULL;
    oogfosGroup.gr_name = p->pw_name;
    oogfosGroup.gr_passwd = (char *)"";
    oogfosGroup.gr_gid = OOGFOS_UID;
    oogfosGroup.gr_mem = oogfosGroupMembers;
    return &oogfosGroup;
}

struct group *getgrgid(gid_t gid)
{
    (void)gid;
    return oogfosGroupFill();
}

struct group *getgrnam(const char *name)
{
    struct group *g = oogfosGroupFill();
    if (name && *name && strcmp(name, g->gr_name) != 0)
        return NULL;
    return g;
}
