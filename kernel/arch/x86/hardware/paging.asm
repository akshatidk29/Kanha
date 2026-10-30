BITS 32

global pagingEnable

pagingEnable:
    mov eax, [esp + 4]   ; physical address of page directory
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000   ; set CR0.PG
    mov cr0, eax

    jmp short .pagingActive

.pagingActive:
    ret   
