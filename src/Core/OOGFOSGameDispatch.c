#include "OOGFOSGameDispatch.h"
#include <proto/exec.h>
#include <string.h>
#define OOGFOS_RPC_MAGIC 0x4f4f4752UL
struct OOGFOSGameMessage {
    struct Message msg;
    ULONG magic;
    ULONG op;
    ULONG property;
    ULONG token;
    LONG value;
    LONG status;
};
/* Publish under Forbid: TryClose cannot delete the target between the
 * live-port check and PutMsg. The owner MUST keep the dispatch struct alive
 * and join queued JS_Post jobs before freeing it. */
static int sendTracked(struct OOGFOSGameDispatch *d, struct OOGFOSGameMessage *m)
{
    int sent = 0;
    Forbid();
    if (d->gamePort && FindTask(NULL) != d->gameTask &&
        (!d->sealed || m->op == OOGFOS_DISPATCH_DONE)) {
        d->inFlight++;
        PutMsg(d->gamePort, &m->msg);
        sent = 1;
    }
    Permit();
    return sent;
}
static void finishTracked(struct OOGFOSGameDispatch *d)
{
    Forbid();
    if (d->inFlight) d->inFlight--;
    Permit();
}
static int dispatchRPC(struct OOGFOSGameDispatch *d, ULONG op, ULONG token,
                       unsigned property, LONG *value)
{
    struct MsgPort *reply;
    struct OOGFOSGameMessage m;
    struct Message *received;
    int accepted;
    if (!d || !value) return 0;
    reply = CreateMsgPort();
    if (!reply) return 0;
    memset(&m, 0, sizeof(m));
    m.msg.mn_Length = sizeof(m);
    m.msg.mn_ReplyPort = reply;
    m.magic = OOGFOS_RPC_MAGIC;
    m.op = op;
    m.property = property;
    m.token = token;
    m.value = *value;
    if (!sendTracked(d, &m)) { DeleteMsgPort(reply); return 0; }
    WaitPort(reply);
    received = GetMsg(reply);
    accepted = received == &m.msg && m.magic == OOGFOS_RPC_MAGIC && m.status == 1;
    if (accepted) *value = m.value;
    DeleteMsgPort(reply);
    finishTracked(d);
    return accepted;
}
int OOGFOSGameDispatchInit(struct OOGFOSGameDispatch *d, void *owner,
                           const struct OOGFOSShipBackend *ownerBackend)
{
    if (!d || !owner || !ownerBackend || !ownerBackend->getLong ||
        !ownerBackend->setLong) return 0;
    memset(d, 0, sizeof(*d));
    d->gameTask = FindTask(NULL);
    d->owner = owner;
    d->ownerBackend = ownerBackend;
    d->gamePort = CreateMsgPort();
    return d->gamePort != NULL;
}
int OOGFOSGameDispatchInitRouted(struct OOGFOSGameDispatch *d, void *owner,
                                const struct OOGFOSGameRouteBackend *backend)
{
    if (!d || !owner || !backend || !backend->getLong || !backend->setLong) return 0;
    memset(d, 0, sizeof(*d));
    d->gameTask = FindTask(NULL);
    d->owner = owner;
    d->routeBackend = backend;
    d->gamePort = CreateMsgPort();
    return d->gamePort != NULL;
}
int OOGFOSGameDispatchSeal(struct OOGFOSGameDispatch *d)
{
    int ok = 0;
    if (!d || FindTask(NULL) != d->gameTask) return 0;
    Forbid();
    if (d->gamePort) { d->sealed = 1; ok = 1; }
    Permit();
    return ok;
}
int OOGFOSGameDispatchTryClose(struct OOGFOSGameDispatch *d)
{
    struct MsgPort *port;
    if (!d || FindTask(NULL) != d->gameTask) return 0;
    Forbid();
    if (!d->gamePort || d->inFlight) {
        Permit();
        return 0;
    }
    port = d->gamePort;
    d->gamePort = NULL;
    Permit();
    /* Only the owner task may dispose the port, and it has no outstanding
     * senders. Clients must still be joined before freeing the dispatch. */
    DeleteMsgPort(port);
    return 1;
}
void OOGFOSGameDispatchClose(struct OOGFOSGameDispatch *d)
{
    (void)OOGFOSGameDispatchTryClose(d);
}
int OOGFOSGameDispatchGet(void *state, unsigned property, LONG *value)
{
    return dispatchRPC((struct OOGFOSGameDispatch *)state,
                       OOGFOS_DISPATCH_GET, 0, property, value);
}
int OOGFOSGameDispatchSet(void *state, unsigned property, LONG value)
{
    return dispatchRPC((struct OOGFOSGameDispatch *)state,
                       OOGFOS_DISPATCH_SET, 0, property, &value);
}
static int routedGet(void *context, unsigned property, LONG *value)
{
    struct OOGFOSGameShipProxy *p=(struct OOGFOSGameShipProxy *)context;
    if (!p || !p->dispatch || !p->token) return 0;
    return dispatchRPC(p->dispatch, OOGFOS_DISPATCH_GET, p->token, property, value);
}
static int routedSet(void *context, unsigned property, LONG value)
{
    struct OOGFOSGameShipProxy *p=(struct OOGFOSGameShipProxy *)context;
    if (!p || !p->dispatch || !p->token) return 0;
    return dispatchRPC(p->dispatch, OOGFOS_DISPATCH_SET, p->token, property, &value);
}
const struct OOGFOSShipBackend *OOGFOSGameShipProxyBackend(void)
{
    static const struct OOGFOSShipBackend backend={routedGet,routedSet};
    return &backend;
}
const struct OOGFOSShipBackend *OOGFOSGameDispatchBackend(void)
{
    static const struct OOGFOSShipBackend backend = {
        OOGFOSGameDispatchGet, OOGFOSGameDispatchSet
    };
    return &backend;
}
int OOGFOSGameDispatchDone(struct OOGFOSGameDispatch *d, LONG rc, LONG value)
{
    struct MsgPort *reply;
    struct OOGFOSGameMessage m;
    struct Message *r;
    if (!d) return 0;
    reply = CreateMsgPort();
    if (!reply) return 0;
    memset(&m, 0, sizeof(m));
    m.msg.mn_Length = sizeof(m);
    m.msg.mn_ReplyPort = reply;
    m.magic = OOGFOS_RPC_MAGIC;
    m.op = OOGFOS_DISPATCH_DONE;
    m.value = value;
    m.status = rc;
    if (!sendTracked(d, &m)) { DeleteMsgPort(reply); return 0; }
    WaitPort(reply);
    r = GetMsg(reply);
    DeleteMsgPort(reply);
    finishTracked(d);
    return r == &m.msg;
}
int OOGFOSGameDispatchServe(struct OOGFOSGameDispatch *d)
{
    struct Message *raw;
    struct OOGFOSGameMessage *m;
    int done=0;
    if (!d || !d->gamePort || FindTask(NULL) != d->gameTask) return -1;
    WaitPort(d->gamePort);
    raw = GetMsg(d->gamePort);
    if (!raw) return -1;
    m = (struct OOGFOSGameMessage *)raw;
    if (raw->mn_Length != sizeof(*m) || m->magic != OOGFOS_RPC_MAGIC) {
        if (raw->mn_ReplyPort) ReplyMsg(raw);
        return -1;
    }
    if (m->op == OOGFOS_DISPATCH_GET) {
        m->status = d->routeBackend ?
            (m->token ? d->routeBackend->getLong(d->owner, m->token, m->property, &m->value) : 0) :
            d->ownerBackend->getLong(d->owner, m->property, &m->value);
        d->requests++;
    } else if (m->op == OOGFOS_DISPATCH_SET) {
        m->status = d->routeBackend ?
            (m->token ? d->routeBackend->setLong(d->owner, m->token, m->property, m->value) : 0) :
            d->ownerBackend->setLong(d->owner, m->property, m->value);
        d->requests++;
    } else if (m->op == OOGFOS_DISPATCH_DONE) {
        d->resultCode = m->status;
        d->result = m->value;
        d->completions++;
        done = 1;
    } else m->status = 0;
    ReplyMsg(raw);
    return done;
}
