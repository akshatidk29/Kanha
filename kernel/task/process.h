#ifndef KANHA_PROCESS_H
#define KANHA_PROCESS_H

#include <stdint.h>
#include <stddef.h>

#include "task/thread.h"
#include "mm/addressSpace.h"

enum ProcessState {
    PROCESS_CREATED,
    PROCESS_RUNNING,
    PROCESS_TERMINATED
};

struct Process {
    uint32_t pid;
    enum ProcessState state;
    int exitCode;

    struct AddressSpace addressSpace;
    struct Thread *mainThread;

    uint32_t entryPoint;
    uint32_t userStackTop;
};


void processInitialize(void);
struct Process *currentProcess(void);

struct Process *processCreate(const uint8_t *code, uint32_t codeSize);
void processExit(int exitCode);

#endif
