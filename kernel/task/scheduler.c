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
    
    thread->next = NULL;
    thread->state = THREAD_READY;

    if (!readyHead) {
        readyHead = thread;
        readyTail = thread;
    } else {
        readyTail->next = thread;
        readyTail = thread;
    }

    restoreInterrupts(flags);
}

void sleepThreadsPush(struct Thread *thread) {
    
    uint32_t flags = disableInterrupts();
    
    thread->next = NULL;

    if (!sleepHead) {
        sleepHead = thread;
        sleepTail = thread;
    } else {
        sleepTail->next = thread;
        sleepTail = thread;
    }

    restoreInterrupts(flags);
}

static struct Thread *readyThreadsPop(void) {

    uint32_t flags = disableInterrupts();
    
    if (!readyHead) {
        restoreInterrupts(flags);
        return NULL;
    }

    struct Thread *t = readyHead;
    readyHead = readyHead->next;

    if (!readyHead) 
        readyTail = NULL;

    t->next = NULL;

    restoreInterrupts(flags);
    return t;
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
                previous->next = current->next;
            }else{
                sleepHead = current->next;
            }

            if(current == sleepTail){
                sleepTail = previous;
            }

            current = current->next;
            readyThreadsPush(wakeUpThread);
        } else {
            previous = current;
            current = current->next;
        }
    }

    if(tickCount % TIME_QUANTUM == 0){
        yield();
    }
}