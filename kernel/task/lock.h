#ifndef KANHA_LOCK_H
#define KANHA_LOCK_H

#include <stdint.h>

static inline uint32_t disableInterrupts(void) {
    uint32_t flags;
    __asm__ volatile("pushf; pop %0; cli" : "=r"(flags) : : "memory");
    return flags;   
}

static inline void enableInterrupts(uint32_t flags) {
    __asm__ volatile("push %0; popfd" : :"r"(flags) : "memory");
}

typedef struct SpinLock {
    volatile uint8_t flag;
} SpinLock;

void spinLockInit(SpinLock* lock);
void spinLockAcquire(SpinLock* lock);
void spinLockRelease(SpinLock* lock);

#endif