#include "io.h"
#include "pic.h"
#include "vga.h"
#include "paging.h"
#include "exception.h"
#include "memLayout.h"

#define ADDRESS_MASK 0xFFFFF000u

void exceptionHandlerDivideByZero(void){

    vgaWrite("ERROR: DIVIDE BY ZERO\n");
    vgaWrite("CPU PANIC\n");

    while(1){
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}

void exceptionHandlerDoubleFault(uint32_t errorCode){

    vgaWrite("\nERROR: DOUBLE FAULT (#DF)\n");
    vgaWrite("Error code: 0x");
    vgaPutUnsignedInt(errorCode);
    vgaWrite("\nCPU PANIC\n");

    while(1){
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}

void exceptionHandlerGPF(uint32_t errorCode){

    vgaWrite("\nERROR: GENERAL PROTECTION FAULT (#GP)\n");
    vgaWrite("Error code: 0x");
    vgaPutUnsignedInt(errorCode);
    vgaWrite("\nCPU PANIC\n");

    while(1){
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}

void inspectPageFault(uint32_t faultAddress){
    
    uint32_t cr3;

    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));

    uint32_t directoryPhysical = cr3 & ADDRESS_MASK;

    uint32_t directoryIndex = faultAddress >> 22;
    uint32_t tableIndex = (faultAddress >> 12) & 0x3FFu;

    uint32_t *directory = (uint32_t *)PHYS_TO_VIRT(directoryPhysical);
    uint32_t pde = directory[directoryIndex];

    vgaWrite("Active CR3: ");
    vgaPutUnsignedInt(directoryPhysical);
    vgaWrite("\nPDE index: ");
    vgaPutUnsignedInt(directoryIndex);
    vgaWrite("\nPDE value: ");
    vgaPutUnsignedInt(pde);
    vgaWrite("\n");

    if (!(pde & PAGE_PRESENT)) {
        vgaWrite("PDE is NOT PRESENT; no page table to inspect.\n");
        return;
    }

    uint32_t tablePhysical = pde & ADDRESS_MASK;

    uint32_t *table = (uint32_t *)PHYS_TO_VIRT(tablePhysical);

    uint32_t pte = table[tableIndex];

    vgaWrite("PTE index: ");
    vgaPutUnsignedInt(tableIndex);
    vgaWrite("\nPTE value: ");
    vgaPutUnsignedInt(pte);
    vgaWrite("\n");

    vgaWrite("PDE permissions: ");
    vgaWrite((pde & PAGE_WRITABLE) ? "Writable " : "Read-only ");
    vgaWrite((pde & PAGE_USER) ? "User\n" : "Supervisor\n");

    if (!(pte & PAGE_PRESENT)) {
        vgaWrite("PTE is NOT PRESENT.\n");
        return;
    }

    vgaWrite("PTE permissions: ");
    vgaWrite((pte & PAGE_WRITABLE) ? "Writable " : "Read-only ");
    vgaWrite((pte & PAGE_USER) ? "User\n" : "Supervisor\n");
}

void exceptionHandlerPageFault(uint32_t faultAddress, uint32_t errorCode){

    int present = (errorCode >> 0) & 1;
    int write   = (errorCode >> 1) & 1;
    int user    = (errorCode >> 2) & 1;
    int reserved = (errorCode >> 3) & 1;
    int instruction = (errorCode >> 4) & 1;

    vgaWrite("\nPAGE FAULT\n");

    vgaWrite("Fault address: 0x");
    vgaPutUnsignedInt(faultAddress);
    vgaWrite("\n");

    vgaWrite("Error code: 0x");
    vgaPutUnsignedInt(errorCode);
    vgaWrite("\n");

    vgaWrite("Cause: ");
    vgaWrite(present ? "Protection violation\n"
                     : "Page not present\n");

    vgaWrite("Access: ");
    vgaWrite(write ? "Write\n" : "Read\n");

    vgaWrite("Origin: ");
    vgaWrite(user ? "User mode\n" : "Supervisor mode\n");

    if (reserved)
        vgaWrite("Detail: Reserved paging bit violation\n");

    if (instruction)
        vgaWrite("Detail: Instruction-fetch fault\n");
    
    vgaWrite("\nPAGE TABLE INSPECTION\n");
    inspectPageFault(faultAddress);

    vgaWrite("\n");
    vgaWrite("CPU PANIC");

    while(1){
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}