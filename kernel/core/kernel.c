#include "vga.h"
#include "idt.h"
#include "io.h"
#include "pic.h"

extern void enable_interrupts(void);

void kernel_main(void)
{
    vga_initialize();
    idt_initialize();
    pic_initialize();

    pic_set_mask(0);            // Mask Timer
    pic_clear_mask(1);          // Unmask Keyboard
    
    enable_interrupts();

    // int x = 5 / 0;

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