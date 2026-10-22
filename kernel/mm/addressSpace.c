#include "addressSpace.h"
#include <stdint.h>

#include "pmm.h"
#include "paging.h"

#define PAGE_SIZE          4096u
#define PAGE_ENTRIES       1024u
#define KERNEL_PDE_COUNT   32u
#define PAGE_ADDRESS_MASK  0xFFFFF000u

extern uint32_t pagingGetDirectory(void);

static uint32_t *physicalPointer(uint32_t phys){
    return (uint32_t *)(uintptr_t)phys;
}


int createAddressSpace(struct AddressSpace *as){

    if(as == 0)
        return -1;

    uint32_t frame = pmmAllocateFrame();
    if(frame == 0)
        return -1;

    uint32_t *pd  = physicalPointer(frame);
    uint32_t *kpd = physicalPointer(pagingGetDirectory());

    for(size_t i = 0; i < KERNEL_PDE_COUNT; i++)
        pd[i] = kpd[i];

    for(size_t i = KERNEL_PDE_COUNT; i < PAGE_ENTRIES; i++)
        pd[i] = 0;
    
    as->pageDirectory = frame;
    return 0;
}

int destroyAddressSpace(struct AddressSpace *as){
    return 0;
}

int switchAddressSpace(struct AddressSpace *as){

    if(as == 0 || as->pageDirectory == 0)
        return -1;
    
    asm volatile("mov %0, %%cr3" : : "r" (as->pageDirectory) : "memory");
    return 0;
}

int addressSpaceMapPage(struct AddressSpace *as, uint32_t virt, uint32_t phys, uint32_t flags){
    if(as == 0 || as->pageDirectory == 0)
        return -1;
    if(virt < ADRESS_SPACE_USER_START)
        return -1;
    return mapPage(as->pageDirectory, virt, phys, flags);
}

int addressSpaceUnMapPage(struct AddressSpace *as, uint32_t virt){
    if(as == 0 || as->pageDirectory == 0)
        return -1;
    if(virt < ADRESS_SPACE_USER_START)
        return -1;
    return unMapPage(as->pageDirectory, virt);
}

int addressSpaceGetPhysicalAddress(struct AddressSpace *as, uint32_t virt, uint32_t *phys){
    if(as == 0 || as->pageDirectory == 0 || phys == 0)
        return -1;
    return getPhysicalAddress(as->pageDirectory, virt, phys);
}