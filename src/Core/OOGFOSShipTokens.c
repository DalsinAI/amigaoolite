#include "OOGFOSShipTokens.h"
#include <proto/exec.h>
#include <string.h>

void OOGFOSShipTokensInit(struct OOGFOSShipTokenTable *t)
{
    if (!t) return;
    memset(t,0,sizeof(*t));
    t->gameTask=FindTask(NULL);
}
ULONG OOGFOSShipTokensAdd(struct OOGFOSShipTokenTable *t, void *object)
{
    ULONG step;
    if (!t || !object || FindTask(NULL)!=t->gameTask) return 0;
    for (step=0;step<OOGFOS_SHIP_TOKEN_CAPACITY;step++) {
        ULONG index=(t->nextSlot+step)%OOGFOS_SHIP_TOKEN_CAPACITY;
        struct OOGFOSShipTokenSlot *slot=&t->slots[index];
        if (!slot->object && slot->generation<OOGFOS_SHIP_TOKEN_GEN_MAX) {
            slot->generation++;
            slot->token=(slot->generation<<12)|(index+1);
            slot->object=object;
            t->nextSlot=(index+1)%OOGFOS_SHIP_TOKEN_CAPACITY;
            return slot->token;
        }
    }
    return 0;
}
void *OOGFOSShipTokensLookup(struct OOGFOSShipTokenTable *t, ULONG token)
{
    ULONG slotIndex;
    struct OOGFOSShipTokenSlot *slot;
    if (!t || !token || FindTask(NULL)!=t->gameTask) return NULL;
    slotIndex=(token&0xFFFUL);
    if (!slotIndex || slotIndex>OOGFOS_SHIP_TOKEN_CAPACITY) return NULL;
    slot=&t->slots[slotIndex-1];
    return slot->token==token ? slot->object : NULL;
}
int OOGFOSShipTokensRemove(struct OOGFOSShipTokenTable *t, ULONG token)
{
    ULONG slotIndex;
    struct OOGFOSShipTokenSlot *slot;
    if (!t || !token || FindTask(NULL)!=t->gameTask)return 0;
    slotIndex=(token&0xFFFUL);
    if (!slotIndex || slotIndex>OOGFOS_SHIP_TOKEN_CAPACITY)return 0;
    slot=&t->slots[slotIndex-1];
    if (!slot->object || slot->token!=token)return 0;
    slot->object=NULL;
    slot->token=0;
    return 1;
}
