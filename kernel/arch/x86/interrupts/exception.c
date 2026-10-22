#include "io.h"
#include "pic.h"
#include "vga.h"
#include "exception.h"

void exceptionHandlerDivideByZero(void){

    vgaWrite("ERROR: DIVIDE BY ZERO\n");
    vgaWrite("CPU PANIC\n");

    while(1){
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}   

void exceptionHandlerPageFault(uint32_t faultAddress, uint32_t errorCode){

    vgaWrite("ERROR: PAGE FAULT\n");
    vgaWrite("CPU PANIC\n");

    int pBit = (errorCode >> 0) & 1;
    int wBit = (errorCode >> 1) & 1;
    int uBit = (errorCode >> 2) & 1;
    int sBit = (errorCode >> 3) & 1;
    int iBit = (errorCode >> 4) & 1;

    vgaWrite("Page fault at 0x");
    vgaPutUnsignedInt(faultAddress);
    vgaWrite("\n");
    
    vgaWrite("Error code: 0x");
    vgaPutUnsignedInt(errorCode);
    vgaWrite("\n");

    vgaWrite("\nCause: ");
    if (pBit)
        vgaWrite("Protection violation\n");
    else
        vgaWrite("Page not present\n");

    vgaWrite("Access: ");
    if (wBit)
        vgaWrite("Write\n");
    else
        vgaWrite("Read\n");

    vgaWrite("Privilege: ");
    if (uBit)
        vgaWrite("User mode\n");
    else
        vgaWrite("Supervisor mode\n");

    if (sBit)
        vgaWrite("Reserved paging bit violation\n");
    else if (iBit)
        vgaWrite("Instruction fetch\n");

    vgaWrite("\n");
    vgaWrite("CPU PANIC");

    __asm__ volatile("cli");
    __asm__ volatile("hlt");
}