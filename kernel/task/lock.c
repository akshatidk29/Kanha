#include "lock.h"
#include "scheduler.h"

void spinLockInit(SpinLock* lock) {
    lock->flag = 0;
}

void spinLockAcquire(SpinLock* lock) {
    while (__sync_lock_test_and_set(&lock->flag, 1))
        yield();
}

void spinLockRelease(SpinLock* lock) {
    __sync_lock_release(&lock->flag);
}
