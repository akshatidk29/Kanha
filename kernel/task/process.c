#include "mm/pmm.h"
#include "mm/paging.h"
#include "mm/heap.h"
#include "task/process.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "fs/vfs.h"
#include "syscall/syscall.h"
#include "task/scheduler.h"
#include "task/lock.h"

extern void jumpToUserMode(uint32_t userIp, uint32_t userSp);

static uint32_t nextPid = 1;
static struct Process kernelProcess;

static volatile int processCount = 0;
struct Process *processTable[MAX_PROCESSES];

SpinLock processTableLock;

static int addProcess(struct Process *proc) {
    if (!proc)
        return -1;

    spinLockAcquire(&processTableLock);
    if (processCount >= MAX_PROCESSES) {
        spinLockRelease(&processTableLock);
        return -1;
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i] == NULL) {
            processTable[i] = proc;
            processCount++;
            spinLockRelease(&processTableLock);
            return 0;
        }
    }

    spinLockRelease(&processTableLock);
    return -1;
}

static int removeProcess(struct Process *proc) {
    if (!proc)
        return -1;

    spinLockAcquire(&processTableLock);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i] == proc) {
            processTable[i] = NULL;
            processCount--;
            spinLockRelease(&processTableLock);
            return 0;
        }
    }

    spinLockRelease(&processTableLock);
    return -1;
}

struct Process *processFindByPid(uint32_t pid) {
    spinLockAcquire(&processTableLock);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i] && processTable[i]->pid == pid) {
            struct Process *proc = processTable[i];
            spinLockRelease(&processTableLock);
            return proc;
        }
    }
    spinLockRelease(&processTableLock);
    return NULL;
}

void processInitialize(void) {
    spinLockInit(&processTableLock);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processTable[i] = NULL;
    }
    processCount = 0;

    kernelProcess.pid = 0;
    kernelProcess.state = PROCESS_RUNNING;
    kernelProcess.exitCode = 0;     

    kernelProcess.addressSpace.pageDirectory = getKernelPageDirectory();
    kernelProcess.entryPoint = 0;
    kernelProcess.userStackTop = 0;

    struct Thread *mainThread = scheduledThread();
    kernelProcess.mainThread = mainThread;
    kernelProcess.threadList = mainThread;
    kernelProcess.threadCount = 1;
    kernelProcess.parent = NULL;

    if (mainThread) {
        mainThread->process = &kernelProcess;
        mainThread->procNext = NULL;
    }

    addProcess(&kernelProcess);
}

static void processEntry(void *arg) { 
    struct Process *proc = (struct Process *)arg;

    switchAddressSpace(&proc->addressSpace);
    jumpToUserMode(proc->entryPoint, proc->userStackTop);
}

struct Process *processCreate(const uint8_t *code, uint32_t codeSize) {
    if (!code || codeSize == 0 || codeSize > PAGE_SIZE) {
        return NULL;
    }

    spinLockAcquire(&processTableLock);
    if (processCount >= MAX_PROCESSES) {
        spinLockRelease(&processTableLock);
        return NULL;
    }
    spinLockRelease(&processTableLock);

    struct Process *proc = (struct Process *)kMalloc(sizeof(struct Process));
    if (!proc) {
        return NULL;
    }
    memset(proc, 0, sizeof(struct Process));

    proc->pid = nextPid++;
    proc->state = PROCESS_CREATED;
    proc->exitCode = 0;
    proc->parent = currentProcess();

    // Private AddressSpace
    if (createAddressSpace(&proc->addressSpace) != 0) {
        kFree(proc);
        return NULL;
    }

    // Map User Code Page
    uint32_t codePhys = pmmAllocateFrame();
    if (!codePhys) {
        destroyAddressSpace(&proc->addressSpace);
        kFree(proc);
        return NULL;
    }

    if (addressSpaceMapPage(&proc->addressSpace, 0x00400000, codePhys, PAGE_USER | PAGE_WRITABLE) != 0) {
        pmmDeallocateFrame(codePhys);
        destroyAddressSpace(&proc->addressSpace);
        kFree(proc);
        return NULL;
    }

    // Copy user code into the physical frame
    uint8_t *codeVirt = (uint8_t *)PHYS_TO_VIRT(codePhys);
    for (uint32_t i = 0; i < codeSize; i++) {
        codeVirt[i] = code[i];
    }
    proc->entryPoint = 0x00400000;

    // Map User Stack Page
    uint32_t stackPhys = pmmAllocateFrame();
    if (!stackPhys) {
        pmmDeallocateFrame(codePhys);
        destroyAddressSpace(&proc->addressSpace);
        kFree(proc);
        return NULL;
    }

    if (addressSpaceMapPage(&proc->addressSpace, 0x00800000, stackPhys, PAGE_USER | PAGE_WRITABLE) != 0) {
        pmmDeallocateFrame(stackPhys);
        pmmDeallocateFrame(codePhys);
        destroyAddressSpace(&proc->addressSpace);
        kFree(proc);
        return NULL;
    }
    uint32_t userStackTop = 0x00800000 + PAGE_SIZE - 16;
    uint8_t *kStackBase = (uint8_t *)PHYS_TO_VIRT(stackPhys);
    userStackTop -= 4;
    *(uint32_t *)(kStackBase + (userStackTop - 0x00800000)) = 0; // argv = NULL
    userStackTop -= 4;
    *(uint32_t *)(kStackBase + (userStackTop - 0x00800000)) = 0; // argc = 0
    proc->userStackTop = userStackTop;

    // Create the main thread for this process
    struct Thread *thread = threadCreate(processEntry, proc);
    if (!thread) {
        pmmDeallocateFrame(stackPhys);
        pmmDeallocateFrame(codePhys);
        destroyAddressSpace(&proc->addressSpace);
        kFree(proc);
        return NULL;
    }

    // Detach from currentProcess if threadCreate attached it
    struct Process *curr = currentProcess();
    if (curr && curr != proc) {
        if (curr->threadList == thread) {
            curr->threadList = thread->procNext;
        } else {
            struct Thread *p = curr->threadList;
            while (p && p->procNext != thread) {
                p = p->procNext;
            }
            if (p) {
                p->procNext = thread->procNext;
            }
        }
        if (curr->threadCount > 0) {
            curr->threadCount--;
        }
    }

    thread->process = proc;
    thread->procNext = NULL;
    proc->mainThread = thread;
    proc->threadList = thread;
    proc->threadCount = 1;
    proc->state = PROCESS_RUNNING;

    if (addProcess(proc) != 0) {
        thread->state = THREAD_TERMINATED;
        pmmDeallocateFrame(stackPhys);
        pmmDeallocateFrame(codePhys);
        destroyAddressSpace(&proc->addressSpace);
        kFree(proc);
        return NULL;
    }

    return proc;
}

struct Process *processFork(struct SyscallFrame *parentFrame) {
    if (!parentFrame)
        return NULL;

    spinLockAcquire(&processTableLock);
    if (processCount >= MAX_PROCESSES) {
        spinLockRelease(&processTableLock);
        return NULL;
    }
    spinLockRelease(&processTableLock);

    struct Process *parentProc = currentProcess();
    if (!parentProc)
        return NULL;

    struct Process *childProc = (struct Process *)kMalloc(sizeof(struct Process));
    if (!childProc)
        return NULL;
    memset(childProc, 0, sizeof(struct Process));

    for (int i = 0; i < MAX_FD; i++) {
        childProc->files[i] = parentProc->files[i];
    }

    childProc->pid = nextPid++;
    childProc->state = PROCESS_CREATED;
    childProc->exitCode = 0;
    childProc->parent = parentProc;

    // Private AddressSpace
    if (createAddressSpace(&childProc->addressSpace) != 0) {
        kFree(childProc);
        return NULL;
    }

    // Map User Code Page
    uint32_t childCodePhys = pmmAllocateFrame();
    if (!childCodePhys) {
        destroyAddressSpace(&childProc->addressSpace);
        kFree(childProc);
        return NULL;
    }

    if (addressSpaceMapPage(&childProc->addressSpace, 0x00400000, childCodePhys, PAGE_USER | PAGE_WRITABLE) != 0) {
        pmmDeallocateFrame(childCodePhys);
        destroyAddressSpace(&childProc->addressSpace);
        kFree(childProc);
        return NULL;
    }

    uint32_t parentCodePhys = 0;
    addressSpaceGetPhysicalAddress(&parentProc->addressSpace, 0x00400000, &parentCodePhys);
    memcpy((void *)PHYS_TO_VIRT(childCodePhys), (void *)PHYS_TO_VIRT(parentCodePhys), PAGE_SIZE);
    
    // Map User Stack Page
    uint32_t childStackPhys = pmmAllocateFrame();
    if (!childStackPhys) {
        pmmDeallocateFrame(childCodePhys);
        destroyAddressSpace(&childProc->addressSpace);
        kFree(childProc);
        return NULL;
    }

    if (addressSpaceMapPage(&childProc->addressSpace, 0x00800000, childStackPhys, PAGE_USER | PAGE_WRITABLE) != 0) {
        pmmDeallocateFrame(childStackPhys);
        pmmDeallocateFrame(childCodePhys);
        destroyAddressSpace(&childProc->addressSpace);
        kFree(childProc);
        return NULL;
    }

    uint32_t parentStackPhys = 0;
    addressSpaceGetPhysicalAddress(&parentProc->addressSpace, 0x00800000, &parentStackPhys);
    memcpy((void *)PHYS_TO_VIRT(childStackPhys), (void *)PHYS_TO_VIRT(parentStackPhys), PAGE_SIZE);

    extern void forkReturn(void);
    childProc->entryPoint = parentProc->entryPoint;
    childProc->userStackTop = parentProc->userStackTop;

    void *kstack = kMalloc(STACK_SIZE);
    if (!kstack) {
        pmmDeallocateFrame(childStackPhys);
        pmmDeallocateFrame(childCodePhys);
        destroyAddressSpace(&childProc->addressSpace);
        kFree(childProc);
        return NULL;
    }

    struct Thread *childThread = (struct Thread *)kMalloc(sizeof(struct Thread));
    if (!childThread) {
        pmmDeallocateFrame(childStackPhys);
        pmmDeallocateFrame(childCodePhys);
        destroyAddressSpace(&childProc->addressSpace);
        kFree(kstack);
        kFree(childProc);
        return NULL;
    }

    childThread->tid = childProc->pid;
    childThread->sleepTime = 0;
    childThread->kernelStack = kstack;
    childThread->process = childProc;
    childThread->schedNext = NULL;
    childThread->procNext = NULL;

    uint8_t *stackTop = (uint8_t *)kstack + STACK_SIZE;
    
    stackTop -= sizeof(struct SyscallFrame);
    struct SyscallFrame *childFrame = (struct SyscallFrame *)stackTop;
    *childFrame = *parentFrame;
    
    childFrame->eax = 0;
    uint32_t *sp = (uint32_t *)childFrame;
    *(--sp) = (uint32_t)forkReturn; // EIP for switchContext's 'ret'
    *(--sp) = 0;                    // EBP
    *(--sp) = 0;                    // EBX
    *(--sp) = 0;                    // ESI
    *(--sp) = 0;                    // EDI
    *(--sp) = 0x202;                // EFLAGS (Interrupts enabled)

    childThread->stackPointer = sp;
    childProc->mainThread = childThread;
    childProc->threadList = childThread;
    childProc->threadCount = 1;
    childProc->state = PROCESS_RUNNING;

    if (addProcess(childProc) != 0) {
        kFree(childThread);
        kFree(kstack);
        pmmDeallocateFrame(childStackPhys);
        pmmDeallocateFrame(childCodePhys);
        destroyAddressSpace(&childProc->addressSpace);
        kFree(childProc);
        return NULL;
    }

    readyThreadsPush(childThread);
    return childProc;
}

#define MAX_EXEC_ARGS 16
#define MAX_EXEC_ARG_LEN 64

int processExec(struct SyscallFrame *frame, const char *path, const char **argv) {
    if (!frame || !path)
        return -1;

    struct VfsNode *node = vfsOpen(path);
    if (!node || node->type != REGULAR || node->size > PAGE_SIZE)
        return -1;

    struct Process *proc = currentProcess();
    if (!proc)
        return -1;

    // Buffer arguments into kernel memory before modifying user address space
    char kargs[MAX_EXEC_ARGS][MAX_EXEC_ARG_LEN];
    int argc = 0;

    if (argv) {
        while (argv[argc] != NULL && argc < MAX_EXEC_ARGS) {
            strncpy(kargs[argc], argv[argc], MAX_EXEC_ARG_LEN - 1);
            kargs[argc][MAX_EXEC_ARG_LEN - 1] = '\0';
            argc++;
        }
    }

    // Terminate all sibling threads
    struct Thread *curr = scheduledThread();
    struct Thread *t = proc->threadList;
    while (t) {
        if (t != curr) {
            t->state = THREAD_TERMINATED;
        }
        t = t->procNext;
    }
    proc->mainThread = curr;
    proc->threadList = curr;
    curr->procNext = NULL;
    proc->threadCount = 1;

    for (int i = 3; i < MAX_FD; i++) {
        proc->files[i].node = NULL;
        proc->files[i].offset = 0;
    }

    uint32_t codePhys = 0;
    addressSpaceGetPhysicalAddress(&proc->addressSpace, 0x00400000, &codePhys);
    memset((void *)PHYS_TO_VIRT(codePhys), 0, PAGE_SIZE);
    vfsRead(node, 0, (uint8_t *)PHYS_TO_VIRT(codePhys), node->size);

    uint32_t stackPhys = 0;
    addressSpaceGetPhysicalAddress(&proc->addressSpace, 0x00800000, &stackPhys);
    memset((void *)PHYS_TO_VIRT(stackPhys), 0, PAGE_SIZE);

    // Build the initial user stack
    uint32_t userStackTop = 0x00800000 + PAGE_SIZE;
    uint8_t *kStackBase = (uint8_t *)PHYS_TO_VIRT(stackPhys);
    uint32_t userArgvPointers[MAX_EXEC_ARGS];

    if (argc > 0) {
        // Copy strings onto user stack from top downwards
        for (int i = argc - 1; i >= 0; i--) {
            size_t slen = strlen(kargs[i]) + 1;
            userStackTop -= slen;
            memcpy(kStackBase + (userStackTop - 0x00800000), kargs[i], slen);
            userArgvPointers[i] = userStackTop;
        }

        // Align to 4-byte boundary
        userStackTop &= ~3;

        // Push argv[argc] = NULL
        userStackTop -= 4;
        *(uint32_t *)(kStackBase + (userStackTop - 0x00800000)) = 0;

        // Push argv[argc-1] .. argv[0]
        for (int i = argc - 1; i >= 0; i--) {
            userStackTop -= 4;
            *(uint32_t *)(kStackBase + (userStackTop - 0x00800000)) = userArgvPointers[i];
        }
        uint32_t userArgvBase = userStackTop;

        // Push pointer to argv array
        userStackTop -= 4;
        *(uint32_t *)(kStackBase + (userStackTop - 0x00800000)) = userArgvBase;

        // Push argc
        userStackTop -= 4;
        *(uint32_t *)(kStackBase + (userStackTop - 0x00800000)) = (uint32_t)argc;
    } else {
        // No arguments: push argv = NULL, argc = 0
        userStackTop -= 4;
        *(uint32_t *)(kStackBase + (userStackTop - 0x00800000)) = 0;

        userStackTop -= 4;
        *(uint32_t *)(kStackBase + (userStackTop - 0x00800000)) = 0;
    }

    frame->eip = 0x00400000;
    frame->userEsp = userStackTop;
    frame->eax = 0;
    frame->ebx = 0;
    frame->ecx = 0;
    frame->edx = 0;
    frame->esi = 0;
    frame->edi = 0;
    frame->ebp = 0;

    return 0;
}   

struct Process *currentProcess(void) {
    struct Thread *curr = scheduledThread();
    if (!curr)
        return NULL;
    return curr->process;
}

static int foregroundPid = -1;

int processGetForeground(void) {
    return foregroundPid;
}

void processSetForeground(int pid) {
    foregroundPid = pid;
}

int processKill(int pid, int exitCode) {
    if (pid <= 0)
        return -1;

    spinLockAcquire(&processTableLock);
    struct Process *target = NULL;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i] && (int)processTable[i]->pid == pid) {
            target = processTable[i];
            break;
        }
    }

    if (!target || target->state == PROCESS_TERMINATED) {
        spinLockRelease(&processTableLock);
        return -1;
    }

    target->exitCode = exitCode;
    target->state = PROCESS_TERMINATED;

    struct Thread *t = target->threadList;
    while (t) {
        t->state = THREAD_TERMINATED;
        t = t->procNext;
    }

    spinLockRelease(&processTableLock);
    return 0;
}

void processExit(int exitCode) {
    struct Process *proc = currentProcess();
    if (!proc || proc->state == PROCESS_TERMINATED) {
        threadExit();
        return;
    }

    proc->exitCode = exitCode;
    proc->state = PROCESS_TERMINATED;

    struct Thread *curr = scheduledThread();
    struct Thread *t = proc->threadList;
    while (t) {
        if (t != curr) {
            t->state = THREAD_TERMINATED;
        }
        t = t->procNext;
    }

    threadExit();
}

int processWaitPid(int pid, int *status, int options) {
    
    struct Process *parent = currentProcess();
    if (!parent)
        return -1;

    if (pid > 0) {
        foregroundPid = pid;
    }

    while (1) {
        int hasChildren = 0;
        struct Process *target = NULL;

        spinLockAcquire(&processTableLock);
        if (pid > 0) {
            for (int i = 0; i < MAX_PROCESSES; i++) {
                if (processTable[i] && processTable[i]->pid == (uint32_t)pid && processTable[i]->parent == parent) {
                    hasChildren = 1;
                    if (processTable[i]->state == PROCESS_TERMINATED) {
                        target = processTable[i];
                    }
                    break;
                }
            }
        } else if (pid == -1) {
            for (int i = 0; i < MAX_PROCESSES; i++) {
                if (processTable[i] && processTable[i]->parent == parent) {
                    hasChildren = 1;
                    if (processTable[i]->state == PROCESS_TERMINATED) {
                        target = processTable[i];
                        break;
                    }
                }
            }
        } else {
            spinLockRelease(&processTableLock);
            if (foregroundPid == pid) {
                foregroundPid = -1;
            }
            return -1;
        }

        if (!hasChildren) {
            spinLockRelease(&processTableLock);
            if (foregroundPid == pid) {
                foregroundPid = -1;
            }
            return -1;
        }

        if (target) {
            if (foregroundPid == (int)target->pid) {
                foregroundPid = -1;
            }
            int childPid = (int)target->pid;
            int childExitCode = target->exitCode;

            for (int i = 0; i < MAX_PROCESSES; i++) {
                if (processTable[i] == target) {
                    processTable[i] = NULL;
                    processCount--;
                    break;
                }
            }
            spinLockRelease(&processTableLock);

            if (status) {
                *status = childExitCode;
            }

            uint32_t codePhys = 0;
            if (addressSpaceGetPhysicalAddress(&target->addressSpace, 0x00400000, &codePhys) == 0 && codePhys) {
                pmmDeallocateFrame(codePhys);
            }
            uint32_t stackPhys = 0;
            if (addressSpaceGetPhysicalAddress(&target->addressSpace, 0x00800000, &stackPhys) == 0 && stackPhys) {
                pmmDeallocateFrame(stackPhys);
            }
            destroyAddressSpace(&target->addressSpace);

            struct Thread *t = target->threadList;
            while (t) {
                struct Thread *nextT = t->procNext;
                if (t->kernelStack) {
                    kFree(t->kernelStack);
                }
                kFree(t);
                t = nextT;
            }

            kFree(target);
            return childPid;
        }

        spinLockRelease(&processTableLock);

        if (options & 1) {
            if (foregroundPid == pid) {
                foregroundPid = -1;
            }
            return 0;
        }

        yield();
    }
}
