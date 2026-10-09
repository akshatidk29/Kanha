#include "vga.h"
#include "idt.h"
#include "io.h"
#include "pic.h"
#include "timer.h"
#include "e820.h"


volatile uint32_t ticks;

extern void enable_interrupts(void);

void printE820(struct bootInfo *bootInfo){

    struct e820Entry *entries =
        (struct e820Entry *)(uintptr_t)bootInfo->e820Address;

    vga_write("E820 Memory Map\n");

    for (uint32_t i = 0; i < bootInfo->e820Count; i++) {

        vga_write("BASE: ");
        vga_put_int64(entries[i].base);

        vga_write(" LENGTH: ");
        vga_put_int64(entries[i].length); 

        vga_write(" TYPE: ");
        vga_put_int64(entries[i].type);  

        vga_write(" ATTRIBUTES: ");
        vga_put_int64(entries[i].attributes);

        vga_write("\n");
    }
}

void kernel_main(struct bootInfo *bootInfo)
{
    vga_initialize();           // Initialize VGA
    idt_initialize();           // Initialize IDT
    pic_initialize();           // Initialize PIC
    pit_initialize(1000);       // Initialize PIT with 1KHz frequency

    ticks = 0;

    pic_clear_mask(0);          // Unmask Timer
    pic_clear_mask(1);          // Unmask Keyboard
    //pic_clear_mask(12);         // Unmask Mouse
    
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
    printE820(bootInfo);
    vga_put_char('\n');


    while (1) {}
}