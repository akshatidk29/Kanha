#ifndef KANHA_SCHEDULER_H
#define KANHA_SCHEDULER_H

#include "thread.h"

void schedulerInitialize(void);
void readyThreadsPush(struct Thread *thread);

struct Thread *scheduledThread(void);

void yield(void);

#endif