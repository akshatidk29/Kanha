#ifndef KANHA_SCHEDULER_H
#define KANHA_SCHEDULER_H

#include "task/thread.h"

#define TIME_QUANTUM    100              // Time Quantum

void schedulerTimer();
void schedulerInitialize(void);
void readyThreadsPush(struct Thread *thread);

struct Thread *scheduledThread(void);

void yield(void);

#endif