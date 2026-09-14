#include "../drivers/vga/vga.h"

void kernel_main(void)
{
    vga_initialize();

    vga_write("Hello, Welcome to Kanha\n");
    vga_write("I'm running on my own Hardware\n");

    while (1) {}
}