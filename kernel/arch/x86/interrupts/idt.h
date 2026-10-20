#include <stdint.h>

#ifndef KANHA_IDT_H
#define KANHA_IDT_H


// __attribute__((packed)) ensures that the Compiler Don't add Padding In These Structures For The Sake Of Alignment...

struct IDTEntry{
    
    uint16_t offsetLow;
    uint16_t selector;
    uint8_t zero;
    uint8_t typeAttributes;
    uint16_t offsetHigh;
} __attribute__((packed));

struct IDTPtr{

    uint16_t limit;
    uint32_t base;
} __attribute__((packed));



extern void isr0(void);
extern void idtLoad(struct IDTPtr* idtPtr);

void idtInitialize(void);


// With Static, assertion is done using compile time

_Static_assert(sizeof(struct IDTEntry) == 8,
               "IDT Entry Must Be 8 bytes");

_Static_assert(sizeof(struct IDTPtr) == 6,
               "IDT Pointer Must Be 6 bytes");

#endif