#ifndef KANHA_E820_H
#define KANHA_E820_H

#include <stdint.h>

#define E820_MAX_ENTRIES 32

struct e820Entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t attributes;
} __attribute__((packed));

struct bootInfo {
    uint32_t e820Count;
    uint32_t e820Address;
};

#endif