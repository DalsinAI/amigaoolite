/* Exec-message dispatch for GFOS JavaScript -> Oolite game task.
 * GPL-2.0-or-later; uses caller-owned game task and no new engine/process.
 */
#ifndef OOLITE_GFOS_GAME_DISPATCH_H
#define OOLITE_GFOS_GAME_DISPATCH_H
#include <exec/types.h>
#include <exec/ports.h>
#include "OOGFOSShipProperties.h"

enum { OOGFOS_DISPATCH_GET=1, OOGFOS_DISPATCH_SET=2, OOGFOS_DISPATCH_DONE=3, OOGFOS_DISPATCH_ACTION=4 };
struct OOGFOSGameRouteBackend {
    int (*getLong)(void *owner, ULONG token, unsigned property, LONG *value);
    int (*setLong)(void *owner, ULONG token, unsigned property, LONG value);
};
/* An action is dispatched to the owner game task only. It is intentionally
 * separate from scalar properties; production must implement game actions. */
typedef int (*OOGFOSGameActionFunc)(void *owner, ULONG token,
                                   unsigned action, LONG argument, LONG *result);
enum { OOGFOS_SHIP_ACTION_LEGACY=1 };
struct OOGFOSGameDispatch;
/* Caller-owned, stable while wrappers may call it; never pass ShipEntity here. */
struct OOGFOSGameShipProxy {
    struct OOGFOSGameDispatch *dispatch;
    ULONG token;
};
struct OOGFOSGameDispatch {
    struct MsgPort *gamePort;
    struct Task *gameTask; /* only owner may service or destroy the port */
    void *owner;
    const struct OOGFOSShipBackend *ownerBackend;
    const struct OOGFOSGameRouteBackend *routeBackend;
    OOGFOSGameActionFunc actionBackend;
    ULONG inFlight; /* protected by Forbid/Permit with gamePort */
    ULONG sealed; /* stop new GET/SET; allow DONE for queued JS jobs */
    ULONG requests;
    ULONG completions;
    ULONG cancelled;
    LONG result;
    LONG resultCode;
};
int OOGFOSGameDispatchInit(struct OOGFOSGameDispatch *d, void *owner,
                           const struct OOGFOSShipBackend *ownerBackend);
/* Owner-task close: 1 if port closed, 0 if requests still in-flight/wrong task. */
/* Owner task can stop admitting new property operations without dropping DONE. */
int OOGFOSGameDispatchSeal(struct OOGFOSGameDispatch *d);
/* Owner seals admission, rejects queued properties by replying status=0,
 * preserves queued worker completion packets. Does NOT stop running callbacks. */
LONG OOGFOSGameDispatchCancelQueued(struct OOGFOSGameDispatch *d);
int OOGFOSGameDispatchTryClose(struct OOGFOSGameDispatch *d);
/* Compatibility wrapper: does NOT close if there are outstanding messages. */
void OOGFOSGameDispatchClose(struct OOGFOSGameDispatch *d);
/* Routed mode: identity is a game-task-validated token, not a cross-task pointer. */
int OOGFOSGameDispatchInitRouted(struct OOGFOSGameDispatch *d, void *owner,
                                const struct OOGFOSGameRouteBackend *backend);
const struct OOGFOSShipBackend *OOGFOSGameShipProxyBackend(void);
/* Configure action handler on game task before publishing JS wrappers. */
int OOGFOSGameDispatchSetAction(struct OOGFOSGameDispatch *d,
                               OOGFOSGameActionFunc action);
/* JS task action call; roundtrip via tracked Exec message to game owner. */
int OOGFOSGameShipAction(struct OOGFOSGameShipProxy *proxy,
                        unsigned action, LONG argument, LONG *result);
int OOGFOSGameDispatchGet(void *state, unsigned property, LONG *value);
int OOGFOSGameDispatchSet(void *state, unsigned property, LONG value);
/* Pump ONE message on the owner's game task. Return 1 on job completion. */
int OOGFOSGameDispatchServe(struct OOGFOSGameDispatch *d);
/* Frame-loop-safe nonblocking pump: 2 empty, 0 RPC, 1 DONE, -1 error. */
int OOGFOSGameDispatchPoll(struct OOGFOSGameDispatch *d);
/* Signals completion from javascript.library JS_Post callback task. */
int OOGFOSGameDispatchDone(struct OOGFOSGameDispatch *d, LONG rc, LONG value);
const struct OOGFOSShipBackend *OOGFOSGameDispatchBackend(void);
#endif
