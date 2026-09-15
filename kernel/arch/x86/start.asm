BITS 32

global _start
global idt_load

extern kernel_main

_start:

    ; Set up stack
    mov esp, 0x90000

    ; Enter C
    call kernel_main

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret 


.hang:
    cli
    hlt
    jmp .hang