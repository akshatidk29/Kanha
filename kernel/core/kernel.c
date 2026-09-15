#include "vga.h"
#include "idt.h"

void kernel_main(void)
{
    vga_initialize();
    idt_initialize();

    int x = 5 / 0;

    for(int i = 65; i < 75; i++){
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