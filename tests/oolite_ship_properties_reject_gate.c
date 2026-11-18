/* Validate rejected writes leave C ship unchanged and stale reads fail safely. */
#include "OOGFOSJavaScript.h"
#include "OOGFOSShipProperties.h"
#include <stdio.h>
struct TestShip { LONG bounty,home,escorts; LONG writes; };
static int getValue(void *ptr,unsigned key,LONG *value) {
    struct TestShip *ship=(struct TestShip *)ptr;
    if (!ship || !value) return 0;
    switch(key) {
    case OOGFOS_SHIP_BOUNTY: *value=ship->bounty;return 1;
    case OOGFOS_SHIP_HOME_SYSTEM: *value=ship->home;return 1;
    case OOGFOS_SHIP_MAX_ESCORTS: *value=ship->escorts;return 1;
    default: return 0;
    }
}
static int setValue(void *ptr,unsigned key,LONG value) {
    struct TestShip *ship=(struct TestShip *)ptr;
    if (!ship) return 0;
    switch(key) {
    case OOGFOS_SHIP_BOUNTY: if(value<0 || value>100000)return 0;ship->bounty=value;break;
    case OOGFOS_SHIP_HOME_SYSTEM: if(value<0 || value>255)return 0;ship->home=value;break;
    case OOGFOS_SHIP_MAX_ESCORTS: if(value<0 || value>16)return 0;ship->escorts=value;break;
    default:return 0;
    }
    ship->writes++;return 1;
}
int main(void) {
    static const struct OOGFOSShipBackend callbacks={getValue,setValue};
    struct TestShip ship={100,7,2,0};
    struct OOGFOSShipHandle handle={&ship,&callbacks,1};
    struct OOGFOSJS js;
    LONG checks=-1, staleCheck=-1,rc=JSERR_BADARGS,staleRC=JSERR_BADARGS;
    int exposed=0,pass=0;
    if (!OOGFOSJSOpen(&js)) return 77;
    exposed=OOGFOSJSExposeHostClass(&js,OOGFOSShipClass(),&handle,(CONST_STRPTR)"ship");
    if (exposed) {
        rc=OOGFOSJSEvaluateLong(&js,
            (CONST_STRPTR)"(function(){var bad=0;try{ship.homeSystem=256;}catch(e){bad++;}try{ship.bounty=-1;}catch(e){bad++;}try{ship.maxEscorts=17;}catch(e){bad++;}return bad;})()",
            (CONST_STRPTR)"oolite-property-reject.js",&checks);
        OOGFOSShipInvalidate(&handle);
        staleRC=OOGFOSJSEvaluateLong(&js,
            (CONST_STRPTR)"(function(){var bad=0;try{ship.homeSystem=12;}catch(e){bad++;}try{var x=ship.maxEscorts;}catch(e){bad++;}return bad;})()",
            (CONST_STRPTR)"oolite-property-stale.js",&staleCheck);
    }
    pass=exposed && rc==JSERR_OK && checks==3 && staleRC==JSERR_OK && staleCheck==2 &&
         ship.home==7 && ship.bounty==100 && ship.escorts==2 && ship.writes==0;
    printf("OOLITE_SHIP_REJECT bound=%d invalid=%ld stale=%ld writes=%ld unchanged=%d status=%s\n",
           exposed,(long)checks,(long)staleCheck,(long)ship.writes,
           ship.home==7 && ship.bounty==100 && ship.escorts==2,
           pass?"PASS":"FAIL");fflush(stdout);
    OOGFOSJSClose(&js);
    return pass?0:2;
}
