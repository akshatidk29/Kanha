#include "io.h"
#include "pic.h"

void pic_initialize(void)
{
    uint8_t masterMask = inb(PIC1_DATA);
    uint8_t slaveMask  = inb(PIC2_DATA);

    // ICW1: Start initialization sequence
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // ICW2: Set interrupt vector offsets
    outb(PIC1_DATA, PIC1_OFFSET);
    outb(PIC2_DATA, PIC2_OFFSET);

    // ICW3: Tell master/slave how they are connected.
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // ICW4: Operate in 8086 mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Restore interrupt masks
    outb(PIC1_DATA, masterMask);
    outb(PIC2_DATA, slaveMask);
}

void pic_set_mask(uint8_t irq){

    int port;
    if(irq < 8){
        port = PIC1_DATA;
    }else{
        port = PIC2_DATA;
        irq -= 8;
    }

    uint8_t mask = inb(port);
    mask = mask | (1 << irq);
    outb(port, mask);
}

void pic_clear_mask(uint8_t irq){

    uint8_t port;
    if(irq < 8){
        port = PIC1_DATA;
    }else{
        port = PIC2_DATA;
        irq -= 8;
    }

    uint8_t mask = inb(port);
    mask = mask & ~(1 << irq);
    outb(port, mask);
}

void pic_send_eoi(uint8_t irq){

    if(irq >= 8){
        outb(PIC2_COMMAND, 0x20);       // Send to Slave
    }
    outb(PIC1_COMMAND, 0x20);           // Send to Master
}