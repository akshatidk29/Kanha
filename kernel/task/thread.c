#include "heap.h"
#include "thread.h"
#include "printf.h"

static struct Thread *currentThread = NULL;
static struct Thread *readyQueueHead = NULL;
static struct Thread *readyQueueTail = NULL;
static uint32_t nextTid = 0;

static void readyQueuePush(struct Thread *thread) { 

    thread->next = NULL;
    thread->state = THREAD_READY;

    if (!readyQueueHead) {
        readyQueueHead = thread;
        readyQueueTail = thread;
    } else {
        readyQueueTail->next = thread;
        readyQueueTail = thread;
    }
}

static struct Thread *readyQueuePop(void) {

    if (!readyQueueHead)
        return NULL;

    struct Thread *thread = readyQueueHead;
    
    readyQueueHead = readyQueueHead->next;

    if (!readyQueueHead)
        readyQueueTail = NULL;

    thread->next = NULL;
    
    return thread;
}

static void threadEntry(void (*func)(void *), void *arg) {
    func(arg);
    threadExit();
}

// kernelMain as Thread 0
void threadInitialize(void) {     

    struct Thread *mainThread = (struct Thread *)kMalloc(sizeof(struct Thread));

    mainThread->tid = nextTid++;
    mainThread->state = THREAD_RUNNING;
    mainThread->kernelStack = NULL;
    mainThread->stackPointer = NULL;
    mainThread->next = NULL;

    currentThread = mainThread;
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
    thread->kernelStack = stack;

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
    readyQueuePush(thread);

    return thread;
}

void yield(void) {

    struct Thread *next = readyQueuePop();
    
    if (!next)
        return; 

    struct Thread *prev = currentThread;
    if (prev->state == THREAD_RUNNING) {
        readyQueuePush(prev);
    }

    next->state = THREAD_RUNNING;
    currentThread = next;

    switchContext(&prev->stackPointer, next->stackPointer);
}

void threadExit(void) {
    currentThread->state = THREAD_TERMINATED;
    yield();
    while (1) {}
}
