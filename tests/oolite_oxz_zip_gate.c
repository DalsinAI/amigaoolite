/* Original pinned Oolite MiniZip native 68040 ZIP read test. */
#include "unzip.h"
#include <stdio.h>
#include <string.h>
int main(void) {
    const char *path = "SYS:Oolite/fixture.oxz";
    unzFile f = unzOpen64(path);
    char name[128] = {0};
    char content[256] = {0};
    unz_file_info64 info = {0};
    int read = -1;
    int ok = 0;
    if (f) {
        if (unzGoToFirstFile(f) == UNZ_OK &&
            unzGetCurrentFileInfo64(f, &info, name, sizeof(name), NULL, 0, NULL, 0) == UNZ_OK &&
            strcmp(name, "manifest.plist") == 0 && info.uncompressed_size < sizeof(content) &&
            unzOpenCurrentFile(f) == UNZ_OK) {
            read = unzReadCurrentFile(f, content, sizeof(content) - 1);
            if (read > 0 && strstr(content, "org.oolite.test") != NULL) ok = 1;
            if (unzCloseCurrentFile(f) != UNZ_OK) ok = 0;
        }
        unzClose(f);
    }
    printf("OOLITE_OXZ_ZIP path=%s entry=%s bytes=%d status=%s\n", path, name, read, ok ? "PASS" : "FAIL");
    fflush(stdout);
    return ok ? 0 : 2;
}
