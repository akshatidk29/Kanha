ENTRY(_start)

KERNEL_VIRTUAL_BASE  = 0xC0000000;
KERNEL_PHYSICAL_BASE = 0x10000;

PHDRS
{
    bootstrap PT_LOAD FLAGS(7); /* R | W | X */
    kernel    PT_LOAD FLAGS(7); /* R | W | X */
}

SECTIONS
{
    . = KERNEL_PHYSICAL_BASE;

    kernelStart = .;
    kernelPhysicalStart = .;

    .bootstrap.text ALIGN(16) :
    {
        KEEP(*(.bootstrap.text))
    } :bootstrap

    .bootstrap.data ALIGN(16) :
    {
        KEEP(*(.bootstrap.data))
    } :bootstrap

    .bootstrap.bss ALIGN(4096) (NOLOAD) :
    {
        KEEP(*(.bootstrap.bss))
    } :bootstrap

    . = ALIGN(4096);
    kernelPhysicalLoadStart = .;

    . += KERNEL_VIRTUAL_BASE;
    kernelVirtualStart = .;

    .text ALIGN(16) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE)
    {
        *(.text*)
    } :kernel

    .rodata ALIGN(16) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE)
    {
        *(.rodata*)
    } :kernel

    .data ALIGN(16) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE)
    {
        *(.data*)
    } :kernel

    .bss ALIGN(16) (NOLOAD) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    } :kernel

    . = ALIGN(4096);
    kernelVirtualEnd = .;

    kernelPhysicalEnd = kernelVirtualEnd - KERNEL_VIRTUAL_BASE;
    kernelEnd = kernelPhysicalEnd;

    /DISCARD/ :
    {
        *(.interp)
        *(.dynamic)
        *(.dynsym)
        *(.dynstr)
        *(.gnu.hash)
        *(.rel.dyn)
        *(.note.gnu.build-id)
    }
}
