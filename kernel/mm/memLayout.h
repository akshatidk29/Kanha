#ifndef KANHA_MEMORY_LAYOUT_H
#define KANHA_MEMORY_LAYOUT_H

#include <stdint.h>

// Virtual Memory Layout 
#define KERNEL_VIRTUAL_BASE     0xC0000000u 
#define KERNEL_SPACE_SIZE       0x40000000u 
#define USER_SPACE_SIZE         KERNEL_VIRTUAL_BASE 

#define KERNEL_DIRECT_MAP_SIZE  (512u * 1024u * 1024u) 
#define KERNEL_DYNAMIC_START    (KERNEL_VIRTUAL_BASE + KERNEL_DIRECT_MAP_SIZE) 
#define KERNEL_DYNAMIC_SIZE     (KERNEL_SPACE_SIZE - KERNEL_DIRECT_MAP_SIZE)   

// Conversion
#define PHYS_TO_VIRT(p)         ((void *)((uintptr_t)(p) + KERNEL_VIRTUAL_BASE))
#define VIRT_TO_PHYS(v)         ((uintptr_t)(v) - KERNEL_VIRTUAL_BASE)

// Physical Memory Layout (Below 1MB)
#define PHYS_IVT_BASE           0x00000000u // IVT 
#define PHYS_BDA_BASE           0x00000400u // BIOS Data Area 
#define PHYS_E820_BUFFER        0x00007000u // E820 memory map buffer
#define PHYS_BOOT_INFO          0x00007400u // BootInfo passed to kernel
#define PHYS_BOOTLOADER_MBR     0x00007C00u // MBR load address
#define PHYS_KERNEL_HEADER      0x00008000u // Kernel header
#define PHYS_KERNEL_LOAD_ADDR   0x00010000u // Kernel load start 

#define PHYS_BOOT_STACK_BASE    0x00080000u // Stack base
#define PHYS_BOOT_STACK_SIZE    0x00010000u // Stack size
#define PHYS_BOOT_STACK_TOP     (PHYS_BOOT_STACK_BASE + PHYS_BOOT_STACK_SIZE) 

#define PHYS_EBDA_APPROX        0x0009FC00u // Extended BIOS Data Area
#define PHYS_VGA_BUFFER         0x000B8000u // VGA frame buffer
#define PHYS_BIOS_ROM_START     0x000C0000u // Video BIOS and Motherboard BIOS ROMs
#define PHYS_LOW_MEMORY_END     0x00100000u // 1 MB



// Virtual Memory Aliases
#define VIRT_VGA_BUFFER         ((void *)(KERNEL_VIRTUAL_BASE + PHYS_VGA_BUFFER)) // 0xC00B8000
#define VIRT_BOOT_STACK_TOP     (KERNEL_VIRTUAL_BASE + PHYS_BOOT_STACK_TOP)       // 0xC0090000

// Linker symbols
extern uint8_t kernelStart[];
extern uint8_t kernelEnd[];

#endif