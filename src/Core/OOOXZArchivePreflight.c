/* GFOS Oolite OXZ archive preflight. GPL-2.0-or-later. */
#include "OOOXZArchivePreflight.h"
#include "OOOXZPathSafety.h"
#include "unzip.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* First-stage limits, not yet a full OXZ installation policy. */
#define GFOS_OXZ_MAX_ENTRIES 8192u
#define GFOS_OXZ_MAX_UNPACKED (256u * 1024u * 1024u)

static int equalFolded(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++;b++;
    }
    return *a == *b;
}
int OOGFOSPreflightOXZ(const char *path, unsigned *entries) {
    unzFile zip;
    uint64_t total = 0;
    unsigned count = 0;
    unsigned i;
    char *names = NULL;
    int rc, ok = 0;
    if (entries) *entries = 0;
    if (!path) return 0;
    zip = unzOpen64(path);
    if (!zip) return 0;
    names = calloc(GFOS_OXZ_MAX_ENTRIES, 512);
    if (!names) goto done;
    rc = unzGoToFirstFile(zip);
    if (rc != UNZ_OK) goto done;
    do {
        unz_file_info64 fi = {0};
        char *name;
        unsigned long type;
        if (count >= GFOS_OXZ_MAX_ENTRIES) goto done;
        name = names + (size_t)count * 512;
        if (unzGetCurrentFileInfo64(zip,&fi,name,512,NULL,0,NULL,0) != UNZ_OK) goto done;
        if (!OOGFOSSafeOXZEntry(name,(size_t)fi.size_filename)) goto done;
        name[fi.size_filename] = 0;
        for (i=0;i<count;i++) if (equalFolded(name,names+(size_t)i*512)) goto done;
        /* Disallow Unix symlink entries; extraction must not follow external targets. */
        type=(fi.external_fa >> 16) & 0170000u;
        if (type == 0120000u) goto done;
        if (fi.uncompressed_size > (uint64_t)GFOS_OXZ_MAX_UNPACKED - total) goto done;
        total += fi.uncompressed_size;
        count++;
        rc = unzGoToNextFile(zip);
    } while (rc == UNZ_OK);
    if (rc == UNZ_END_OF_LIST_OF_FILE) {
        ok=1;
        if (entries) *entries=count;
    }
done:
    free(names);
    unzClose(zip);
    return ok;
}
