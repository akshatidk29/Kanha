BITS 32

global _start
global idtLoad
global enableInterrupts

extern kernelMain
extern __bss_end
extern __bss_start

_start:

    ; Set up stack
    mov esp, 0x90000

    cld

    ; Zero the BSS region
    xor eax, eax
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    rep stosb

    ; Pass boot info to kernelMain
    push 0x7400
    
    ; Enter C
    call kernelMain


.hang:              ; Kernel will not return but what if!
    cli
    hlt
    jmp .hang


idtLoad:
    mov eax, [esp + 4]
    lidt [eax]
    ret 

enableInterrupts:
    sti
    ret