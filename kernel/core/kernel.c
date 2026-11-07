#include "drivers/vga.h"
#include "drivers/serial.h"
#include "drivers/stdin.h"
#include "interrupts/idt.h"
#include "hardware/io.h"
#include "hardware/pic.h"
#include "hardware/timer.h"
#include "hardware/tss.h"
#include "boot/e820.h"
#include "mm/pmm.h"
#include "mm/paging.h"
#include "mm/addressSpace.h"
#include "mm/memLayout.h"
#include "mm/heap.h"
#include "lib/kprintf.h"
#include "task/lock.h"
#include "task/thread.h"
#include "task/scheduler.h"
#include "task/process.h"
#include "fs/vfs.h"
#include "drivers/ata.h"
#include "fs/bcache.h"
#include "fs/ext2.h"
#include "lib/string.h"

volatile uint32_t ticks;

int globalCounter = 0;
struct SpinLock lock;

extern void enableInterrupts(void);
extern void jumpToUserMode(uint32_t, uint32_t);

void kernelPanic(char* msg){
    
    if (msg)
        vgaWrite(msg);
    
    vgaWrite("\nCPU PANIC\n");
    while(1){
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}

void kernelMain(struct BootInfo *bootInfo){
    ticks = 0;

    vgaInitialize();           // Initialize VGA
    idtInitialize();           // Initialize IDT
    picInitialize();           // Initialize PIC
    pitInitialize(1000);       // Initialize PIT with 1KHz frequency
    serialInitialize();
    stdinInitialize();
    pmmInitialize(bootInfo);
    
    if(pagingInitialize() != 0)
        kernelPanic("Paging Initialization Failed\n");
    
    pagingEnable(getKernelPageDirectory());
    
    if(heapInitialize() != 0)
        kernelPanic("Heap Initialization Failed\n");

    uint32_t currentEsp;
    __asm__ volatile("mov %%esp, %0" : "=r"(currentEsp));
    
    tssInitialize(0x10, currentEsp);

    picClearMask(0);          // Unmask Timer
    picClearMask(1);          // Unmask Keyboard
    picClearMask(4);          // Unmask Serial
    enableInterrupts();
    schedulerInitialize();

    // Initialize ATA, Buffer Cache, and Mount ext2 root filesystem
    ataInitialize();
    bcacheInitialize();
    ext2Mount(ATA_DRIVE_SLAVE);

    // Launch Shell
    struct VfsNode *initProc = vfsOpen("/bin/shell");
    if (initProc) {
        uint8_t *codeBuf = (uint8_t *)kMalloc(initProc->size);
        if (codeBuf) {
            vfsRead(initProc, 0, codeBuf, initProc->size);
            processCreate(codeBuf, initProc->size);
            kFree(codeBuf);
        }
    }

    while (1) {
        __asm__ volatile("hlt");
    }
}