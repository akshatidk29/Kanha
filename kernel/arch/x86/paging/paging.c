#include "paging.h"
#include "pmm.h"

#include <stdint.h>

#define ENTRIES_PER_TABLE 1024u
#define IDENTITY_MAP_SIZE (128u * 1024u * 1024u)
#define TABLE_COUNT (IDENTITY_MAP_SIZE / (4u * 1024u * 1024u))

static uint32_t pageDirectoryPhysical;
static uint32_t pageTablePhysical[TABLE_COUNT];

void pagingInitialize(void){

    pageDirectoryPhysical = (uint32_t)pmm_allocate_frame();

    if(pageDirectoryPhysical == 0)
        return;

    uint32_t *pageDirectory = (uint32_t *)(uintptr_t)pageDirectoryPhysical;
    
    for(uint32_t entry = 0; entry < ENTRIES_PER_TABLE; entry++)
        pageDirectory[entry] = 0;

    for(uint32_t table = 0; table < TABLE_COUNT; table++){
        pageTablePhysical[table] = (uint32_t)pmm_allocate_frame();
        
        if(pageTablePhysical[table] == 0)
            return;

        uint32_t* pageTable = (uint32_t* )(uintptr_t )(pageTablePhysical[table]);
        
        for(uint32_t entry = 0; entry < ENTRIES_PER_TABLE; entry++){
            uint32_t physical = table * 0x400000u + entry * PAGE_SIZE;
            pageTable[entry] = physical | PAGE_PRESENT | PAGE_WRITABLE;
        }

        pageDirectory[table] = pageTablePhysical[table] | PAGE_PRESENT | PAGE_WRITABLE;
    }
}

uint32_t pagingGetDirectory(void){
    return pageDirectoryPhysical;
}