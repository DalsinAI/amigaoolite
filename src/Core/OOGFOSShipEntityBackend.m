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
@end
#define MAX_ESCORTS 16
#include "OOGFOSShipEntitySelectors.h"
#include <limits.h>

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
