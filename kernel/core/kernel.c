#include "vga.h"
#include "idt.h"
#include "io.h"
#include "pic.h"
#include "timer.h"
#include "e820.h"
#include "pmm.h"
#include "paging.h"
#include "addressSpace.h"

volatile uint32_t ticks;

extern void enableInterrupts(void);

void kernelPanic(char* msg){
    
    vgaWrite(msg);
    vgaWrite("\nCPU PANIC\n");
    while(1){
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}


void printE820(struct BootInfo *bootInfo){

    struct E820Entry *entries =
        (struct E820Entry *)(uintptr_t)bootInfo->e820Address;

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
    uintptr_t frame = pmmAllocateFrame();

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

void kernelMain(struct BootInfo *bootInfo)
{
    vgaInitialize();           // Initialize VGA
    idtInitialize();           // Initialize IDT
    picInitialize();           // Initialize PIC
    pitInitialize(1000);       // Initialize PIT with 1KHz frequency

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

    if (getPhysicalAddress(pageDirectory, 0xC0100000u, &physical) != 0) {
        vgaWrite("Higher-half translation FAILED\n");
    } else {
        vgaWrite("Higher-half translation: ");
        vgaPutUnsignedInt(physical);
        vgaWrite("\n");
    }

    ticks = 0;

    picClearMask(0);          // Unmask Timer
    picClearMask(1);          // Unmask Keyboard
    //picClearMask(12);       // Unmask Mouse
    
    enableInterrupts();

    vgaPutChar('\n');
    printE820(bootInfo);
    vgaPutChar('\n');


    testPaging();

    struct AddressSpace kernelSpace;
    kernelSpace.pageDirectory = getKernelPageDirectory();

    struct AddressSpace testSpace = {0};
    if (createAddressSpace(&testSpace) != 0) 
        kernelPanic("Address Space Creation Failed\n");
    
    uintptr_t frame1 = pmmAllocateFrame();
    uintptr_t frame2 = pmmAllocateFrame();
    volatile uint32_t *test = (volatile uint32_t *)0x10000000u;

    if (frame1 != 0 && addressSpaceMapPage(&kernelSpace, 0x10000000u, (uint32_t)frame1, (PAGE_WRITABLE | PAGE_USER)) == 0) {
        vgaWrite("HI\n");
        *test = 123456789;
    }
    
    if (frame2 != 0 && addressSpaceMapPage(&testSpace, 0x10000000u, (uint32_t)frame2, (PAGE_WRITABLE | PAGE_USER)) == 0) {
        vgaWrite("HI\n");
    }
    
    vgaWrite("Switching To New Address Space\n");
    if (switchAddressSpace(&testSpace) != 0) {
        kernelPanic("Address Space switch failed\n");
    }

    *test = 987654321;
    vgaWrite("NEW ADDRESS SPACE VALUE: ");
    vgaPutUnsignedInt(*test);
    vgaPutChar('\n');
    
    if(switchAddressSpace(&kernelSpace) != 0){
        kernelPanic("Address Space switch failed\n");
    }

    vgaWrite("Switched Back To Kernel Space\n");
    
    vgaWrite("KERNEL SPACE VALUE: ");
    vgaPutUnsignedInt(*test);
    vgaPutChar('\n');


    while (1) {}
}