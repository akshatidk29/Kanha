#ifndef KANHA_PROCESS_H
#define KANHA_PROCESS_H

#define MAX_PROCESSES 1024
#define MAX_FD        16

#include <stdint.h>
#include <stddef.h>

#include "task/thread.h"
#include "mm/addressSpace.h"

struct VfsNode;

struct FileDescriptor {
    struct VfsNode *node; 
    uint32_t offset;      
    int flags;           
};

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
    struct FileDescriptor files[MAX_FD];
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

int processKill(int pid, int exitCode);
int processGetForeground(void);
void processSetForeground(int pid);

#endif
