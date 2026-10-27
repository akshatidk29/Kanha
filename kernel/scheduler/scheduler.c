#include <stddef.h>

#include "thread.h"
#include "scheduler.h"

static struct Thread *currentThread = NULL;
static struct Thread *readyHead = NULL;
static struct Thread *readyTail = NULL;

volatile uint32_t tickCount = 0;

void schedulerInitialize(void) {
    currentThread = threadInitialize();
}

struct Thread *scheduledThread(void) {
    return currentThread;
}

void readyThreadsPush(struct Thread *thread) {

    thread->next = NULL;
    thread->state = THREAD_READY;

    if (!readyHead) {
        readyHead = thread;
        readyTail = thread;
    } else {
        readyTail->next = thread;
        readyTail = thread;
    }
}

static struct Thread *readyThreadsPop(void) {

    if (!readyHead) 
        return NULL;

    struct Thread *t = readyHead;
    readyHead = readyHead->next;

    if (!readyHead) 
        readyTail = NULL;

    t->next = NULL;
    return t;
}

void yield(void) {

    struct Thread *next = readyThreadsPop();

    if (!next) 
        return;

    struct Thread *prev = currentThread;
    if (prev->state == THREAD_RUNNING) {
        readyThreadsPush(prev);
    }

    next->state = THREAD_RUNNING;
    currentThread = next;

    switchContext(&prev->stackPointer, next->stackPointer);
}

void schedulerTimer(){
    tickCount++;
    if(tickCount % TIME_QUANTUM == 0){
        yield();
    }
}