/* Native ObjC selector fixture, matching pinned ShipEntity method signatures.
 * This is a test fixture, NOT upstream ShipEntity implementation. */
#import "OOTypes.h"
#import "OOGFOSShipEntitySelectors.h"
@interface OOShipGroup : NSObject
- (NSUInteger)count;
@end
@implementation OOShipGroup
- (NSUInteger)count { return 1; }
@end
@interface ShipEntity : NSObject {
    OOCreditsQuantity _bounty;
    OOSystemID _home;
    uint8_t _maxEscorts;
    OOLegalStatusReason _reason;
    OOShipGroup *_group;
}
- (OOCreditsQuantity)bounty;
- (void)setBounty:(OOCreditsQuantity)amount withReason:(OOLegalStatusReason)reason;
- (OOSystemID)homeSystem;
- (void)setHomeSystem:(OOSystemID)system;
- (uint8_t)maxEscortCount;
- (void)setMaxEscortCount:(uint8_t)newCount;
- (OOShipGroup *)escortGroup;
- (BOOL)isPlayer;
@end
@implementation ShipEntity
- (id)init { self=[super init];if(self){_bounty=10;_home=7;_maxEscorts=2;_group=[[OOShipGroup alloc]init];}return self; }
- (void)dealloc {[_group release];[super dealloc];}
- (OOCreditsQuantity)bounty{return _bounty;}
- (void)setBounty:(OOCreditsQuantity)amount withReason:(OOLegalStatusReason)reason{_bounty=amount;_reason=reason;}
- (OOSystemID)homeSystem{return _home;}
- (void)setHomeSystem:(OOSystemID)system{_home=system;}
- (uint8_t)maxEscortCount{return _maxEscorts;}
- (void)setMaxEscortCount:(uint8_t)value{_maxEscorts=value;}
- (OOShipGroup *)escortGroup{return _group;}
- (BOOL)isPlayer{return NO;}
@end
void *OOGFOSTestObjCShipNew(void){return [[ShipEntity alloc]init];}
void OOGFOSTestObjCShipFree(void *raw){[(id)raw release];}
int OOGFOSTestObjCShipValues(void *raw,int *bounty,int *home,int *escorts,int *reason){
 ShipEntity *s=(ShipEntity *)raw;
 if(!s||!bounty||!home||!escorts||!reason)return 0;
 *bounty=(int)[s bounty];*home=[s homeSystem];*escorts=[s maxEscortCount];*reason=s->_reason;
 return 1;
}
int OOGFOSTestObjCReasonByScript(void){return kOOLegalStatusReasonByScript;}
