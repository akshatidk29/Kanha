#include <stdint.h>

#ifndef KANHA_IDT_H
#define KANHA_IDT_H


// __attribute__((packed)) ensures that the Compiler Don't add Padding In These Structures For The Sake Of Alignment...

struct idt_entry{
    
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attributes;
    uint16_t offset_high;
} __attribute__((packed));


struct idt_ptr{

    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void idt_initialize(void);


// With Static, assertion is done using compile time

_Static_assert(sizeof(struct idt_entry) == 8,
               "IDT Entry Must Be 8 bytes");

_Static_assert(sizeof(struct idt_ptr) == 6,
               "IDT Pointer Must Be 6 bytes");

#endif