/* Real Oolite ShipEntity scalar JS property selectors, game-task only.
 * Intentionally imports upstream Objective-C headers without NDK exec/types.h:
 * GNU objc BOOL is 8-bit, Amiga NDK BOOL is 16-bit. GPL-2.0-or-later.
 */
/* Upstream ShipEntity.h presently imports OOJSPropID.h -> SpiderMonkey jsapi.h.
 * Project ONLY the exact selectors this bridge uses, with real OOTypes.h;
 * do not fabricate jsapi.h or link a second runtime. Match the pinned
 * ShipEntity.h / OOJSShip.m declarations before each upstream update. */
#import "OOTypes.h"
@interface OOShipGroup : NSObject
- (NSUInteger)count;
@end
@interface ShipEntity : NSObject
- (OOCreditsQuantity)bounty;
- (void)setBounty:(OOCreditsQuantity)amount withReason:(OOLegalStatusReason)reason;
- (OOSystemID)homeSystem;
- (void)setHomeSystem:(OOSystemID)system;
- (uint8_t)maxEscortCount;
- (void)setMaxEscortCount:(uint8_t)newCount;
- (OOShipGroup *)escortGroup;
- (BOOL)isPlayer;
/* Exact pinned Entity selectors; GLfloat is single-precision on 68k. */
- (float)energy;
- (void)setEnergy:(float)value;
- (float)maxEnergy;
- (void)setMaxEnergy:(float)value;
@end
#define MAX_ESCORTS 16
#include "OOGFOSShipEntitySelectors.h"
#include <limits.h>
#include <math.h>
#include <string.h>

void *OOGFOSShipObjCRetain(void *ship)
{
    return ship ? [(id)ship retain] : (void *)0;
}
void OOGFOSShipObjCRelease(void *ship)
{
    if (ship) [(id)ship release];
}
int OOGFOSShipObjCGetLong(void *raw,unsigned property,int32_t *out)
{
    ShipEntity *ship=(ShipEntity *)raw;
    if(!ship || !out)return 0;
    switch(property) {
    case OOGFOS_OOJS_BOUNTY: {
        OOCreditsQuantity bounty=[ship bounty];
        if(bounty>(OOCreditsQuantity)INT32_MAX)return 0;
        *out=(int32_t)bounty;
        return 1;
    }
    case OOGFOS_OOJS_HOME_SYSTEM:*out=(int32_t)[ship homeSystem];return 1;
    case OOGFOS_OOJS_MAX_ESCORTS:*out=(int32_t)[ship maxEscortCount];return 1;
    case OOGFOS_OOJS_ENERGY:
    case OOGFOS_OOJS_MAX_ENERGY: {
        float value=property==OOGFOS_OOJS_ENERGY ?
                    [ship energy] : [ship maxEnergy];
        _Static_assert(sizeof(float)==sizeof(int32_t),"Float scalar ABI must be 32-bit");
        memcpy(out,&value,sizeof(value));
        return 1;
    }
    default:return 0;
    }
}
int OOGFOSShipObjCSetLong(void *raw,unsigned property,int32_t value)
{
    ShipEntity *ship=(ShipEntity *)raw;
    if(!ship)return 0;
    switch(property) {
    case OOGFOS_OOJS_BOUNTY:
        [ship setBounty:(OOCreditsQuantity)(value<0 ? 0 : value)
              withReason:kOOLegalStatusReasonByScript];
        return 1;
    case OOGFOS_OOJS_HOME_SYSTEM:
        [ship setHomeSystem:(OOSystemID)(value<0 ? 0 : value)];
        return 1;
    case OOGFOS_OOJS_ENERGY:
    case OOGFOS_OOJS_MAX_ENERGY: {
        float decoded;
        memcpy(&decoded,&value,sizeof(decoded));
        if(!isfinite(decoded)) return 0;
        if(property==OOGFOS_OOJS_MAX_ENERGY) {
            if(decoded<=0.0f) return 0; /* OOJSEntity set maxEnergy */
            [ship setMaxEnergy:decoded];
        } else {
            float cap=[ship maxEnergy];
            if(!isfinite(cap) || cap<0.0f) return 0;
            if(decoded<0.0f) decoded=0.0f;
            if(decoded>cap) decoded=cap;
            [ship setEnergy:decoded];
        }
        return 1;
    }
    case OOGFOS_OOJS_MAX_ESCORTS: {
        NSInteger current=[[ship escortGroup] count]-1;
        if([ship isPlayer] || value<current || value>MAX_ESCORTS || value<0)
            return 0;
        [ship setMaxEscortCount:(uint8_t)value];
        return 1;
    }
    default:return 0;
    }
}
