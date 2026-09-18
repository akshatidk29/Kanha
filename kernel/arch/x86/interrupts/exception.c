#include "exception.h"
#include "vga.h"
#include "pic.h"
#include "io.h"

void error_handler_divide_by_zero(void){

    vga_write("ERROR: DIVIDE BY ZERO\n");
    vga_write("CPU PANIC\n");

    while(1){};
}

