BITS 16
ORG 0x7C00

start:
    mov si, message

print:
    lodsb
    cmp al, 0
    je hang

    mov ah, 0x0E
    int 0x10

    jmp print

hang:
    cli
    hlt
    jmp hang

message:
    db "Kanha is alive!", 0

times 510 - ($ - $$) db 0
dw 0xAA55
