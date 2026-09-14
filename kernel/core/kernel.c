#include "../drivers/vga/vga.h"

void kernel_main(void)
{
    vga_initialize();

    vga_put_char('K');
    vga_put_char('a');
    vga_put_char('n');
    vga_put_char('h');
    vga_put_char('a');

    while (1) {}
}