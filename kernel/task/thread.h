#ifndef KANHA_THREAD_H
#define KANHA_THREAD_H

#include <stdint.h>

#define STACK_SIZE 4096u

enum ThreadState{
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_SLEEPING,
    THREAD_TERMINATED,
};

struct Thread{

    uint32_t tid;
    enum ThreadState state;
    
    uint32_t sleepTime;
    
    void *kernelStack;
    uint32_t *stackPointer;
    struct Thread* next;
};



struct Thread* threadInitialize(void);
struct Thread* threadCreate(void(*func)(void*), void* arg);

void threadExit(void);
void threadSleep(uint32_t tms);

extern void switchContext(uint32_t **oldEsp, uint32_t *newEsp);

#endif