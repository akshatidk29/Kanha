#ifndef KANHA_HEAP_H
#define KANHA_HEAP_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "pmm.h"
#include "paging.h"
#include "memLayout.h"

#define STRUCT_BLOCK_SIZE ((sizeof(struct Block) + 7) & ~7u)

#define MIN_BLOCK_SIZE    (STRUCT_BLOCK_SIZE + 8u)
#define MAX_HEAP_SIZE     (1024u * 1024u)
#define HEAP_NUM_PAGES    (MAX_HEAP_SIZE / PAGE_SIZE)

struct Block{
    bool isFree;
    uint32_t size;      
    struct Block* prev;
    struct Block* next;
};

struct Heap{
    struct Block* start;
    struct Block* end;
};


int heapInitialize(void);

void *kMalloc(size_t size);
int kFree(void* ptr);


#endif