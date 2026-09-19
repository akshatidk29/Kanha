#include "vga.h"
#include "idt.h"
#include "io.h"
#include "pic.h"
#include "timer.h"

volatile uint32_t ticks;

extern void enable_interrupts(void);

void kernel_main(void)
{
    vga_initialize();           // Initialize VGA
    idt_initialize();           // Initialize IDT
    pic_initialize();           // Initialize PIC
    pit_initialize(1000);       // Initialize PIT with 1KHz frequency

    ticks = 0;

    pic_clear_mask(0);          // Unmask Timer
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

    vga_put_char('\n');
    vga_put_int(ticks);
    vga_put_char('\n');

    for(int i = 65; i < 75; i++){
        char c = i;
        vga_put_char(c);
        vga_put_char('\n'); 

        for(int j = 0; j < 1e7; j++){
            j++;
            j--;
        }
    }

    vga_put_char('\n');
    vga_put_int(100000);
    vga_put_char('\n');

    while (1) {}
}