#ifndef KANHA_PROCESS_H
#define KANHA_PROCESS_H

#define MAX_PROCESSES 1024

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
    struct Thread *threadList;
    uint32_t threadCount;
    uint32_t entryPoint;
    uint32_t userStackTop;

    struct Process *parent;
};


struct SyscallFrame;

void processInitialize(void);
struct Process *currentProcess(void);

struct Process *processFindByPid(uint32_t pid);
struct Process *processCreate(const uint8_t *code, uint32_t codeSize);
struct Process *processFork(struct SyscallFrame *parentFrame);
int processExec(struct SyscallFrame *frame, const char *path, const char **argv);
int processWaitPid(int pid, int *status, int options);

void processExit(int exitCode);

#endif
