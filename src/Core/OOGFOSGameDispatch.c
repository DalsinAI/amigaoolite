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
LONG OOGFOSGameDispatchCancelQueued(struct OOGFOSGameDispatch *d)
{
    struct Message *raw;
    struct OOGFOSGameMessage *m;
    LONG cancelled = 0;
    if (!d || FindTask(NULL) != d->gameTask || !d->gamePort) return -1;
    if (!OOGFOSGameDispatchSeal(d)) return -1;
    /* No new GET/SET can be enqueued after Seal. GetMsg is owner-task-only;
     * status=0 returns ownership of each stack-backed message to its sender. */
    while ((raw = GetMsg(d->gamePort)) != NULL) {
        m = (struct OOGFOSGameMessage *)raw;
        if (raw->mn_Length == sizeof(*m) && m->magic == OOGFOS_RPC_MAGIC &&
            m->op == OOGFOS_DISPATCH_DONE) {
            d->resultCode = m->status;
            d->result = m->value;
            d->completions++;
        } else {
            if (raw->mn_Length == sizeof(*m) && m->magic == OOGFOS_RPC_MAGIC)
                m->status = 0;
            cancelled++;
            d->cancelled++;
        }
        if (raw->mn_ReplyPort) ReplyMsg(raw);
    }
    return cancelled;
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
int OOGFOSGameDispatchSetAction(struct OOGFOSGameDispatch *d,
                               OOGFOSGameActionFunc action)
{
    if (!d || !d->gamePort || FindTask(NULL)!=d->gameTask) return 0;
    Forbid();
    if (d->inFlight || d->sealed) { Permit(); return 0; }
    d->actionBackend=action;
    Permit();
    return 1;
}
int OOGFOSGameShipAction(struct OOGFOSGameShipProxy *proxy,
                        unsigned action, LONG argument, LONG *result)
{
    LONG value=argument;
    if (!proxy || !proxy->dispatch || !proxy->token || !result) return 0;
    if (!dispatchRPC(proxy->dispatch,OOGFOS_DISPATCH_ACTION,
                     proxy->token,action,&value)) return 0;
    *result=value;
    return 1;
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
static int dispatchServeNext(struct OOGFOSGameDispatch *d, int wait)
{
    struct Message *raw;
    struct OOGFOSGameMessage *m;
    int done=0;
    if (!d || !d->gamePort || FindTask(NULL) != d->gameTask) return -1;
    if (wait) WaitPort(d->gamePort);
    raw = GetMsg(d->gamePort);
    if (!raw) return wait ? -1 : 2;
    m = (struct OOGFOSGameMessage *)raw;
    if (raw->mn_Length != sizeof(*m) || m->magic != OOGFOS_RPC_MAGIC) {
        if (raw->mn_ReplyPort) ReplyMsg(raw);
        return -1;
    }
    /* Seal also protects requests already queued before admission closed.
     * CancelQueued handles draining in bulk; Serve must enforce the same
     * boundary when the owner services an earlier queued message instead. */
    if (d->sealed && m->op != OOGFOS_DISPATCH_DONE) {
        m->status = 0;
        d->cancelled++;
        ReplyMsg(raw);
        return 0;
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
    } else if (m->op == OOGFOS_DISPATCH_ACTION) {
        m->status = d->actionBackend && m->token ?
            d->actionBackend(d->owner,m->token,m->property,m->value,&m->value) : 0;
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

/* Non-blocking service for a live Universe/game frame. 2 means empty queue;
 * 0 is ordinary RPC/cancel, 1 is JS worker DONE, -1 invalid owner/message.
 * A caller must bound messages per frame to avoid starving gameplay. */
int OOGFOSGameDispatchPoll(struct OOGFOSGameDispatch *d)
{
    return dispatchServeNext(d, 0);
}
int OOGFOSGameDispatchServe(struct OOGFOSGameDispatch *d)
{
    return dispatchServeNext(d, 1);
}
