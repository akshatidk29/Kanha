#include "idt.h"
#include "vga.h"

static struct idt_ptr idtr;
static struct idt_entry idt[256];

extern void isr0(void);
extern void isr32(void);
extern void isr33(void);
extern void isr44(void);

extern void idt_load(struct idt_ptr* idt_ptr);

static void idt_set_gate(int vector, uint32_t handler, uint16_t selector, uint8_t attributes){

    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].type_attributes = attributes;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_initialize(void){
    
    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint32_t)idt;
    
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(32, (uint32_t)isr32, 0x08, 0x8E);  
    idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)isr44, 0x08, 0x8E);
    
    idt_load(&idtr);
}

