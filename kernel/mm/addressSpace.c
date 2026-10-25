#include <stdint.h>
#include "addressSpace.h"

#include "pmm.h"
#include "paging.h"
#include "memLayout.h"

#define PAGE_SIZE          4096u
#define PAGE_ENTRIES       1024u
#define PAGE_ADDRESS_MASK  0xFFFFF000u

#define KERNEL_PDE_START 768u

extern uint32_t getKernelPageDirectory(void);
extern uint32_t getCurrentPageDirectory(void);

int createAddressSpace(struct AddressSpace *as){

    if(as == 0 || as->pageDirectory != 0)
        return -1;

    uint32_t frame = pmmAllocateFrame();
    if(frame == 0)
        return -1;

    uint32_t *pd  = (uint32_t *)PHYS_TO_VIRT(frame);
    uint32_t *kpd = (uint32_t *)PHYS_TO_VIRT(getKernelPageDirectory());

    for(size_t i = 0; i < KERNEL_PDE_START; i++)
        pd[i] = 0;

    for(size_t i = KERNEL_PDE_START; i < PAGE_ENTRIES; i++)
        pd[i] = kpd[i];
        
    as->pageDirectory = frame;
    return 0;
}

int destroyAddressSpace(struct AddressSpace *as){

    if(as == 0 || as->pageDirectory == 0)
        return -1;

    uint32_t kernelDirectory = getKernelPageDirectory();
    uint32_t currentDirectory = getCurrentPageDirectory();
    uint32_t directory = as->pageDirectory & PAGE_ADDRESS_MASK;

    if(directory == (kernelDirectory & PAGE_ADDRESS_MASK))
        return -1;
    if(directory == currentDirectory)
        return -1;

    uint32_t *pd = (uint32_t *)PHYS_TO_VIRT(directory);

    // Free user page tables in PDE 0 to 767
    for(uint32_t i = 0; i < KERNEL_PDE_START; i++){
        uint32_t pde = pd[i];

        if(!(pde & PAGE_PRESENT))
            continue;

        pd[i] = 0;
        
        uint32_t tableFrame = pde & PAGE_ADDRESS_MASK;
        if(pmmDeallocateFrame(tableFrame) != 0)
            return -1;
    }

    if(pmmDeallocateFrame(directory) != 0)
        return -1;

    as->pageDirectory = 0;
    return 0;
}

int switchAddressSpace(struct AddressSpace *as){

    if(as == 0 || as->pageDirectory == 0)
        return -1;
    
    __asm__ volatile("mov %0, %%cr3" : : "r" (as->pageDirectory) : "memory");
    return 0;
}

int addressSpaceMapPage(struct AddressSpace *as, uint32_t virt, uint32_t phys, uint32_t flags){
    if(as == 0 || as->pageDirectory == 0)
        return -1;
    if(virt < ADDRESS_SPACE_USER_START || virt >= ADDRESS_SPACE_USER_END)
        return -1;
    if((flags & PAGE_USER) == 0)
        return -1;
    if((flags & ~(PAGE_WRITABLE | PAGE_USER)) != 0)
        return -1;
    return mapPage(as->pageDirectory, virt, phys, flags);
}

int addressSpaceUnMapPage(struct AddressSpace *as, uint32_t virt){
    if(as == 0 || as->pageDirectory == 0)
        return -1;
    if(virt < ADDRESS_SPACE_USER_START || virt >= ADDRESS_SPACE_USER_END)
        return -1;
    return unMapPage(as->pageDirectory, virt);
}

int addressSpaceGetPhysicalAddress(struct AddressSpace *as, uint32_t virt, uint32_t *phys){
    if(as == 0 || as->pageDirectory == 0 || phys == 0)
        return -1;
    if(virt < ADDRESS_SPACE_USER_START || virt >= ADDRESS_SPACE_USER_END)
        return -1;
    return getPhysicalAddress(as->pageDirectory, virt, phys);
}