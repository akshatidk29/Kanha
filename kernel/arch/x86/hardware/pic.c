#include "hardware/io.h"
#include "hardware/pic.h"

void picInitialize(void)
{
    uint8_t masterMask = inB(PIC1_DATA);
    uint8_t slaveMask  = inB(PIC2_DATA);

    // ICW1: Start initialization sequence
    outB(PIC1_COMMAND, 0x11);
    outB(PIC2_COMMAND, 0x11);

    // ICW2: Set interrupt vector offsets
    outB(PIC1_DATA, PIC1_OFFSET);
    outB(PIC2_DATA, PIC2_OFFSET);

    // ICW3: Tell master/slave how they are connected.
    outB(PIC1_DATA, 0x04);
    outB(PIC2_DATA, 0x02);

    // ICW4: Operate in 8086 mode
    outB(PIC1_DATA, 0x01);
    outB(PIC2_DATA, 0x01);

    // Restore interrupt masks
    outB(PIC1_DATA, masterMask);
    outB(PIC2_DATA, slaveMask);
}

void picSetMask(uint8_t irq){

    int port;
    if(irq < 8){
        port = PIC1_DATA;
    }else{
        port = PIC2_DATA;
        irq -= 8;
    }

    uint8_t mask = inB(port);
    mask = mask | (1 << irq);
    outB(port, mask);
}

void picClearMask(uint8_t irq){

    uint8_t port;
    if(irq < 8){
        port = PIC1_DATA;
    }else{
        port = PIC2_DATA;
        irq -= 8;
    }

    uint8_t mask = inB(port);
    mask = mask & ~(1 << irq);
    outB(port, mask);
}

void picSendEOI(uint8_t irq){

    if(irq >= 8){
        outB(PIC2_COMMAND, 0x20);       // Send to Slave
    }
    outB(PIC1_COMMAND, 0x20);           // Send to Master
}