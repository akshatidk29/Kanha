#include "pmm.h"
#include <stdint.h>
#include <stddef.h>

#include "memLayout.h"
#include "e820.h"

#define BITS_PER_BYTE 8u
#define BITMAP_SIZE   (PMM_FRAME_COUNT / BITS_PER_BYTE)

static uint8_t usable_bitMap[BITMAP_SIZE];
static uint8_t used_bitMap[BITMAP_SIZE];

static uint32_t freeFrameCount;


static inline void setFrameBit(uint8_t* bitMap, uint32_t frame){
    bitMap[frame / BITS_PER_BYTE] |= (1u << (frame % BITS_PER_BYTE));
}

static inline void unsetFrameBit(uint8_t* bitMap, uint32_t frame){
    bitMap[frame / BITS_PER_BYTE] &= ~(1u << (frame % BITS_PER_BYTE));
}

static inline int isFrameBitSet(uint8_t* bitMap, uint32_t frame){
    return (bitMap[frame / BITS_PER_BYTE] & (1u << (frame % BITS_PER_BYTE)));
}


static void setUsableRange(uint64_t base, uint64_t length){

    if(length == 0 || base >= PMM_MEMORY_LIMIT)
        return;

    uint64_t available = PMM_MEMORY_LIMIT - base;
    uint64_t end = (length > available) ? PMM_MEMORY_LIMIT : base + length;

    uint64_t firstFrame = (base / PMM_PAGE_SIZE) + (base % PMM_PAGE_SIZE != 0);
    uint64_t lastFrame = end / PMM_PAGE_SIZE;

    for(uint64_t frame = firstFrame; frame < lastFrame; ++frame){
        if(!isFrameBitSet(usable_bitMap, frame)){
            setFrameBit(usable_bitMap, frame);
            unsetFrameBit(used_bitMap, frame);
        }
    }
}

static void setReservedRange(uint64_t base, uint64_t length){
    
    if (length == 0 || base >= PMM_MEMORY_LIMIT)
        return;

    uint64_t available = PMM_MEMORY_LIMIT - base;
    uint64_t end = (length > available) ? PMM_MEMORY_LIMIT : base + length;

    uint64_t firstFrame = base / PMM_PAGE_SIZE;

    uint64_t endFrame = (end / PMM_PAGE_SIZE) + (end % PMM_PAGE_SIZE != 0);

    for (uint64_t frame = firstFrame; frame < endFrame; ++frame) {
        setFrameBit(used_bitMap, (uint32_t)frame);
    }
}
    

void pmmInitialize(void* bootInfo){

    struct BootInfo* bi = (struct BootInfo*) bootInfo;
    
    for(int i = 0; i < BITMAP_SIZE; i++){
        usable_bitMap[i] = 0;
        used_bitMap[i] = 0xFF;
    }

    freeFrameCount = 0;

    struct E820Entry* entry = (struct E820Entry*)PHYS_TO_VIRT(bi->e820Address);
    
    for(uint32_t i = 0; i < bi->e820Count; i++){
        if(entry[i].type == 1){
            setUsableRange(entry[i].base, entry[i].length);
        }
    }

    for(uint32_t i = 0; i < bi->e820Count; i++){
        if(entry[i].type != 1){
            setReservedRange(entry[i].base, entry[i].length);
        }
    }

    setReservedRange(0, PHYS_LOW_MEMORY_END);

    uintptr_t kernelBeginning = (uintptr_t)kernelStart;
    uintptr_t kernelEnding = (uintptr_t)kernelEnd;
    uint64_t kernelSize = kernelEnding - kernelBeginning;
    setReservedRange((uint64_t)kernelBeginning, kernelSize);

    // Boot stack
    setReservedRange(PHYS_BOOT_STACK_BASE, PHYS_BOOT_STACK_SIZE);

    for(uint32_t frame = 0; frame < PMM_FRAME_COUNT; frame++){ 
        if(isFrameBitSet(usable_bitMap, frame) && !isFrameBitSet(used_bitMap, frame)){
            freeFrameCount++;
        }
    }
}

uintptr_t pmmAllocateFrame(){
    for(uint32_t frame = 0; frame < PMM_FRAME_COUNT; frame++){
        if(isFrameBitSet(usable_bitMap, frame) && !isFrameBitSet(used_bitMap, frame)){
            setFrameBit(used_bitMap, frame);
            freeFrameCount--;
            return (uintptr_t)frame * PMM_PAGE_SIZE;
        }
    }
    return 0;
}

uint32_t pmmDeallocateFrame(uintptr_t phyAddress){

    if (phyAddress == 0 ||
        phyAddress >= PMM_MEMORY_LIMIT ||
        phyAddress % PMM_PAGE_SIZE != 0) {
        return 1;
    }

    uint32_t frame = phyAddress / PMM_PAGE_SIZE;

    if(!isFrameBitSet(used_bitMap, frame) || !isFrameBitSet(usable_bitMap, frame)){
        return 1;
    }

    unsetFrameBit(used_bitMap, frame);
    freeFrameCount++;

    return 0;
}

uint32_t pmmFreeFramesCount(){
    return freeFrameCount;
}

uint32_t pmmReservedFramesCount(){
    return PMM_FRAME_COUNT - freeFrameCount;
}

uint32_t pmmTotalFramesCount(){
    return PMM_FRAME_COUNT;
}