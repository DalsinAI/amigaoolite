/* Native ObjC selector fixture, matching pinned ShipEntity method signatures.
 * This is a test fixture, NOT upstream ShipEntity implementation. */
#import "OOTypes.h"
#import "OOGFOSShipEntitySelectors.h"
@interface OOShipGroup : NSObject { NSUInteger _count; }
- (NSUInteger)count;
- (void)setTestCount:(NSUInteger)count;
@end
@implementation OOShipGroup
- (id)init {self=[super init];if(self)_count=1;return self;}
- (NSUInteger)count { return _count; }
- (void)setTestCount:(NSUInteger)count {_count=count;}
@end
@interface ShipEntity : NSObject {
    OOCreditsQuantity _bounty;
    OOSystemID _home;
    uint8_t _maxEscorts;
    OOLegalStatusReason _reason;
    OOShipGroup *_group;
    BOOL _testPlayer;
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
static unsigned long testShipDeallocations=0;
@implementation ShipEntity
- (id)init { self=[super init];if(self){_bounty=10;_home=7;_maxEscorts=2;_group=[[OOShipGroup alloc]init];}return self; }
- (void)dealloc {testShipDeallocations++;[_group release];[super dealloc];}
- (OOCreditsQuantity)bounty{return _bounty;}
- (void)setBounty:(OOCreditsQuantity)amount withReason:(OOLegalStatusReason)reason{_bounty=amount;_reason=reason;}
- (OOSystemID)homeSystem{return _home;}
- (void)setHomeSystem:(OOSystemID)system{_home=system;}
- (uint8_t)maxEscortCount{return _maxEscorts;}
- (void)setMaxEscortCount:(uint8_t)value{_maxEscorts=value;}
- (OOShipGroup *)escortGroup{return _group;}
- (BOOL)isPlayer{return _testPlayer;}
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

/* Controlled test-only knobs. These are never part of Oolite's real API. */
void OOGFOSTestObjCShipSetPlayer(void *raw,int value){ ((ShipEntity *)raw)->_testPlayer=(value!=0); }
void OOGFOSTestObjCShipSetGroupSize(void *raw,unsigned value){ [((ShipEntity *)raw)->_group setTestCount:value]; }
void OOGFOSTestObjCShipSetBounty64(void *raw,uint64_t value){ ((ShipEntity *)raw)->_bounty=value; }

unsigned long OOGFOSTestObjCShipDeallocations(void){return testShipDeallocations;}
