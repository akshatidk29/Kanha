#include "mm/pmm.h"
#include "mm/paging.h"
#include "mm/heap.h"
#include "task/process.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "fs/vfs.h"
#include "syscall/syscall.h"
#include "task/scheduler.h"

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

struct Process *processFork(struct SyscallFrame *parentFrame){

    struct Process *parentProc = currentProcess();
    struct Process *childProc = (struct Process *)kMalloc(sizeof(struct Process));
    if(!childProc)
        return NULL;

    childProc->pid = nextPid++;
    childProc->state = PROCESS_CREATED;
    childProc->exitCode = 0;

    // Private AddressSpace
    if (createAddressSpace(&childProc->addressSpace) != 0) {
        kFree(childProc);
        return NULL;
    }

    // Map User Code Page
    uint32_t childCodePhys = pmmAllocateFrame();
    if(addressSpaceMapPage(&childProc->addressSpace, 0x00400000, childCodePhys, PAGE_USER | PAGE_WRITABLE) != 0){
        pmmDeallocateFrame(childCodePhys);
        destroyAddressSpace(&childProc->addressSpace);
        kFree(childProc);
        return NULL;
    }

    uint32_t parentCodePhys;
    addressSpaceGetPhysicalAddress(&parentProc->addressSpace, 0x00400000, &parentCodePhys);
    memcpy((void *)PHYS_TO_VIRT(childCodePhys), (void *)PHYS_TO_VIRT(parentCodePhys), PAGE_SIZE);
    
    uint32_t childStackPhys = pmmAllocateFrame();
    addressSpaceMapPage(&childProc->addressSpace, 0x00800000, childStackPhys, PAGE_USER | PAGE_WRITABLE);

    uint32_t parentStackPhys;
    addressSpaceGetPhysicalAddress(&parentProc->addressSpace, 0x00800000, &parentStackPhys);
    memcpy((void *)PHYS_TO_VIRT(childStackPhys), (void *)PHYS_TO_VIRT(parentStackPhys), PAGE_SIZE);

    extern void forkReturn(void);
    childProc->entryPoint = parentProc->entryPoint;
    childProc->userStackTop = parentProc->userStackTop;

    void *kstack = kMalloc(STACK_SIZE);
    if (!kstack) {
        pmmDeallocateFrame(childCodePhys);
        pmmDeallocateFrame(childStackPhys);
        destroyAddressSpace(&childProc->addressSpace);
        kFree(childProc);
        return NULL;
    }

    struct Thread *childThread = (struct Thread *)kMalloc(sizeof(struct Thread));
    if (!childThread) {
        pmmDeallocateFrame(childCodePhys);
        pmmDeallocateFrame(childStackPhys);
        destroyAddressSpace(&childProc->addressSpace);
        kFree(childProc);
        kFree(kstack);
        return NULL;
    }

    childThread->tid = childProc->pid;
    childThread->sleepTime = 0;
    childThread->kernelStack = kstack;
    childThread->process = childProc;
    childThread->next = NULL;

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
    childProc->state = PROCESS_RUNNING;
    readyThreadsPush(childThread);
    return childProc;
}

int processExec(struct SyscallFrame *frame, const char *path){
    
    if(!frame || !path)
        return -1;

    struct VfsNode *node = vfsOpen(path);
    if(!node || node->type != REGULAR)
        return -1;

    struct Process *proc = currentProcess();
    if(!proc)
        return -1;
        
    uint32_t codePhys;
    addressSpaceGetPhysicalAddress(&proc->addressSpace, 0x00400000, &codePhys);
    memset((void *)PHYS_TO_VIRT(codePhys), 0, PAGE_SIZE);
    memcpy((void *)PHYS_TO_VIRT(codePhys), node->deviceData, node->size);

    uint32_t stackPhys;
    addressSpaceGetPhysicalAddress(&proc->addressSpace, 0x00800000, &stackPhys);
    memset((void *)PHYS_TO_VIRT(stackPhys), 0, PAGE_SIZE);

    frame->eip = 0x00400000;
    frame->userEsp = 0x00800000 + PAGE_SIZE - 16;
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

void processExit(int exitCode) {
    struct Process *proc = currentProcess();
    if (proc) {
        proc->state = PROCESS_TERMINATED;
        proc->exitCode = exitCode;
    }
    threadExit();
}
