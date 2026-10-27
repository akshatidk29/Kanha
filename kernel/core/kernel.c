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

    vgaWrite("PMM Allocation Test Passed\n");
}

static void testHeap(void){
    if (heapInitialize() != 0) {
        kernelPanic("Heap initialization failed");
    }
    vgaWrite("Heap Initialized\n");

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

void kernelMain(struct BootInfo *bootInfo)
{
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

    uint32_t pageDirectory = getKernelPageDirectory();
    vgaWrite("PAGE DIRECTORY AT: ");
    vgaPutUnsignedInt(pageDirectory);
    vgaPutChar('\n');
    
    pagingEnable(pageDirectory);
    
    uint32_t physical = 0;

    if (getPhysicalAddress(pageDirectory, 0xC0012000u, &physical) != 0) {
        kernelPanic("Higher Half kernel mapping missing");
    }

    if (physical != 0x12000u) {
        kernelPanic("Higher Half translation incorrect");
    }

    vgaWrite("Higher Half mapping verified\n");

    // Verify that lower 3GB is completely unmapped
    if (getPhysicalAddress(pageDirectory, 0x00012000u, &physical) == 0) {
        kernelPanic("Lower Half mapping is still present\n");
    }
    vgaWrite("Lower 3GB verified\n");
    
    ticks = 0;

    picClearMask(0);          // Unmask Timer
    picClearMask(1);          // Unmask Keyboard
    
    enableInterrupts();

    vgaPutChar('\n');
    printE820(bootInfo);
    vgaPutChar('\n');

    testPaging();
    testHeap();

    struct AddressSpace kernelSpace;
    kernelSpace.pageDirectory = getKernelPageDirectory();

    struct AddressSpace testSpace = {0};
    if (createAddressSpace(&testSpace) != 0) 
        kernelPanic("Address Space Creation Failed\n");
    
    uint32_t frame1 = pmmAllocateFrame();
    uint32_t frame2 = pmmAllocateFrame();
    uint32_t frame3 = pmmAllocateFrame();

    volatile uint32_t *testUpper = (volatile uint32_t *)0x10000000u; // 256MB
    volatile uint32_t *testLower = (volatile uint32_t *)0x00400000u; // 4MB 

    if (frame1 != 0 && addressSpaceMapPage(&kernelSpace, 0x10000000u, frame1, (PAGE_WRITABLE | PAGE_USER)) == 0) {
        *testUpper = 123456789;
    }
    
    if (frame2 != 0 && addressSpaceMapPage(&testSpace, 0x10000000u, frame2, (PAGE_WRITABLE | PAGE_USER)) == 0) {
        //
    }

    if (frame3 != 0 && addressSpaceMapPage(&testSpace, 0x00400000u, frame3, (PAGE_WRITABLE | PAGE_USER)) == 0) {
        //
    }
    
    vgaWrite("Switching To New Address Space\n");
    if (switchAddressSpace(&testSpace) != 0) {
        kernelPanic("Address Space switch failed\n");
    }

    *testUpper = 987654321;
    *testLower = 555555555;

    vgaWrite("NEW ADDRESS SPACE 256MB VALUE: ");
    vgaPutUnsignedInt(*testUpper);
    vgaPutChar('\n');

    vgaWrite("NEW ADDRESS SPACE 4MB VALUE: ");
    vgaPutUnsignedInt(*testLower);
    vgaPutChar('\n');
    
    if(switchAddressSpace(&kernelSpace) != 0){
        kernelPanic("Address Space switch failed\n");
    }

    vgaWrite("Switched Back To Kernel Space\n");
    
    vgaWrite("KERNEL SPACE 256MB VALUE: ");
    vgaPutUnsignedInt(*testUpper);
    vgaPutChar('\n');


    printf("Hello, My Name is Akshat. \nI am %d years old!\n", 0x14);

    while (1) {}
}