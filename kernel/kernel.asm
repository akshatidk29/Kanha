BITS 16

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
    db "Hello from kernel!", 0