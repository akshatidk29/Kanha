#ifndef KANHA_PMM_H
#define KANHA_PMM_H

#include <stdint.h> 
#include <stddef.h>

#define PMM_PAGE_SIZE 4096u
#define PMM_MEMORY_LIMIT (128u * 4096u * 4096u)
#define PMM_FRAME_COUNT (PMM_MEMORY_LIMIT / PMM_PAGE_SIZE)

void pmm_initialize(void* bootInfo);

uintptr_t pmm_allocate_frame();

uint32_t pmm_deallocate_frame(uintptr_t frame);

uint32_t pmm_free_frames_count();
uint32_t pmm_reserved_frames_count();
uint32_t pmm_total_frames_count();

#endif