/* Game-task-owned O(1) ship identity lookup for Oolite's GFOS JavaScript host
 * objects. No live ShipEntity pointer crosses into javascript.library.
 * GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_SHIP_TOKENS_H
#define OOLITE_GFOS_SHIP_TOKENS_H
#include <exec/types.h>
#include <exec/tasks.h>
/* 12 low token bits encode 1-based slot 1..4095; high 20 bits generation.
 * A slot retires permanently when its 20-bit generation counter is exhausted.
 */
#define OOGFOS_SHIP_TOKEN_CAPACITY 4095
#define OOGFOS_SHIP_TOKEN_GEN_MAX 0xFFFFFUL
struct OOGFOSShipTokenSlot {
    ULONG token;
    ULONG generation;
    void *object;
};
struct OOGFOSShipTokenTable {
    struct Task *gameTask;
    ULONG nextSlot;
    struct OOGFOSShipTokenSlot slots[OOGFOS_SHIP_TOKEN_CAPACITY];
};
void OOGFOSShipTokensInit(struct OOGFOSShipTokenTable *table);
/* Tokens never alias after a slot is reused. Zero means unavailable. */
ULONG OOGFOSShipTokensAdd(struct OOGFOSShipTokenTable *table, void *object);
void *OOGFOSShipTokensLookup(struct OOGFOSShipTokenTable *table, ULONG token);
int OOGFOSShipTokensRemove(struct OOGFOSShipTokenTable *table, ULONG token);
#endif
