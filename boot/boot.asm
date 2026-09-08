BITS 16
ORG 0x7C00

    ; Initialize Segments
    xor ax, ax              ; because we cant load an immediate value into a segment register directly
    mov ds, ax              ; data segment
    mov es, ax              ; extra segment
    
    mov [boot_drive], dl    ; Save boot drive (BIOS puts it in dl)

    ; Read Sector 2 into 0x0000:0x500   [Because the 0x000:0x500 is used/reserved by IVT + BDA]

    mov ah, 0x02            ; Read Sectors
    mov al, 1               ; Number of Sectors
    mov ch, 0               ; Cylinder
    mov cl, 2               ; Sector (sector 2 is kernel)
    mov dh, 0               ; Head
    mov dl, [boot_drive]    ; Save Boot Drive
    mov bx, 0x0500          ; ES:BX = 0x0000:0x500
    int 0x13                ; ES:BX = 0x0000:0x500

    jmp 0x0000:0x0500       ; Jump to Kernel

boot_drive:
    db 0

times 510 - ($ - $$) db 0
dw 0xAA55