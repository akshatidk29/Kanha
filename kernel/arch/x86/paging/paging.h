
#ifndef KANHA_PAGING_H
#define KANHA_PAGING_H

#include <stdint.h>

#define PAGE_SIZE       4096u
#define PAGE_PRESENT    0x001u
#define PAGE_WRITABLE   0x002u
#define PAGE_USER       0x004u

void pagingInitialize(void);
uint32_t pagingGetDirectory(void);
void pagingEnable(uint32_t page_directory_physical);

#endif
