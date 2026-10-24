#include "paging.h"
#include "pmm.h"

#include <stdint.h>

#define PAGE_MASK   0xFFFFF000u
#define ADDRESS_MASK 0xFFFFF000u


#define ENTRIES_PER_TABLE 1024u
#define IDENTITY_MAP_SIZE (128u * 1024u * 1024u)
#define TABLE_COUNT (IDENTITY_MAP_SIZE / (4u * 1024u * 1024u))


#define KERNEL_VIRTUAL_BASE       0xC0000000u
#define KERNEL_PDE_START          768u
#define HIGHER_HALF_TABLE_COUNT   TABLE_COUNT

static uint32_t higherHalfTablePhysical[HIGHER_HALF_TABLE_COUNT];


static uint32_t pageDirectoryPhysical;
static uint32_t pageTablePhysical[TABLE_COUNT];

static inline void invalidatePage(uint32_t virt){
    asm volatile("invlpg (%0)" :: "r" ((uintptr_t)virt) : "memory");
}


uint32_t pagingInitialize(void){

    if (pageDirectoryPhysical != 0)
        return 1;

    uint32_t identityMapTableCount = 0;
    uint32_t higherHalfTableCount = 0;

    pageDirectoryPhysical = (uint32_t)pmmAllocateFrame();

    if(pageDirectoryPhysical == 0)
        goto failure;

    uint32_t *pageDirectory = (uint32_t *)(uintptr_t)pageDirectoryPhysical;
    
    for(uint32_t entry = 0; entry < ENTRIES_PER_TABLE; entry++)
        pageDirectory[entry] = 0;

    for(uint32_t table = 0; table < TABLE_COUNT; table++){
        pageTablePhysical[table] = (uint32_t)pmmAllocateFrame();
        
        if(pageTablePhysical[table] == 0)
            goto failure;

        identityMapTableCount++;

        uint32_t* pageTable = (uint32_t* )(uintptr_t )(pageTablePhysical[table]);
        
        for(uint32_t entry = 0; entry < ENTRIES_PER_TABLE; entry++){
            uint32_t physical = table * 0x400000u + entry * PAGE_SIZE;
            pageTable[entry] = physical | PAGE_PRESENT | PAGE_WRITABLE;
        }

        pageDirectory[table] = pageTablePhysical[table] | PAGE_PRESENT | PAGE_WRITABLE;
    }

    // Initialize higher half aliases for kernel
    for (uint32_t table = 0; table < HIGHER_HALF_TABLE_COUNT; table++) {

        higherHalfTablePhysical[table] = (uint32_t)pmmAllocateFrame();

        if (higherHalfTablePhysical[table] == 0)
            goto failure;

        higherHalfTableCount++;

        uint32_t *pageTable = (uint32_t *)(uintptr_t)higherHalfTablePhysical[table];

        for (uint32_t entry = 0; entry < ENTRIES_PER_TABLE; entry++) {
            uint32_t physical = table * 0x400000u + entry * PAGE_SIZE;
            pageTable[entry] = physical | PAGE_PRESENT | PAGE_WRITABLE;
        }

        uint32_t pdeIndex = KERNEL_PDE_START + table;
        pageDirectory[pdeIndex] = higherHalfTablePhysical[table] | PAGE_PRESENT | PAGE_WRITABLE;
    }

    return 0;

failure:
    for (uint32_t i = 0; i < identityMapTableCount; i++) {
        pmmDeallocateFrame(pageTablePhysical[i]);
        pageTablePhysical[i] = 0;
    }

    for (uint32_t i = 0; i < higherHalfTableCount; i++) {
        pmmDeallocateFrame(higherHalfTablePhysical[i]);
        higherHalfTablePhysical[i] = 0;
    }

    if (pageDirectoryPhysical != 0) {
        pmmDeallocateFrame(pageDirectoryPhysical);
        pageDirectoryPhysical = 0;
    }

    return 1;
}

uint32_t getKernelPageDirectory(void){
    return pageDirectoryPhysical;
}

uint32_t getCurrentPageDirectory(void){
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3 & ADDRESS_MASK;
}

int mapPage(uint32_t pd, uint32_t virt, uint32_t phys, uint32_t flags){
    
    if(pd == 0)
        return -1;

    if((virt & 0xFFFu) || (phys & 0xFFFu))
        return -1;

    if(flags & ~(PAGE_WRITABLE | PAGE_USER))
        return -1;

    uint32_t* dir = (uint32_t*)(uintptr_t)pd;

    uint32_t dirIdx = virt >> 22u;
    uint32_t tableIdx = (virt >> 12u) & 0x3FFu;
    
    uint32_t dirEntry = dir[dirIdx];
    uint32_t* table;

    int newTable = 0;
    uint32_t tableFrame = 0;

    if(!(dirEntry & PAGE_PRESENT)){
        tableFrame = (uint32_t)pmmAllocateFrame();
        if(tableFrame == 0)
            return -1;

        table = (uint32_t*)(uintptr_t)tableFrame;
        for(uint32_t i = 0; i < ENTRIES_PER_TABLE; i++){
            table[i] = 0;
        }

        newTable = 1;
    }else{
        table = (uint32_t*)(uintptr_t)(dirEntry & ADDRESS_MASK);
        if ((flags & PAGE_WRITABLE) && !(dirEntry & PAGE_WRITABLE))
            return -1;

        if ((flags & PAGE_USER) && !(dirEntry & PAGE_USER))
            return -1;

        if (table[tableIdx] & PAGE_PRESENT)
            return -1;
    }

    if (newTable){
        uint32_t pdeFlags = PAGE_PRESENT;

        if (flags & PAGE_WRITABLE)
            pdeFlags |= PAGE_WRITABLE;

        if (flags & PAGE_USER)
            pdeFlags |= PAGE_USER;

        dir[dirIdx] = (tableFrame & ADDRESS_MASK) | pdeFlags;
    }

    table[tableIdx] = (phys & ADDRESS_MASK) | flags | PAGE_PRESENT;
    invalidatePage(virt);
    return 0;
}


int unMapPage(uint32_t pd, uint32_t virt){
    
    if(pd == 0 || (virt & 0xFFFu))
        return -1;

    uint32_t* dir = (uint32_t*)(uintptr_t)pd;

    uint32_t dirIdx = virt >> 22u;
    uint32_t tableIdx = (virt >> 12u) & 0x3FFu;
    uint32_t dirEntry = dir[dirIdx];

    if(!(dirEntry & PAGE_PRESENT))
        return -1;

    uint32_t* table = (uint32_t*)(uintptr_t)(dirEntry & ADDRESS_MASK);
   
    if (!(table[tableIdx] & PAGE_PRESENT))
        return -1;
    
    table[tableIdx] = 0;
    invalidatePage(virt);
    return 0;
}

int getPhysicalAddress(uint32_t pd, uint32_t virt, uint32_t* phys){

    if(pd == 0 || phys == 0)
        return -1;
    
    uint32_t* dir = (uint32_t*)(uintptr_t)pd;
    uint32_t dirIdx = virt >> 22u;
    uint32_t tableIdx = (virt >> 12u) & 0x3FFu;
    uint32_t dirEntry = dir[dirIdx];

    if(!(dirEntry & PAGE_PRESENT))
        return -1;

    uint32_t* table = (uint32_t*)(uintptr_t)(dirEntry & ADDRESS_MASK);
    uint32_t tableEntry = table[tableIdx];

    if(!(tableEntry & PAGE_PRESENT))
        return -1;

    *phys = (tableEntry & ADDRESS_MASK) | (virt & 0xFFFu);
    return 0;
}