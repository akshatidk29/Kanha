BITS 32

global _start
global higherHalfEntry

global idtLoad
global enableInterrupts

extern kernelMain
extern __bss_start
extern __bss_end

%define KERNEL_VIRTUAL_BASE  0xC0000000
%define KERNEL_PDE_START     768
%define PAGE_PRESENT         0x001
%define PAGE_WRITABLE        0x002
%define PAGE_SIZE_4MB        0x080
%define PAGE_FLAGS           (PAGE_PRESENT | PAGE_WRITABLE | PAGE_SIZE_4MB)

section .bootstrap.text

_start:
    cli
    cld

    ; Initial stack
    mov esp, 0x90000

    ; Zero the bootstrap page directory.
    mov edi, bootstrapPageDirectory
    xor eax, eax
    mov ecx, 1024
    rep stosd

    ; Map physical 0-128 MiB at virtual 0-128 MiB, also map the same physical range at 0xC0000000.
    mov edi, bootstrapPageDirectory
    xor ebx, ebx
    mov ecx, 32

.mapPages:
    mov eax, ebx
    or eax, PAGE_FLAGS

    ; Identity mapping: PDE 0-31
    mov [edi], eax

    ; Higher-half mapping: PDE 768-799
    mov [edi + KERNEL_PDE_START * 4], eax

    add edi, 4
    add ebx, 0x400000
    loop .mapPages

    ; Enable 4 MiB pages
    mov eax, cr4
    or eax, 0x10
    mov cr4, eax

    ; Load the physical page-directory address
    mov eax, bootstrapPageDirectory
    mov cr3, eax

    ; Enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; Transfer execution to the higher-half entry
    mov eax, higherHalfEntry
    jmp eax

section .text

higherHalfEntry:
    ; Switch to the higher-half alias of the existing stack
    mov esp, 0xC0090000
    cld

    ; Load higher-half GDT so CPU doesn't access GDT in unmapped low memory (0x7da4)
    lgdt [gdtDescriptor]
    jmp 0x08:.reloadCS
.reloadCS:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Unmap bootstrap identity mapping (PDE 0-31) now that we are in higher half
    mov edi, (bootstrapPageDirectory + KERNEL_VIRTUAL_BASE)
    xor eax, eax
    mov ecx, 32
    rep stosd

    ; Flush TLB by reloading cr3
    mov eax, cr3
    mov cr3, eax

    ; Clear the higher-half kernel's BSS
    xor eax, eax
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    rep stosb

    ; Pass the boot information pointer to kernelMain in higher half
    push dword (KERNEL_VIRTUAL_BASE + 0x7400)
    call kernelMain

.hang:
    cli
    hlt
    jmp .hang

section .data
align 8
gdtStart:
    dq 0x0000000000000000           ; Null descriptor

    dw 0xFFFF                       ; Kernel Code Segment
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00
    
    dw 0xFFFF                       ; Kernel Data Segment
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00

    dw 0xFFFF                        ; User Code Segment 
    dw 0x0000
    db 0x00
    db 0xFA
    db 0xCF
    db 0x00

    dw 0xFFFF                        ; User Data Segment 
    dw 0x0000
    db 0x00
    db 0xF2
    db 0xCF
    db 0x00
gdtEnd:

gdtDescriptor:
    dw gdtEnd - gdtStart - 1
    dd gdtStart


section .bootstrap.bss nobits align=4096

bootstrapPageDirectory:
    resd 1024

section .text

idtLoad:
    mov eax, [esp + 4]
    lidt [eax]
    ret

enableInterrupts:
    sti
    ret
