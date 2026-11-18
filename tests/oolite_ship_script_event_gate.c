/* Run actual unmodified Oolite default ship script + legacy event handler
 * against canonical GFOS shared JavaScriptCore host-class ABI.
 */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ShipProbe { LONG legacyCalls; LONG itemsSeen; };
static JSValue callLegacyActions(JSTask task, APTR self, LONG argc,
                                 JSValue *argv, APTR data)
{
    struct ShipProbe *ship = (struct ShipProbe *)self;
    JSValue length;
    LONG count = 0;
    (void)data;
    ship->legacyCalls++;
    if (argc >= 2) {
        length = JS_GetProp(task, argv[1], (CONST_STRPTR)"length");
        if (length) {
            JS_ToLong(task, length, &count);
            JS_Free(task, length);
        }
        ship->itemsSeen += count;
    }
    return JS_NewLong(task, count);
}
static const struct JSHostSlot shipSlots[] = {
    { (CONST_STRPTR)"__runLegacyScriptActions", JSS_METHOD, 0,
      (APTR)callLegacyActions, NULL, 0, 2 },
    { NULL, 0, 0, NULL, NULL, 0, 0 }
};
static const struct JSHostClass shipClass = {
    (CONST_STRPTR)"OoliteShipLegacy", NULL, shipSlots, NULL, NULL
};
static int loadScript(const char *path, char **out)
{
    FILE *f = fopen(path, "rb");
    long length = -1;
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) == 0) length = ftell(f);
    if (length <= 0 || length >= 262144 || fseek(f, 0, SEEK_SET)) {
        fclose(f); return 0;
    }
    *out = malloc((size_t)length+1);
    if (!*out || fread(*out, 1, (size_t)length, f) != (size_t)length) {
        free(*out); *out = NULL; fclose(f); return 0;
    }
    (*out)[length] = 0;
    fclose(f); return (int)length;
}
int main(void)
{
    const char *file = "SYS:Oolite/Scripts/oolite-default-ship-script.js";
    struct ShipProbe ship = { 0, 0 };
    struct OOGFOSJS js;
    char *source = NULL;
    int bytes = loadScript(file, &source), opened = 0, wrapped = 0;
    LONG rc = JSERR_BADARGS, result = -1;
    int pass = 0;
    if (bytes && OOGFOSJSOpen(&js)) {
        opened = 1;
        wrapped = OOGFOSJSExposeHostClass(&js, &shipClass, &ship, (CONST_STRPTR)"ship");
        if (wrapped) {
            rc = OOGFOSJSEvaluateSource(&js,
               (CONST_STRPTR)"this.system={mainStation:{}};this.legacy_launchActions=['launch'];",
               (CONST_STRPTR)"ship-event-setup.js");
            if (rc == JSERR_OK) rc = OOGFOSJSEvaluateSource(&js,
                       (CONST_STRPTR)source, (CONST_STRPTR)file);
            if (rc == JSERR_OK) rc = OOGFOSJSEvaluateLong(&js,
               (CONST_STRPTR)"typeof shipSpawned === 'function' ? 1 : 0",
               (CONST_STRPTR)"ship-handler-pre.js", &result);
            if (rc == JSERR_OK && result == 1) rc = OOGFOSJSEvaluateLong(&js,
               (CONST_STRPTR)"this.shipSpawned();typeof this.shipSpawned === 'undefined' ? 1 : 0",
               (CONST_STRPTR)"ship-handler-call.js", &result);
        }
        if (rc != JSERR_OK) {
            char error[256] = {0};
            OOGFOSJSErrorText(&js, (STRPTR)error, sizeof error);
            printf("OOLITE_SHIP_EVENT_ERROR %s\n", error);
        }
        OOGFOSJSClose(&js);
    }
    pass = opened && wrapped && rc == JSERR_OK && result == 1 &&
           ship.legacyCalls == 1 && ship.itemsSeen == 1;
    printf("OOLITE_SHIP_EVENT bytes=%d opened=%d wrapped=%d calls=%ld actions=%ld removed=%ld rc=%ld status=%s\n",
           bytes, opened, wrapped, (long)ship.legacyCalls, (long)ship.itemsSeen,
           (long)result, (long)rc, pass ? "PASS" : "FAIL");
    free(source); fflush(stdout);
    return pass ? 0 : 2;
}
