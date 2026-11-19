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
@public
    OOCreditsQuantity _bounty;
    OOSystemID _home;
    uint8_t _maxEscorts;
    OOLegalStatusReason _reason;
    OOShipGroup *_group;
    BOOL _testPlayer;
    float _energy, _maxEnergy;
    int _testStatus;
    BOOL _testStation, _testDock, _testInSpace;
    int32_t _legacyCalls;
    int32_t _legacyActions;
}
- (OOCreditsQuantity)bounty;
- (void)setBounty:(OOCreditsQuantity)amount withReason:(OOLegalStatusReason)reason;
- (OOSystemID)homeSystem;
- (void)setHomeSystem:(OOSystemID)system;
- (uint8_t)maxEscortCount;
- (void)setMaxEscortCount:(uint8_t)newCount;
- (OOShipGroup *)escortGroup;
- (BOOL)isPlayer;
- (BOOL)isShip;
- (BOOL)isStation;
- (BOOL)isDock;
- (BOOL)isInSpace;
- (int)status;
- (float)energy;
- (void)setEnergy:(float)value;
- (float)maxEnergy;
- (void)setMaxEnergy:(float)value;
- (void)testRecordLegacyCount:(int)count;
- (int32_t)testLegacyCalls;
- (int32_t)testLegacyActions;
@end
static unsigned long testShipDeallocations=0;
@implementation ShipEntity
- (id)init { self=[super init];if(self){_bounty=10;_home=7;_maxEscorts=2;_energy=64.0f;_maxEnergy=128.0f;_testStatus=100;_testInSpace=YES;_group=[[OOShipGroup alloc]init];}return self; }
- (void)dealloc {testShipDeallocations++;[_group release];[super dealloc];}
- (OOCreditsQuantity)bounty{return _bounty;}
- (void)setBounty:(OOCreditsQuantity)amount withReason:(OOLegalStatusReason)reason{_bounty=amount;_reason=reason;}
- (OOSystemID)homeSystem{return _home;}
- (void)setHomeSystem:(OOSystemID)system{_home=system;}
- (uint8_t)maxEscortCount{return _maxEscorts;}
- (void)setMaxEscortCount:(uint8_t)value{_maxEscorts=value;}
- (OOShipGroup *)escortGroup{return _group;}
- (BOOL)isPlayer{return _testPlayer;}
- (BOOL)isShip{return YES;}
- (BOOL)isStation{return _testStation;}
- (BOOL)isDock{return _testDock;}
- (BOOL)isInSpace{return _testInSpace;}
- (int)status{return _testStatus;}
- (float)energy{return _energy;}
- (void)setEnergy:(float)value{_energy=value;}
- (float)maxEnergy{return _maxEnergy;}
- (void)setMaxEnergy:(float)value{_maxEnergy=value;}
- (void)testRecordLegacyCount:(int)count {_legacyCalls++;_legacyActions+=count;}
- (int32_t)testLegacyCalls{return _legacyCalls;}
- (int32_t)testLegacyActions{return _legacyActions;}
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

int OOGFOSTestObjCRecordLegacy(void *raw,int count){
 if(!raw || count<0)return 0;
 [(ShipEntity *)raw testRecordLegacyCount:count];
 return 1;
}
int OOGFOSTestObjCGetLegacyCounts(void *raw,long *calls,long *actions){
 if(!raw || !calls || !actions)return 0;
 *calls=(long)[(ShipEntity *)raw testLegacyCalls];
 *actions=(long)[(ShipEntity *)raw testLegacyActions];
 return 1;
}

/* Controlled test fixture to verify floating-point ObjC game-task selectors. */
int OOGFOSTestObjCShipEnergy(void *raw,float *energy,float *maxEnergy)
{
    ShipEntity *ship=(ShipEntity *)raw;
    if(!ship || !energy || !maxEnergy)return 0;
    *energy=[ship energy];*maxEnergy=[ship maxEnergy];
    return 1;
}

void OOGFOSTestObjCShipSetStatus(void *raw,int status)
{ if(raw) ((ShipEntity *)raw)->_testStatus=status; }
void OOGFOSTestObjCShipSetStation(void *raw,int enabled)
{ if(raw) ((ShipEntity *)raw)->_testStation=(enabled!=0); }
void OOGFOSTestObjCShipSetDock(void *raw,int enabled)
{ if(raw) ((ShipEntity *)raw)->_testDock=(enabled!=0); }
void OOGFOSTestObjCShipSetInSpace(void *raw,int enabled)
{ if(raw) ((ShipEntity *)raw)->_testInSpace=(enabled!=0); }
