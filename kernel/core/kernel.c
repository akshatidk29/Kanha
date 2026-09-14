#include "../drivers/vga/vga.h"

void kernel_main(void)
{
    vga_initialize();

    for(int i = 0; i < 200; i++){
        char c = i;
        vga_put_char(c);
        vga_put_char('\n'); 

        for(int j = 0; j < 1e7; j++){
            j++;
            j--;
        }
    }

    while (1) {}
}