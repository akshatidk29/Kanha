BITS 16
ORG 0x7C00

start:
    
    mov [boot_drive], dl  ; Save the boot drive number

    ; Load kernel from disk

    mov ah, 0x02         ; BIOS: read sectors
    mov al, 1            ; Read 1 sector

    mov ch, 0            ; Cylinder 0
    mov cl, 2            ; Sector 2
    mov dh, 0            ; Head 0
    mov dl, [boot_drive]

    ; Destination = 0x10000
    mov ax, 0x1000
    mov es, ax
    mov bx, 0x0000

    int 0x13

    ; If BIOS returns here, reading succeeded

    ; Jump to loaded kernel
    jmp 0x1000:0x0000


boot_drive:
    db 0

times 510 - ($ - $$) db 0
dw 0xAA55