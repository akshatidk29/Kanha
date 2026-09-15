#include "idt.h"
#include "vga.h"

static struct idt_ptr idtr;
static struct idt_entry idt[256];

extern void isr0(void);
extern void idt_load(struct idt_ptr* idt_ptr);

static void idt_set_gate(int vector, uint32_t handler, uint16_t selector, uint8_t attributes){

    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].type_attributes = attributes;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}

void divide_by_zero_handler(void){

    vga_write("DIVIDE_BY_ZERO ERROR\n");
    while (1) {}
}


void idt_initialize(void){
    
    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint32_t)idt;
    
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    
    idt_load(&idtr);
}

