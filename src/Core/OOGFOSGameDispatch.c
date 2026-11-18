#include "OOGFOSGameDispatch.h"
#include <proto/exec.h>
#include <string.h>
#define OOGFOS_RPC_MAGIC 0x4f4f4752UL
struct OOGFOSGameMessage {
    struct Message msg;
    ULONG magic;
    ULONG op;
    ULONG property;
    LONG value;
    LONG status;
};
static int dispatchRPC(struct OOGFOSGameDispatch *d, ULONG op,
                       unsigned property, LONG *value)
{
    struct MsgPort *reply;
    struct OOGFOSGameMessage m;
    struct Message *received;
    int accepted;
    if (!d || !d->gamePort || !value) return 0;
    reply = CreateMsgPort();
    if (!reply) return 0;
    memset(&m, 0, sizeof(m));
    m.msg.mn_Length = sizeof(m);
    m.msg.mn_ReplyPort = reply;
    m.magic = OOGFOS_RPC_MAGIC;
    m.op = op;
    m.property = property;
    m.value = *value;
    PutMsg(d->gamePort, &m.msg);
    WaitPort(reply);
    received = GetMsg(reply);
    accepted = received == &m.msg && m.magic == OOGFOS_RPC_MAGIC && m.status == 1;
    if (accepted) *value = m.value;
    DeleteMsgPort(reply);
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
void OOGFOSGameDispatchClose(struct OOGFOSGameDispatch *d)
{
    if (!d || !d->gamePort || FindTask(NULL) != d->gameTask) return;
    /* Caller MUST join all JS_Post users and drain replies first. */
    DeleteMsgPort(d->gamePort);
    d->gamePort = NULL;
}
int OOGFOSGameDispatchGet(void *state, unsigned property, LONG *value)
{
    return dispatchRPC((struct OOGFOSGameDispatch *)state,
                       OOGFOS_DISPATCH_GET, property, value);
}
int OOGFOSGameDispatchSet(void *state, unsigned property, LONG value)
{
    return dispatchRPC((struct OOGFOSGameDispatch *)state,
                       OOGFOS_DISPATCH_SET, property, &value);
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
    if (!d || !d->gamePort) return 0;
    reply = CreateMsgPort();
    if (!reply) return 0;
    memset(&m, 0, sizeof(m));
    m.msg.mn_Length = sizeof(m);
    m.msg.mn_ReplyPort = reply;
    m.magic = OOGFOS_RPC_MAGIC;
    m.op = OOGFOS_DISPATCH_DONE;
    m.value = value;
    m.status = rc;
    PutMsg(d->gamePort, &m.msg);
    WaitPort(reply);
    r = GetMsg(reply);
    DeleteMsgPort(reply);
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
        m->status = d->ownerBackend->getLong(d->owner, m->property, &m->value);
        d->requests++;
    } else if (m->op == OOGFOS_DISPATCH_SET) {
        m->status = d->ownerBackend->setLong(d->owner, m->property, m->value);
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
