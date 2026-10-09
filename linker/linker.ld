ENTRY(_start)

SECTIONS
{
    . = 0x10000;
    kernel_start = .;

    .text : ALIGN(16) {
        *(.text*)
    }

    .rodata : ALIGN(16) {
        *(.rodata*)
    }

    .data : ALIGN(16) {
        *(.data*)
    }

    .bss : ALIGN(16) {
        *(COMMON)
        *(.bss*)
    }

    . = ALIGN(4096);
    kernel_end = .;

    /DISCARD/ : {
        *(.interp)
        *(.dynamic)
        *(.dynsym)
        *(.dynstr)
        *(.gnu.hash)
        *(.rel.dyn)
        *(.note.gnu.build-id)
    }
}