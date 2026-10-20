BITS 32

global _start
global idtLoad
global enableInterrupts

extern kernelMain

_start:

    ; Set up stack
    mov esp, 0x90000

    push 0x7400
    
    ; Enter C
    call kernelMain

idtLoad:
    mov eax, [esp + 4]
    lidt [eax]
    ret 

enableInterrupts:
    sti
    ret