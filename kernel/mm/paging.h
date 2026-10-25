
#ifndef KANHA_PAGING_H
#define KANHA_PAGING_H

#include <stdint.h>

#define PAGE_SIZE       4096u
#define PAGE_PRESENT    0x001u
#define PAGE_WRITABLE   0x002u
#define PAGE_USER       0x004u

uint32_t pagingInitialize(void);
uint32_t getKernelPageDirectory(void);
uint32_t getCurrentPageDirectory(void);

void pagingEnable(uint32_t page_directory_physical);


int mapPage(uint32_t pd, uint32_t virt, uint32_t phys, uint32_t flags);
int unMapPage(uint32_t pd, uint32_t virt);
int getPhysicalAddress(uint32_t pd, uint32_t virt, uint32_t* phys);


#endif
