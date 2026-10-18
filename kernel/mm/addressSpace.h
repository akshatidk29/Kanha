#ifndef KANHA_ADDRESS_SPACE_H
#define KANHA_ADDRESS_SPACE_H

#include <stdint.h>

#define ADDRESS_SPACE_KERNEL_SIZE (128u * 1024u * 1024u)
#define ADRESS_SPACE_USER_START (ADDRESS_SPACE_KERNEL_SIZE)

struct addressSpace{
    uint32_t pageDirectory;
};

int createAddressSpace(struct addressSpace *as);
int destroyAddressSpace(struct addressSpace *as);

int switchAddressSpace(struct addressSpace *as);

int addressSpaceMapPage(struct addressSpace *as, uint32_t virt, uint32_t phys, uint32_t flags);
int addressSpaceUnMapPage(struct addressSpace *as, uint32_t virt);
int addressSpaceGetPhysicalAddress(struct addressSpace *as, uint32_t virt, uint32_t *phys);

#endif
