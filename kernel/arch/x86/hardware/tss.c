#include <stdint.h>
#include <stddef.h>
#include "hardware/tss.h"

// 8-byte 
struct GDTEntry {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t  baseMiddle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  baseHigh;
} __attribute__((packed));

extern struct GDTEntry gdtStart[];

static struct TSS tss;

static void writeTSSDescriptor(struct GDTEntry *entry, uint32_t base, uint32_t limit) {
    
    entry->limitLow    = (uint16_t)(limit & 0xFFFF);
    entry->baseLow     = (uint16_t)(base & 0xFFFF);
    entry->baseMiddle  = (uint8_t)((base >> 16) & 0xFF);
    entry->access      = 0x89; 
    entry->granularity = (uint8_t)((limit >> 16) & 0x0F); 
    entry->baseHigh    = (uint8_t)((base >> 24) & 0xFF);
}

void tssInitialize(uint32_t kernelSS, uint32_t kernelESP) {
    // Zero out the TSS
    uint8_t *ptr = (uint8_t *)&tss;
    for (uint32_t i = 0; i < sizeof(struct TSS); i++) {
        ptr[i] = 0;
    }

    // Setup Ring 0 stack segment and stack pointer
    tss.ss0       = kernelSS;
    tss.esp0      = kernelESP;

    // Offset >= sizeof(TSS) means no I/O permissions bitmap
    tss.iomapBase = sizeof(struct TSS);

    // Write TSS descriptor into GDT index 5
    writeTSSDescriptor(&gdtStart[5], (uint32_t)&tss, sizeof(struct TSS) - 1);

    // Load the Task Register (TR)
    __asm__ volatile("ltr %%ax" : : "a" ((uint16_t)0x28));
}

void tssSetKernelStack(uint32_t stack) {
    tss.esp0 = stack;
}
