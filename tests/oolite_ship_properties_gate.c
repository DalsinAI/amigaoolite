/* Qualified OOJSShip numeric property semantics, through GFOS JS accessors.
 * The stand-in backend is NOT Oolite's ShipEntity. */
#include "OOGFOSJavaScript.h"
#include "OOGFOSShipProperties.h"
#include <stdio.h>
struct TestShip { LONG bounty, homeSystem, maxEscorts; unsigned updates; };
static int readShip(void *self, unsigned key, LONG *out)
{
    struct TestShip *s=(struct TestShip *)self;
    if (!s || !out) return 0;
    switch (key) {
    case OOGFOS_SHIP_BOUNTY: *out=s->bounty;return 1;
    case OOGFOS_SHIP_HOME_SYSTEM: *out=s->homeSystem;return 1;
    case OOGFOS_SHIP_MAX_ESCORTS: *out=s->maxEscorts;return 1;
    default:return 0;
    }
}
static int writeShip(void *self, unsigned key, LONG value)
{
    struct TestShip *s=(struct TestShip *)self;
    if (!s) return 0;
    switch (key) {
    case OOGFOS_SHIP_BOUNTY: if (value<0 || value>100000) return 0; s->bounty=value;break;
    case OOGFOS_SHIP_HOME_SYSTEM: if (value<0 || value>255) return 0; s->homeSystem=value;break;
    case OOGFOS_SHIP_MAX_ESCORTS: if (value<0 || value>16) return 0; s->maxEscorts=value;break;
    default:return 0;
    }
    ++s->updates;return 1;
}
int main(void)
{
    struct TestShip ship={10,7,2,0};
    static const struct OOGFOSShipBackend callbacks={readShip,writeShip};
    struct OOGFOSShipHandle handle={&ship,&callbacks,1};
    struct OOGFOSJS js;
    LONG result=-1,stale=-1,rc=JSERR_BADARGS,staleRC=JSERR_BADARGS;
    int bound=0,passed=0;
    if (!OOGFOSJSOpen(&js)) {puts("OOLITE_SHIP_PROPERTIES status=UNAVAILABLE");return 77;}
    bound=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,(CONST_STRPTR)"ship");
    if (bound) {
        rc=OOGFOSJSEvaluateLong(&js,
           (CONST_STRPTR)"(ship.bounty===10 && ship.homeSystem===7 && ship.maxEscorts===2) ? (ship.bounty=44,ship.homeSystem=42,ship.maxEscorts=4,ship.bounty+ship.homeSystem+ship.maxEscorts) : 0",
           (CONST_STRPTR)"oolite-ship-properties.js",&result);
        OOGFOSShipInvalidate(&handle);
        staleRC=OOGFOSJSEvaluateLong(&js,
          (CONST_STRPTR)"(function(){try {return ship.bounty;} catch(e){return 1;}})()",
          (CONST_STRPTR)"oolite-ship-stale.js",&stale);
    }
    passed=bound && rc==JSERR_OK && result==90 &&
        ship.bounty==44 && ship.homeSystem==42 && ship.maxEscorts==4 &&
        ship.updates==3 && staleRC==JSERR_OK && stale==1;
    printf("OOLITE_SHIP_PROPERTIES bound=%d bounty=%ld home=%ld escorts=%ld writes=%u sum=%ld stale_rc=%ld stale=%ld status=%s\n",
        bound,(long)ship.bounty,(long)ship.homeSystem,(long)ship.maxEscorts,
        ship.updates,(long)result,(long)staleRC,(long)stale,passed?"PASS":"FAIL");fflush(stdout);
    OOGFOSJSClose(&js);
    return passed?0:2;
}
