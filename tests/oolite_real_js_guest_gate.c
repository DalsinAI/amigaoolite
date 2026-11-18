/* Load unmodified Oolite upstream JavaScript scripts from AmigaDOS,
 * execute via canonical GFOS javascript.library, check script exports.
 * Upstream scripts are staged separately, not vendored into this port.
 */
#include "OOGFOSJavaScript.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int testScript(const char *name)
{
    char path[256], expression[256];
    struct OOGFOSJS js;
    FILE *f;
    long length = -1;
    char *buffer = NULL;
    LONG result = -1, code = JSERR_BADARGS;
    int opened = 0, ok = 0;
    snprintf(path, sizeof path, "SYS:Oolite/Scripts/%s.js", name);
    f = fopen(path, "rb");
    if (f) {
        if (fseek(f, 0, SEEK_END) == 0) length = ftell(f);
        if (length > 0 && length < 262144 && fseek(f, 0, SEEK_SET) == 0) {
            buffer = malloc((size_t)length + 1);
            if (buffer && fread(buffer, 1, (size_t)length, f) == (size_t)length)
                buffer[length] = 0;
            else { free(buffer); buffer = NULL; }
        }
        fclose(f);
    }
    if (buffer) {
        opened = OOGFOSJSOpen(&js);
        if (opened) {
            code = OOGFOSJSEvaluateSource(&js, (CONST_STRPTR)buffer, (CONST_STRPTR)path);
            if (code == JSERR_OK) {
                snprintf(expression, sizeof expression, "this.name === '%s' ? 1 : 0", name);
                code = OOGFOSJSEvaluateLong(&js, (CONST_STRPTR)expression,
                                              (CONST_STRPTR)"oolite-script-name.js", &result);
            }
            if (code != JSERR_OK) {
                char message[256] = {0};
                OOGFOSJSErrorText(&js, (STRPTR)message, (LONG)sizeof message);
                printf("OOLITE_REAL_JS_ERROR name=%s text=%s\n", name, message);
            }
            OOGFOSJSClose(&js);
        }
    }
    ok = buffer && opened && code == JSERR_OK && result == 1;
    printf("OOLITE_REAL_JS name=%s bytes=%ld opened=%d rc=%ld property=%ld status=%s\n",
           name, length, opened, (long)code, (long)result, ok ? "PASS" : "FAIL");
    fflush(stdout);
    free(buffer);
    return ok;
}
int main(void)
{
    int a = testScript("oolite-default-player-script");
    int b = testScript("oolite-default-effect-script");
    printf("OOLITE_REAL_JS_TOTAL passed=%d total=2 status=%s\n",
           a + b, a && b ? "PASS" : "FAIL");
    return a && b ? 0 : 2;
}
