global jumpToUserMode

; void jumpToUserMode(uint32_t userIp, uint32_t userSp);

jumpToUserMode:
    mov edx, [esp + 4]      ; userIp
    mov ecx, [esp + 8]      ; userSp

    ; Load User Data selector (0x20 | 3 = 0x23) into data segments
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Push iret frame (SS, ESP, EFLAGS, CS, EIP)
    push dword 0x23         ; SS (User Data)
    push ecx                ; ESP (User stack)

    pushfd
    pop eax
    or eax, 0x200           ; Set IF bit
    push eax                ; EFLAGS

    push dword 0x1B         ; CS (User Code: 0x18 | 3)
    push edx                ; EIP (User entry)

    ; Execute iret to drop to Ring 3
    iret
