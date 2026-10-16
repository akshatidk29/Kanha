#include "exception.h"
#include "vga.h"
#include "pic.h"
#include "io.h"

void exception_handler_divide_by_zero(void){

    vga_write("ERROR: DIVIDE BY ZERO\n");
    vga_write("CPU PANIC\n");

    __asm__ volatile("cli");
    __asm__ volatile("hlt");
}   

void exception_handler_page_fault(uint32_t faultAddress, uint32_t errorCode){

    vga_write("ERROR: PAGE FAULT\n");
    vga_write("CPU PANIC\n");

    int pBit = (errorCode >> 0) & 1;
    int wBit = (errorCode >> 1) & 1;
    int uBit = (errorCode >> 2) & 1;
    int sBit = (errorCode >> 3) & 1;
    int iBit = (errorCode >> 4) & 1;

    vga_write("Page fault at 0x");
    vga_put_int(faultAddress);
    vga_write("\n");
    
    vga_write("Error code: 0x");
    vga_put_int(errorCode);
    vga_write("\n");

    vga_write("\nCause: ");
    if (pBit)
        vga_write("Protection violation\n");
    else
        vga_write("Page not present\n");

    vga_write("Access: ");
    if (wBit)
        vga_write("Write\n");
    else
        vga_write("Read\n");

    vga_write("Privilege: ");
    if (uBit)
        vga_write("User mode\n");
    else
        vga_write("Supervisor mode\n");

    if (sBit)
        vga_write("Reserved paging bit violation\n");
    else if (iBit)
        vga_write("Instruction fetch\n");

    vga_write("\n");
    vga_write("CPU PANIC");

    __asm__ volatile("cli");
    __asm__ volatile("hlt");
}