#ifndef KANHA_PMM_H
#define KANHA_PMM_H

#include <stdint.h> 
#include <stddef.h>

#define PMM_PAGE_SIZE 4096u
#define PMM_MEMORY_LIMIT (128u * 1024u * 1024u)
#define PMM_FRAME_COUNT (PMM_MEMORY_LIMIT / PMM_PAGE_SIZE)

void pmmInitialize(void* bootInfo);

uintptr_t pmmAllocateFrame();

uint32_t pmmDeallocateFrame(uintptr_t frame);

uint32_t pmmFreeFramesCount();
uint32_t pmmReservedFramesCount();
uint32_t pmmTotalFramesCount();

#endif