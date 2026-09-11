BITS 32
ORG 0x0500                      ; Loaded at 0x0000:0x0500
    
    mov ax, 0x10                ; Data Segment Selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov esi, message
    mov edi, 0xB8000            ; VGA text buffer (physical address)
    mov ah, 0x0F                ; Attribute: white text on black background

.print:
    lodsb                       ; Load byte from [DS:ESI] into AL, increment ESI
    cmp al, 0                   ; Check for null terminator
    je .hang
    mov [edi], ax               ; Write char (AL) + attribute (AH) to VGA buffer
    add edi, 2                  ; Each VGA cell = 2 bytes (char + color)
    jmp .print

.hang:
    hlt
    jmp .hang

message:
    db "Hello from kernel!", 0

times 512 - ($ - $$) db 0