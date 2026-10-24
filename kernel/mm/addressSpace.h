#ifndef KANHA_ADDRESS_SPACE_H
#define KANHA_ADDRESS_SPACE_H

#include <stdint.h>
#include "memLayout.h"

// User virtual memory spans from 0x00001000 to 0xC0000000 (3 GB)
#define ADDRESS_SPACE_USER_START 0x00001000u
#define ADDRESS_SPACE_USER_END   KERNEL_VIRTUAL_BASE

struct AddressSpace{
    uint32_t pageDirectory;
};

int createAddressSpace(struct AddressSpace *as);
int destroyAddressSpace(struct AddressSpace *as);

int switchAddressSpace(struct AddressSpace *as);

int addressSpaceMapPage(struct AddressSpace *as, uint32_t virt, uint32_t phys, uint32_t flags);
int addressSpaceUnMapPage(struct AddressSpace *as, uint32_t virt);
int addressSpaceGetPhysicalAddress(struct AddressSpace *as, uint32_t virt, uint32_t *phys);

#endif
