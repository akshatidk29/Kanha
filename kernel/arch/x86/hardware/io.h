#ifndef KANHA_IO_H
#define KANHA_IO_H

#include <stdint.h>

static inline void outB(uint16_t port, uint8_t value)           // Send to Hardware Port
{
    __asm__ volatile (                  // Tells the Compiler to Volatilely Emitt Raw Assembly Instructions
        "outb %0, %1"                   // The Instructions to be Emitted
        :                               // Output
        : "a"(value), "Nd"(port)        // Input
    );
}

static inline uint8_t inB(uint16_t port)                        // Read from Hardware Port
{
    uint8_t value;

    __asm__ volatile (
        "inb %1, %0"                    // Reading from the Port
        : "=a"(value)                   // Output
        : "Nd"(port)                    // Input
    );

    return value;
}

static inline void outW(uint16_t port, uint16_t value)
{
    __asm__ volatile (
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

static inline uint16_t inW(uint16_t port)
{
    uint16_t value;

    __asm__ volatile (
        "inw %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

static inline void insW(uint16_t port, void *addr, uint32_t count)
{
    __asm__ volatile (
        "cld; rep insw"
        : "+D"(addr), "+c"(count)
        : "d"(port)
        : "memory"
    );
}

static inline void outsW(uint16_t port, const void *addr, uint32_t count)
{
    __asm__ volatile (
        "cld; rep outsw"
        : "+S"(addr), "+c"(count)
        : "d"(port)
        : "memory"
    );
}

#endif