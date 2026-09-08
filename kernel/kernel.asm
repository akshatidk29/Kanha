BITS 16
ORG 0x0500          ; Loaded at 0x0000:0x0500

    xor ax, ax
    mov ds, ax      ; DS = 0 
    mov si, message

.print:
    lodsb           ; Load byte from [DS:SI] into AL, Also increments the SI
    cmp al, 0       ; AL = Character to Display
    je .hang        ; if AL is 0, jump to .hang
    mov ah, 0x0E    ; AH = Video Mode, 0x0E means Display a Character
    mov bh, 0       ; BH = Page Number
    int 0x10        ; Interrupt for Teleoperation
    jmp .print

.hang:
    hlt
    jmp .hang

message:
    db "Hello from kernel!", 0

times 512 - ($ - $$) db 0