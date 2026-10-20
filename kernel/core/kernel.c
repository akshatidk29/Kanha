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

void printE820(struct BootInfo *bootInfo){

    struct E820Entry *entries =
        (struct E820Entry *)(uintptr_t)bootInfo->e820Address;

    vgaWrite("E820 Memory Map\n");

    for (uint32_t i = 0; i < bootInfo->e820Count; i++) {

        vgaWrite("BASE: ");
        vgaPutInt64(entries[i].base);

        vgaWrite(" LENGTH: ");
        vgaPutInt64(entries[i].length); 

        vgaWrite(" TYPE: ");
        vgaPutInt64(entries[i].type);  

        vgaWrite(" ATTRIBUTES: ");
        vgaPutInt64(entries[i].attributes);

        vgaWrite("\n");
    }
}

static void testPageFault(void){
    volatile uint32_t *invalidAddress = (volatile uint32_t *)0x20000000;
    uint32_t value = *invalidAddress;
    (void)value;
}

void kernelMain(struct BootInfo *bootInfo)
{
    vgaInitialize();           // Initialize VGA
    idtInitialize();           // Initialize IDT
    picInitialize();           // Initialize PIC
    pitInitialize(1000);       // Initialize PIT with 1KHz frequency

    pmmInitialize(bootInfo);

    pagingInitialize();

    uint32_t pageDirectory = pagingGetDirectory();
    vgaWrite("PAGE DIRECTORY AT: ");
    vgaPutInt(pageDirectory);
    vgaPutChar('\n');
    
    pagingEnable(pageDirectory);


    ticks = 0;

    picClearMask(0);          // Unmask Timer
    picClearMask(1);          // Unmask Keyboard
    //picClearMask(12);       // Unmask Mouse
    
    enableInterrupts();

    vgaPutChar('\n');
    printE820(bootInfo);
    vgaPutChar('\n');

    vgaWrite("TOTAL FRAMES: ");
    vgaPutInt(pmmTotalFramesCount());
    vgaPutChar('\n');

    vgaWrite("FREE FRAMES: ");
    vgaPutInt(pmmFreeFramesCount());
    vgaPutChar('\n');

    vgaWrite("RESERVED FRAMES: ");
    vgaPutInt(pmmReservedFramesCount());
    vgaPutChar('\n');
    vgaPutChar('\n');

    uint32_t page_addr = pmmAllocateFrame();
    vgaWrite("PAGE ALLOCATED AT: ");
    vgaPutInt(page_addr);
    vgaPutChar('\n');

    uint32_t page_addr2 = pmmAllocateFrame();
    vgaWrite("PAGE ALLOCATED AT: ");
    vgaPutInt(page_addr2);
    vgaPutChar('\n');

    uint32_t ret = pmmDeallocateFrame(page_addr);
    vgaWrite("PAGE DEALLOCATED: ");
    vgaPutInt(ret);
    vgaPutChar('\n');    
    
    uint32_t ret2 = pmmDeallocateFrame(page_addr);
    vgaWrite("PAGE DEALLOCATED: ");
    vgaPutInt(ret2);
    vgaPutChar('\n');

    uint32_t page_addr3 = pmmAllocateFrame();
    vgaWrite("PAGE ALLOCATED AT: ");
    vgaPutInt(page_addr3);
    vgaPutChar('\n');

    // testPageFault();

    struct AddressSpace kernelSpace;
    kernelSpace.pageDirectory = pagingGetDirectory();

    struct AddressSpace testSpace;
    if (createAddressSpace(&testSpace) != 0) {
        vgaWrite("Address Space creation failed\n");
        for(;;)asm volatile ("cli; hlt");
    }
    
    uintptr_t frame1 = pmmAllocateFrame();
    uintptr_t frame2 = pmmAllocateFrame();
    volatile uint32_t *test = (volatile uint32_t *)0x10000000u;

    if (frame1 != 0 && addressSpaceMapPage(&kernelSpace, 0x10000000u, (uint32_t)frame1, PAGE_WRITABLE) == 0) {
        *test = 123456789;
    }

    if (frame2 != 0 && addressSpaceMapPage(&testSpace, 0x10000000u, (uint32_t)frame2, PAGE_WRITABLE) == 0) {
    }

    vgaWrite("Switching To New Address Space\n");
    if (switchAddressSpace(&testSpace) != 0) {
        vgaWrite("Address Space switch failed\n");
        for(;;)asm volatile ("cli; hlt");
    }
    *test = 987654321;
    vgaWrite("NEW ADDRESS SPACE VALUE: ");
    vgaPutInt(*test);
    vgaPutChar('\n');
    
    if(switchAddressSpace(&kernelSpace) != 0){
        vgaWrite("Address Space switch failed\n");
        for(;;)asm volatile ("cli; hlt");
    }
    vgaWrite("Switched Back To Kernel Space\n");
    
    vgaWrite("KERNEL SPACE VALUE: ");
    vgaPutInt(*test);
    vgaPutChar('\n');

    while (1) {}
}