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

