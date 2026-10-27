BITS 32
global switchContext

switchContext:
    ; Save callee-saved registers
    push ebp
    push ebx
    push esi
    push edi
    pushfd                  ; Save EFLAGS

    mov eax, [esp + 24]     ; [esp + 24] holds oldStackPointer
    mov [eax], esp          ; Save ESP into oldThread->stackPointer

    mov edx, [esp + 28]
    mov esp, edx            ; Load ESP from newThread->stackPointer

    popfd                   ; Restore EFLAGS
    pop edi
    pop esi
    pop ebx
    pop ebp

    ret
