#ifndef KANHA_LOCK_H
#define KANHA_LOCK_H

#include <stdint.h>

typedef struct SpinLock {
    volatile uint8_t flag;
} SpinLock;

void spinLockInit(SpinLock* lock);
void spinLockAcquire(SpinLock* lock);
void spinLockRelease(SpinLock* lock);

#endif