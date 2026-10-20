#include "idt.h"
#include "vga.h"

static struct IDTPtr idtr;
static struct IDTEntry idt[256];

extern void isr0(void);
extern void isr14(void);
extern void isr32(void);
extern void isr33(void);
extern void isr44(void);

extern void idtLoad(struct IDTPtr* idtPtr);

static void idtSetGate(int vector, uint32_t handler, uint16_t selector, uint8_t attributes){

    idt[vector].offsetLow = handler & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].typeAttributes = attributes;
    idt[vector].offsetHigh = (handler >> 16) & 0xFFFF;
}

void idtInitialize(void){
    
    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint32_t)idt;
    
    idtSetGate(0, (uint32_t)isr0, 0x08, 0x8E);
    idtSetGate(14, (uint32_t)isr14, 0x08, 0x8E);
    idtSetGate(32, (uint32_t)isr32, 0x08, 0x8E);  
    idtSetGate(33, (uint32_t)isr33, 0x08, 0x8E);
    idtSetGate(44, (uint32_t)isr44, 0x08, 0x8E);
    
    idtLoad(&idtr);
}

