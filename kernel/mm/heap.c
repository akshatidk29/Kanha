#include "pmm.h"
#include "heap.h"
#include "paging.h"
#include "memLayout.h"

struct Heap heap;

int heapInitialize(void){

    uint32_t kpd = getKernelPageDirectory();

    for(int i = 0; i < HEAP_NUM_PAGES; i++){

        uint32_t frame = pmmAllocateFrame();

        if(frame == 0){
            for(int j = 0; j < i; j++){
                uint32_t paddr = 0;
                if(getPhysicalAddress(kpd, KERNEL_DYNAMIC_START + j * PAGE_SIZE, &paddr) == 0){
                    pmmDeallocateFrame(paddr);
                    unMapPage(kpd, KERNEL_DYNAMIC_START + j * PAGE_SIZE);
                }
            }
            return -1;
        }

        if(mapPage(kpd, KERNEL_DYNAMIC_START + i * PAGE_SIZE, frame, PAGE_WRITABLE) != 0){
            pmmDeallocateFrame(frame);
            for(int j = 0; j < i; j++){
                uint32_t paddr = 0;
                if(getPhysicalAddress(kpd, KERNEL_DYNAMIC_START + j * PAGE_SIZE, &paddr) == 0){
                    pmmDeallocateFrame(paddr);
                    unMapPage(kpd, KERNEL_DYNAMIC_START + j * PAGE_SIZE);
                }
            }
            return -1;
        }
    }

    struct Block* block = (struct Block*)KERNEL_DYNAMIC_START;
    block->isFree = true;
    block->size = MAX_HEAP_SIZE - STRUCT_BLOCK_SIZE;
    block->prev = NULL;
    block->next = NULL;

    heap.start = block;
    heap.end = block;

    return 0;
}

void *kMalloc(size_t size){

    if (size == 0)
        return NULL;

    struct Block* block = heap.start;
    size_t alignedSize = (size + 7) & ~7u;
    
    while (block != NULL){

        if(block->isFree && (block->size >= alignedSize)){
            
            if((block->size - alignedSize) >= MIN_BLOCK_SIZE){

                struct Block* newBlock = (struct Block*)((uint32_t)block + STRUCT_BLOCK_SIZE + alignedSize);
                newBlock->isFree = true;
                newBlock->size = block->size - alignedSize - STRUCT_BLOCK_SIZE;
                newBlock->prev = block;
                newBlock->next = block->next;

                if(block->next != NULL)
                    block->next->prev = newBlock;
                else
                    heap.end = newBlock;

                block->size = alignedSize;
                block->next = newBlock;
            }
            
            block->isFree = false;

            return (void*)((uint32_t)block + STRUCT_BLOCK_SIZE);
        }

        block = block->next;
    }

    return NULL;
}


int kFree(void* ptr){

    if(ptr == NULL){
        return -1;
    }

    uint32_t addr = (uint32_t)ptr;
    if(addr < (KERNEL_DYNAMIC_START + STRUCT_BLOCK_SIZE) || addr >= (KERNEL_DYNAMIC_START + MAX_HEAP_SIZE)){
        return -1;
    }
    
    struct Block* block = (struct Block*)(addr - STRUCT_BLOCK_SIZE);

    if(!block->isFree){
        block->isFree = true;

        if(block->next != NULL && block->next->isFree){
            block->size += block->next->size + STRUCT_BLOCK_SIZE;
            block->next = block->next->next;
            
            if(block->next != NULL)
                block->next->prev = block;
            else
                heap.end = block;
        }

        if(block->prev != NULL && block->prev->isFree){
            block->prev->size += block->size + STRUCT_BLOCK_SIZE;
            block->prev->next = block->next;
            
            if(block->next != NULL)
                block->next->prev = block->prev;
            else
                heap.end = block->prev;
        }
        return 0;
    }
    
    return -1;
}
