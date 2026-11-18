/* Small native C ABI; do not include Exec types in Oolite Objective-C headers. */
#ifndef OOLITE_GFOS_SYSTEM_INFO_H
#define OOLITE_GFOS_SYSTEM_INFO_H
/* Bytes of available physical memory on the current AmigaOS instance. */
unsigned long OOGFOSMemoryFreeBytes(void);
unsigned long OOGFOSMemoryTotalBytes(void);
#endif
