ENTRY(_start)

SECTIONS
{
    . = 0x00400000;             /* User code starts at 4MB virtual address */

    .text ALIGN(16) :
    {
        KEEP(*crt0*(.text))     
        *(.text*)
    }

    .rodata ALIGN(16) :
    {
        *(.rodata*)
    }

    .data ALIGN(16) :
    {
        *(.data*)
    }

    .bss ALIGN(16) :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
