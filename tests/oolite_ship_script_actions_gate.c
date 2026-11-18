/* Actual pinned Oolite default ship script with native GFOS JavaScript host
 * callbacks for setup, launch, death and missile damage/detonation.
 */
#include "OOGFOSJavaScript.h"
#include <proto/javascript.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ShipActions {
    LONG legacyCalls, actionsSeen, damageCalls, explodeCalls;
    double damageAmount, damageRadius, damageFactor;
};
static JSValue legacy(JSTask task, APTR self, LONG argc, JSValue *argv, APTR data)
{
    struct ShipActions *s = (struct ShipActions *)self;
    LONG count = 0;
    (void)data;
    s->legacyCalls++;
    if (argc >= 2) {
        JSValue length = JS_GetProp(task, argv[1], (CONST_STRPTR)"length");
        if (length) {
            JS_ToLong(task, length, &count);
            JS_Free(task, length);
        }
        s->actionsSeen += count;
    }
    return JS_NewLong(task, count);
}
static JSValue damage(JSTask task, APTR self, LONG argc, JSValue *argv, APTR data)
{
    struct ShipActions *s = (struct ShipActions *)self;
    (void)data;
    s->damageCalls++;
    if (argc >= 3) {
        JS_ToDouble(task, argv[0], &s->damageAmount);
        JS_ToDouble(task, argv[1], &s->damageRadius);
        JS_ToDouble(task, argv[2], &s->damageFactor);
    }
    return JS_NewLong(task, 1);
}
static JSValue explode(JSTask task, APTR self, LONG argc, JSValue *argv, APTR data)
{
    struct ShipActions *s = (struct ShipActions *)self;
    (void)argc; (void)argv; (void)data;
    s->explodeCalls++;
    return JS_NewLong(task, 1);
}
static const struct JSHostSlot shipSlots[] = {
    { (CONST_STRPTR)"__runLegacyScriptActions", JSS_METHOD, 0, (APTR)legacy, NULL, 0, 2 },
    { (CONST_STRPTR)"dealEnergyDamage", JSS_METHOD, 0, (APTR)damage, NULL, 0, 3 },
    { (CONST_STRPTR)"explode", JSS_METHOD, 0, (APTR)explode, NULL, 0, 0 },
    { NULL, 0, 0, NULL, NULL, 0, 0 }
};
static const struct JSHostClass shipClass = {
    (CONST_STRPTR)"OoliteShipActions", NULL, shipSlots, NULL, NULL
};
static char *readFile(const char *path, long *length)
{
    FILE *f = fopen(path, "rb"); char *p = NULL;
    *length = 0;
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) == 0) *length = ftell(f);
    if (*length > 0 && *length < 262144 && fseek(f, 0, SEEK_SET) == 0) {
        p = malloc((size_t)*length+1);
        if (p && fread(p,1,(size_t)*length,f) == (size_t)*length) p[*length] = 0;
        else { free(p);p=NULL; }
    }
    fclose(f); return p;
}
int main(void)
{
    const char *path="SYS:Oolite/Scripts/oolite-default-ship-script.js";
    struct ShipActions s = {0}; struct OOGFOSJS js;
    long bytes = 0; char *source = readFile(path,&bytes);
    LONG rc = JSERR_BADARGS, removed = 0;
    int opened = 0, wrapped = 0, ok = 0;
    if (source && OOGFOSJSOpen(&js)) {
        opened = 1;
        wrapped = OOGFOSJSExposeHostClass(&js,&shipClass,&s,(CONST_STRPTR)"ship");
        if (wrapped) {
            rc = OOGFOSJSEvaluateSource(&js,
                (CONST_STRPTR)"this.system={mainStation:{}};this.legacy_setupActions=['initial','spawn'];this.legacy_launchActions=['launch'];this.legacy_deathActions=['death'];",
                (CONST_STRPTR)"oolite-setup.js");
            if (rc == JSERR_OK) rc = OOGFOSJSEvaluateSource(&js,
                                     (CONST_STRPTR)source,(CONST_STRPTR)path);
            if (rc == JSERR_OK) rc = OOGFOSJSEvaluateLong(&js,
                (CONST_STRPTR)"this.shipSpawned();this.shipDied();this.ooliteDetonateAsStandardMissile();typeof this.shipSpawned === 'undefined' ? 1 : 0",
                (CONST_STRPTR)"oolite-events.js", &removed);
        }
        if (rc != JSERR_OK) {
            char e[192] = {0}; OOGFOSJSErrorText(&js,(STRPTR)e,sizeof e);
            printf("OOLITE_SHIP_ACTIONS_ERROR text=%s\n",e);
        }
        OOGFOSJSClose(&js);
    }
    ok = source && opened && wrapped && rc == JSERR_OK && removed == 1 &&
         s.legacyCalls == 3 && s.actionsSeen == 4 &&
         s.damageCalls == 1 && s.explodeCalls == 1 &&
         s.damageAmount == 170.0 && s.damageRadius == 32.5 && s.damageFactor == 0.25;
    printf("OOLITE_SHIP_ACTIONS bytes=%ld legacy=%ld actions=%ld damage=%ld explode=%ld amount=%ld radius_x2=%ld factor_x4=%ld removed=%ld status=%s\n",
          bytes,(long)s.legacyCalls,(long)s.actionsSeen,(long)s.damageCalls,
          (long)s.explodeCalls,(long)s.damageAmount,(long)(s.damageRadius*2),(long)(s.damageFactor*4),
          (long)removed,ok?"PASS":"FAIL"); fflush(stdout);
    free(source);return ok?0:2;
}
