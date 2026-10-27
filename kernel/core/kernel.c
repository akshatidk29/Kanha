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
#include "printf.h"
#include "thread.h"
#include "scheduler.h"

volatile uint32_t ticks;

extern void enableInterrupts(void);

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

    vgaWrite("Heap Tests Passed!\n\n");
}

void* func1(){
    printf("Akshat Mittal from Thread1\n");
    return NULL;
}

void kernelMain(struct BootInfo *bootInfo)
{
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

    picClearMask(0);          // Unmask Timer
    picClearMask(1);          // Unmask Keyboard
    enableInterrupts();
    
    testPaging();
    testHeap();
    
    schedulerInitialize();

    threadCreate((void(*)(void*))func1, NULL);

    
    yield();
    
    printf("Akshat Mittal from Main Thread\n");

    while (1) {}
}