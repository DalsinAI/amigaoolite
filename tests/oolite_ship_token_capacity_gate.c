/* Stress O(1) registry at full capacity and force same-slot generation reuse. */
#include "OOGFOSShipTokens.h"
#include <stdio.h>
static struct OOGFOSShipTokenTable table;
static LONG object[OOGFOS_SHIP_TOKEN_CAPACITY+2];
static ULONG tokens[OOGFOS_SHIP_TOKEN_CAPACITY];
int main(void){
 ULONG i,old,newToken=0,fullFailed=0,retired=0;int pass=1;
 OOGFOSShipTokensInit(&table);
 for(i=0;i<OOGFOS_SHIP_TOKEN_CAPACITY;i++){
  object[i]=(LONG)i;
  tokens[i]=OOGFOSShipTokensAdd(&table,&object[i]);
  if(!tokens[i] || OOGFOSShipTokensLookup(&table,tokens[i])!=&object[i])pass=0;
 }
 fullFailed=OOGFOSShipTokensAdd(&table,&object[OOGFOS_SHIP_TOKEN_CAPACITY])==0;
 old=tokens[0];
 if(!OOGFOSShipTokensRemove(&table,old))pass=0;
 newToken=OOGFOSShipTokensAdd(&table,&object[OOGFOS_SHIP_TOKEN_CAPACITY]);
 if(!newToken || newToken==old || (newToken&0xFFFUL)!=(old&0xFFFUL) ||
     OOGFOSShipTokensLookup(&table,old)!=NULL ||
     OOGFOSShipTokensLookup(&table,newToken)!=&object[OOGFOS_SHIP_TOKEN_CAPACITY])pass=0;
 /* Artificial generation exhaustion retires slot 0 rather than alias. */
 if(!OOGFOSShipTokensRemove(&table,newToken))pass=0;
 table.slots[0].generation=OOGFOS_SHIP_TOKEN_GEN_MAX;
 retired=OOGFOSShipTokensAdd(&table,&object[OOGFOS_SHIP_TOKEN_CAPACITY+1])==0;
 pass=pass&&fullFailed&&retired;
 printf("OOLITE_SHIP_TOKEN_CAPACITY slots=%u full_rejected=%lu old=%lu replacement=%lu retired=%lu status=%s\n",
 (unsigned)OOGFOS_SHIP_TOKEN_CAPACITY,(unsigned long)fullFailed,(unsigned long)old,
 (unsigned long)newToken,(unsigned long)retired,pass?"PASS":"FAIL");fflush(stdout);
 return pass?0:2;
}
