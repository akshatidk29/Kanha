#ifndef KANHA_LOCK_H
#define KANHA_LOCK_H

#include <stdint.h>

static inline uint32_t disableInterrupts(void) {
    uint32_t flags;
    __asm__ volatile("pushfl; popl %0; cli" : "=r"(flags) : : "memory");
    return flags;   
}

static inline void restoreInterrupts(uint32_t flags) {
    __asm__ volatile("pushl %0; popfl" : : "r"(flags) : "memory");
}

typedef struct SpinLock {
    volatile uint8_t flag;
} SpinLock;

void spinLockInit(SpinLock* lock);
void spinLockAcquire(SpinLock* lock);
void spinLockRelease(SpinLock* lock);

#endif