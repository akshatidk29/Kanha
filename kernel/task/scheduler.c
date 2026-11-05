#include <stddef.h>

#include "hardware/tss.h"
#include "task/lock.h"
#include "task/thread.h"
#include "mm/paging.h"
#include "task/process.h"
#include "task/scheduler.h"

static struct Thread *currentThread = NULL;
static struct Thread *readyHead = NULL;
static struct Thread *readyTail = NULL;

static struct Thread* sleepHead = NULL;
static struct Thread* sleepTail = NULL;

volatile uint32_t tickCount = 0;

void schedulerInitialize(void) {
    currentThread = threadInitialize();
    processInitialize();
}

struct Thread *scheduledThread(void) {
    return currentThread;
}

void readyThreadsPush(struct Thread *thread) {
    
    uint32_t flags = disableInterrupts();
    
    thread->schedNext = NULL;
    thread->state = THREAD_READY;

    if (!readyHead) {
        readyHead = thread;
        readyTail = thread;
    } else {
        readyTail->schedNext = thread;
        readyTail = thread;
    }

    restoreInterrupts(flags);
}

void sleepThreadsPush(struct Thread *thread) {
    
    uint32_t flags = disableInterrupts();
    
    thread->schedNext = NULL;

    if (!sleepHead) {
        sleepHead = thread;
        sleepTail = thread;
    } else {
        sleepTail->schedNext = thread;
        sleepTail = thread;
    }

    restoreInterrupts(flags);
}

static struct Thread *readyThreadsPop(void) {

    uint32_t flags = disableInterrupts();
    
    while (readyHead) {
        struct Thread *t = readyHead;
        readyHead = readyHead->schedNext;

        if (!readyHead) 
            readyTail = NULL;

        t->schedNext = NULL;

        if (t->state != THREAD_TERMINATED) {
            restoreInterrupts(flags);
            return t;
        }
    }

    restoreInterrupts(flags);
    return NULL;
}

void yield(void) {

    struct Thread *next = readyThreadsPop();
    if (!next) 
        return;

    struct Thread *prev = currentThread;
    if (prev->state == THREAD_RUNNING) {
        readyThreadsPush(prev);
    } else if (prev->state == THREAD_SLEEPING) {
        sleepThreadsPush(prev);
    }

    next->state = THREAD_RUNNING;
    currentThread = next;

    if (next->kernelStack) {
        tssSetKernelStack((uint32_t)(next->kernelStack) + STACK_SIZE);
    }

    // Switch AddressSpace 
    if (next->process != prev->process) {
        switchAddressSpace(&next->process->addressSpace);
    }
     
    switchContext(&prev->stackPointer, next->stackPointer);
}


void schedulerTimer(void){

    tickCount++;

    struct Thread* current = sleepHead;
    struct Thread* previous = NULL;

    while(current != NULL){

        if(current->sleepTime > 0){
            current->sleepTime--;
        }

        if(current->sleepTime == 0){

            struct Thread* wakeUpThread = current;

            if(previous != NULL){
                previous->schedNext = current->schedNext;
            }else{
                sleepHead = current->schedNext;
            }

            if(current == sleepTail){
                sleepTail = previous;
            }

            current = current->schedNext;
            if (wakeUpThread->state != THREAD_TERMINATED) {
                readyThreadsPush(wakeUpThread);
            }
        } else {
            previous = current;
            current = current->schedNext;
        }
    }

    if(tickCount % TIME_QUANTUM == 0){
        yield();
    }
}