#ifndef KANHA_THREAD_H
#define KANHA_THREAD_H

#include <stdint.h>

#define STACK_SIZE 4096u

enum ThreadState{
    THREAD_READY,
    THREAD_TERMINATED,
    THREAD_RUNNING,
};

struct Thread{
    uint32_t tid;
    enum ThreadState state;
    
    void *kernelStack;
    uint32_t *stackPointer;

    struct Thread* next;
};



void threadInitialize(void);
struct Thread* threadCreate(void(*func)(void*), void* arg);

void yield(void);
void threadExit(void);

extern void switchContext(uint32_t **oldEsp, uint32_t *newEsp);

#endif