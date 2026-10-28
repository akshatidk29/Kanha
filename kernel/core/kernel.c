#include "vga.h"
#include "idt.h"
#include "io.h"
#include "pic.h"
#include "timer.h"
#include "e820.h"
#include "pmm.h"
#include "paging.h"
#include "addressSpace.h"
#include "memLayout.h"
#include "heap.h"
#include "serial.h"
#include "kprintf.h"
#include "lock.h"
#include "tss.h"
#include "thread.h"
#include "scheduler.h"
#include "process.h"


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


void printE820(struct BootInfo *bootInfo){

    struct E820Entry *entries =
        (struct E820Entry *)PHYS_TO_VIRT(bootInfo->e820Address);

    vgaWrite("E820 Memory Map\n");

    for (uint32_t i = 0; i < bootInfo->e820Count; i++) {

        vgaWrite("BASE: ");
        vgaPutUnsignedInt64(entries[i].base);

        vgaWrite(" LENGTH: ");
        vgaPutUnsignedInt64(entries[i].length); 

        vgaWrite(" TYPE: ");
        vgaPutUnsignedInt64(entries[i].type);  

        vgaWrite(" ATTRIBUTES: ");
        vgaPutUnsignedInt64(entries[i].attributes);

        vgaWrite("\n");
    }
}

static void testPaging(void){

    vgaWrite("TOTAL FRAMES: ");
    vgaPutUnsignedInt(pmmTotalFramesCount());
    vgaPutChar('\n');

    vgaWrite("FREE FRAMES: ");
    vgaPutUnsignedInt(pmmFreeFramesCount());
    vgaPutChar('\n');

    vgaWrite("RESERVED FRAMES: ");
    vgaPutUnsignedInt(pmmReservedFramesCount());
    vgaPutChar('\n');
    vgaPutChar('\n');

    uint32_t before = pmmFreeFramesCount();
    uint32_t frame = pmmAllocateFrame();

    if (frame == 0)
        kernelPanic("PMM allocation failed");

    if (frame % PMM_PAGE_SIZE != 0)
        kernelPanic("PMM returned an unaligned frame");

    if (pmmFreeFramesCount() != before - 1)
        kernelPanic("PMM free count incorrect after allocation");

    if (pmmDeallocateFrame(frame) != 0)
        kernelPanic("PMM deallocation failed");

    if (pmmFreeFramesCount() != before)
        kernelPanic("PMM free count incorrect after deallocation");

    vgaWrite("PMM Tests Passed\n");
}

static void testHeap(void){

    int *p1 = (int *)kMalloc(sizeof(int) * 10);
    if (!p1) 
        kernelPanic("kMalloc failed for p1\n");
    p1[0] = 42;
    p1[9] = 99;

    char *p2 = (char *)kMalloc(8192);
    if (!p2) 
        kernelPanic("kMalloc failed\n");
    p2[0] = 'A';
    p2[8191] = 'Z';

    if (kFree(p1) != 0) 
        kernelPanic("kFree failed\n");
    if (kFree(p2) != 0) 
        kernelPanic("kFree failed\n");

    char *p3 = (char *)kMalloc(10000);
    if (!p3) 
        kernelPanic("kMalloc failed\n");
    p3[0] = 'X';
    p3[9999] = 'Y';
    if (kFree(p3) != 0) 
        kernelPanic("kFree failed\n");

    if ((void *)p3 != (void *)p1) {
        kernelPanic("Coalescing failed\n");
    }

    vgaWrite("Heap Tests Passed\n");
}

void* func(int count){
    spinLockAcquire(&lock);
    for(int i = 0; i < count; i++){
        globalCounter++;
    }
    spinLockRelease(&lock);
    return NULL;
}

void kernelMain(struct BootInfo *bootInfo){
    ticks = 0;

    vgaInitialize();           // Initialize VGA (at 0xC00B8000)
    idtInitialize();           // Initialize IDT
    picInitialize();           // Initialize PIC
    pitInitialize(1000);       // Initialize PIT with 1KHz frequency
    serialInitialize();
    pmmInitialize(bootInfo);
    
    if(pagingInitialize() != 0){
        vgaWrite("Paging Initialization Failed\n");
        kernelPanic(NULL);
    }
    pagingEnable(getKernelPageDirectory());
    
    if(heapInitialize() != 0){
        vgaWrite("Heap Initialization Failed\n");
        kernelPanic(NULL);
    }

    uint32_t currentEsp;
    __asm__ volatile("mov %%esp, %0" : "=r"(currentEsp));
    
    tssInitialize(0x10, currentEsp);

    picClearMask(0);          // Unmask Timer
    picClearMask(1);          // Unmask Keyboard
    picClearMask(4);          // Unmask Serial
    enableInterrupts();
    
    testPaging();
    testHeap();
    
    schedulerInitialize();

    int count = 1000000;
    spinLockInit(&lock);
    threadCreate((void(*)(void*))func, (void*)(count));
    threadCreate((void(*)(void*))func, (void*)(count));
    threadCreate((void(*)(void*))func, (void*)(count));

    threadSleep(2000);

    kprintf("Akshat Mittal from Main Thread\n");
    kprintf("Global Counter: %d\n", globalCounter);


    uint8_t userBinary[128];
    uint8_t userProgram[] = {
        0xB8, 0x02, 0x00, 0x00, 0x00, // mov eax, 2 (SYS_WRITE)
        0xBB, 0x01, 0x00, 0x00, 0x00, // mov ebx, 1 (stdout)
        0xB9, 0x40, 0x00, 0x40, 0x00, // mov ecx, 0x00400040
        0xBA, 0x17, 0x00, 0x00, 0x00, // mov edx, 23
        0xCD, 0x80,                   // int 0x80

        0xB8, 0x01, 0x00, 0x00, 0x00, // mov eax, 1 (SYS_EXIT)
        0xBB, 0x2A, 0x00, 0x00, 0x00, // mov ebx, 42
        0xCD, 0x80,                   // int 0x80

        0xEB, 0xFE                    // jmp $
    };

    for (uint32_t i = 0; i < sizeof(userProgram); i++) {
        userBinary[i] = userProgram[i];
    }
    const char *msg = "Hello from User Mode!\n";
    for (uint32_t i = 0; msg[i] != '\0'; i++) {
        userBinary[0x40 + i] = (uint8_t)msg[i];
    }

    // Launch real User Process!
    processCreate(userBinary, 0x40 + 23);

    threadSleep(2000);

    vgaWrite("Done\n");
    
    while (1) {
        __asm__ volatile("hlt");
    }
}