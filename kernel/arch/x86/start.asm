BITS 32

global _start
extern kernel_main

_start:

    ; Set up stack
    mov esp, 0x90000

    ; Enter C
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang