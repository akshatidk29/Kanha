BITS 32

global _start

extern main
extern exit
extern __bss_start
extern __bss_end

section .text

_start:
    xor ebp, ebp                ; Clear ebp 

    mov edi, __bss_start        ; Zero out the BSS segment
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb   

    ; Stack layout
    ; [esp]     = argc
    ; [esp + 4] = argv
    push dword [esp + 4]        ; push argv
    push dword [esp + 4]        ; push argc
    call main                   
    add esp, 8                  ; Clean up the arguments

    push eax
    call exit                   ; exit(eax)

    mov ebx, eax                ; Store Exit Status Code in ebx
    mov eax, 1                  ; SYS_EXIT = 1
    int 0x80

.hang:
    jmp .hang                   ; Loop if Syscall fails