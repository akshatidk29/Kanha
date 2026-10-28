#include "heap.h"
#include "thread.h"
#include "kprintf.h"
#include "scheduler.h"
#include "process.h"

#define BOOT_STACK_TOP (0xC0000000u + 0x90000u)

static uint32_t nextTid = 0;

static void threadEntry(void (*func)(void *), void *arg) {
    func(arg);
    threadExit();
}

// kernelMain as Thread 0
struct Thread* threadInitialize(void) {     

    struct Thread *mainThread = (struct Thread *)kMalloc(sizeof(struct Thread));

    mainThread->tid = nextTid++;
    mainThread->state = THREAD_RUNNING;
    mainThread->sleepTime = 0;
    mainThread->kernelStack = (void*)(BOOT_STACK_TOP - STACK_SIZE);
    mainThread->stackPointer = NULL;
    mainThread->next = NULL;

    mainThread->process = NULL;
    return mainThread;
}

struct Thread *threadCreate(void (*func)(void *), void *arg) {

    void *stack = kMalloc(STACK_SIZE);
    if (!stack)
        return NULL;

    struct Thread *thread = (struct Thread *)kMalloc(sizeof(struct Thread));
    if (!thread) {
        kFree(stack);
        return NULL;
    }

    thread->tid = nextTid++;
    thread->sleepTime = 0;
    thread->kernelStack = stack;
    thread->process = currentProcess();
    
    uint32_t *top = (uint32_t *)((uint8_t *)stack + STACK_SIZE);

    *(--top) = (uint32_t)arg;               
    *(--top) = (uint32_t)func;              
    *(--top) = 0;                            // Dummy return address for threadEntry
    *(--top) = (uint32_t)threadEntry;        // Return address for switchContext's 'ret'
    *(--top) = 0;                            // EBP
    *(--top) = 0;                            // EBX
    *(--top) = 0;                            // ESI
    *(--top) = 0;                            // EDI
    *(--top) = 0x202;                        // EFLAGS (Interrupts Enabled)

    thread->stackPointer = top;
    readyThreadsPush(thread);

    return thread;
}

void threadExit(void) {
    
    struct Thread* currentThread = scheduledThread();
    currentThread->state = THREAD_TERMINATED;
    
    __asm__ volatile("sti");
    while (1) {
        yield();
        __asm__ volatile("hlt");
    }
}

void threadSleep(uint32_t tms){

    if(tms != 0){
        struct Thread* currentThread = scheduledThread();
        currentThread->state = THREAD_SLEEPING;
        currentThread->sleepTime = tms;
        yield();
    }
}