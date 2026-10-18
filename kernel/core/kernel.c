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

extern void enable_interrupts(void);

void printE820(struct bootInfo *bootInfo){

    struct e820Entry *entries =
        (struct e820Entry *)(uintptr_t)bootInfo->e820Address;

    vga_write("E820 Memory Map\n");

    for (uint32_t i = 0; i < bootInfo->e820Count; i++) {

        vga_write("BASE: ");
        vga_put_int64(entries[i].base);

        vga_write(" LENGTH: ");
        vga_put_int64(entries[i].length); 

        vga_write(" TYPE: ");
        vga_put_int64(entries[i].type);  

        vga_write(" ATTRIBUTES: ");
        vga_put_int64(entries[i].attributes);

        vga_write("\n");
    }
}

static void testPageFault(void){
    volatile uint32_t *invalidAddress = (volatile uint32_t *)0x20000000;
    uint32_t value = *invalidAddress;
    (void)value;
}

void kernel_main(struct bootInfo *bootInfo)
{
    vga_initialize();           // Initialize VGA
    idt_initialize();           // Initialize IDT
    pic_initialize();           // Initialize PIC
    pit_initialize(1000);       // Initialize PIT with 1KHz frequency

    pmm_initialize(bootInfo);

    pagingInitialize();

    uint32_t pageDirectory = pagingGetDirectory();
    vga_write("PAGE DIRECTORY AT: ");
    vga_put_int(pageDirectory);
    vga_put_char('\n');
    
    pagingEnable(pageDirectory);


    ticks = 0;

    pic_clear_mask(0);          // Unmask Timer
    pic_clear_mask(1);          // Unmask Keyboard
    //pic_clear_mask(12);       // Unmask Mouse
    
    enable_interrupts();

    vga_put_char('\n');
    printE820(bootInfo);
    vga_put_char('\n');

    vga_write("TOTAL FRAMES: ");
    vga_put_int(pmm_total_frames_count());
    vga_put_char('\n');

    vga_write("FREE FRAMES: ");
    vga_put_int(pmm_free_frames_count());
    vga_put_char('\n');

    vga_write("RESERVED FRAMES: ");
    vga_put_int(pmm_reserved_frames_count());
    vga_put_char('\n');
    vga_put_char('\n');

    uint32_t page_addr = pmm_allocate_frame();
    vga_write("PAGE ALLOCATED AT: ");
    vga_put_int(page_addr);
    vga_put_char('\n');

    uint32_t page_addr2 = pmm_allocate_frame();
    vga_write("PAGE ALLOCATED AT: ");
    vga_put_int(page_addr2);
    vga_put_char('\n');

    uint32_t ret = pmm_deallocate_frame(page_addr);
    vga_write("PAGE DEALLOCATED: ");
    vga_put_int(ret);
    vga_put_char('\n');    
    
    uint32_t ret2 = pmm_deallocate_frame(page_addr);
    vga_write("PAGE DEALLOCATED: ");
    vga_put_int(ret2);
    vga_put_char('\n');

    uint32_t page_addr3 = pmm_allocate_frame();
    vga_write("PAGE ALLOCATED AT: ");
    vga_put_int(page_addr3);
    vga_put_char('\n');

    // testPageFault();

    struct addressSpace kernelSpace;
    kernelSpace.pageDirectory = pagingGetDirectory();

    struct addressSpace testSpace;
    if (createAddressSpace(&testSpace) != 0) {
        vga_write("Address Space creation failed\n");
        for(;;)asm volatile ("cli; hlt");
    }
    
    uintptr_t frame1 = pmm_allocate_frame();
    uintptr_t frame2 = pmm_allocate_frame();
    volatile uint32_t *test = (volatile uint32_t *)0x10000000u;

    if (frame1 != 0 &&
        addressSpaceMapPage(&kernelSpace, 0x10000000u,
                    (uint32_t)frame1,
                    PAGE_WRITABLE) == 0) {
        *test = 123456789;
    }

    if (frame2 != 0 &&
        addressSpaceMapPage(&testSpace, 0x10000000u,
                    (uint32_t)frame2,
                    PAGE_WRITABLE) == 0) {
    }

    vga_write("Switching To New Address Space\n");
    if (switchAddressSpace(&testSpace) != 0) {
        vga_write("Address Space switch failed\n");
        for(;;)asm volatile ("cli; hlt");
    }
    *test = 987654321;
    vga_write("NEW ADDRESS SPACE VALUE: ");
    vga_put_int(*test);
    vga_put_char('\n');
    
    if(switchAddressSpace(&kernelSpace) != 0){
        vga_write("Address Space switch failed\n");
        for(;;)asm volatile ("cli; hlt");
    }
    vga_write("Switched Back To Kernel Space\n");
    
    vga_write("KERNEL SPACE VALUE: ");
    vga_put_int(*test);
    vga_put_char('\n');

    while (1) {}
}