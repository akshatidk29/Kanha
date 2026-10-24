#ifndef KANHA_MEMORY_LAYOUT_H
#define KANHA_MEMORY_LAYOUT_H

#include <stdint.h>

#define KERNEL_VIRTUAL_BASE 0xC0000000u
#define PHYS_TO_VIRT(p)     ((void *)((uintptr_t)(p) + KERNEL_VIRTUAL_BASE))
#define VIRT_TO_PHYS(v)     ((uintptr_t)(v) - KERNEL_VIRTUAL_BASE)

extern uint8_t kernelStart[];
extern uint8_t kernelEnd[];

#endif