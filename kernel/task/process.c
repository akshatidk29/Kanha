#include "pmm.h"
#include "paging.h"
#include "heap.h"
#include "process.h"
#include "kprintf.h"
#include "scheduler.h"

extern void jumpToUserMode(uint32_t userIp, uint32_t userSp);

static uint32_t nextPid = 1;
static struct Process kernelProcess;

void processInitialize(void){
    kernelProcess.pid = 0;
    kernelProcess.state = PROCESS_RUNNING;
    kernelProcess.exitCode = 0; 

    kernelProcess.addressSpace.pageDirectory = getKernelPageDirectory();
    kernelProcess.entryPoint = 0;
    kernelProcess.userStackTop = 0;

    struct Thread *mainThread = scheduledThread();
    kernelProcess.mainThread = mainThread;
    mainThread->process = &kernelProcess;
}

static void processEntry(void *arg) { 

    struct Process *proc = (struct Process *)arg;

    switchAddressSpace(&proc->addressSpace);
    jumpToUserMode(proc->entryPoint, proc->userStackTop);
}

struct Process *processCreate(const uint8_t *code, uint32_t codeSize) {

    if (!code || codeSize == 0 || codeSize > PAGE_SIZE)
        return NULL;

    struct Process *proc = (struct Process *)kMalloc(sizeof(struct Process));
    if (!proc)
        return NULL;

    proc->pid = nextPid++;
    proc->state = PROCESS_CREATED;
    proc->exitCode = 0;

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
    proc->userStackTop = 0x00800000 + PAGE_SIZE - 16;

    // Create the main thread for this process
    struct Thread *thread = threadCreate(processEntry, proc);
    if (!thread) {
        pmmDeallocateFrame(stackPhys);
        pmmDeallocateFrame(codePhys);
        destroyAddressSpace(&proc->addressSpace);
        kFree(proc);
        return NULL;
    }

    thread->process = proc;
    proc->mainThread = thread;
    proc->state = PROCESS_RUNNING;

    return proc;
}

struct Process *currentProcess(void) {
    struct Thread *curr = scheduledThread();
    if (!curr)
        return NULL;
    return curr->process;
}

void processExit(int exitCode) {
    struct Process *proc = currentProcess();
    if (proc) {
        proc->state = PROCESS_TERMINATED;
        proc->exitCode = exitCode;
    }
    threadExit();
}
