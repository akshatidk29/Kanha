#ifndef KANHA_IO_H
#define KANHA_IO_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value)           // Send to Hardware Port
{
    __asm__ volatile (                  // Tells the Compiler to Volatilely Emitt Raw Assembly Instructions
        "outb %0, %1"                   // The Instructions to be Emitted
        :                               // Output
        : "a"(value), "Nd"(port)        // Input
    );
}

static inline uint8_t inb(uint16_t port)                        // Read from Hardware Port
{
    uint8_t value;

    __asm__ volatile (
        "inb %1, %0"                    // Reading from the Port
        : "=a"(value)                   // Output
        : "Nd"(port)                    // Input
    );

    return value;
}

#endif