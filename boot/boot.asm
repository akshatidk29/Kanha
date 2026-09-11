BITS 16
ORG 0x7C00

; Initialize Segments
    xor ax, ax                              ; because we cant load an immediate value into a segment register directly
    mov ds, ax                              ; data segment
    mov es, ax                              ; extra segment
    
    mov [bootDriveVariable], dl             ; Save boot drive (BIOS puts it in dl)

; Read Sector 2 into 0x0000:0x0500   [Because the 0x000:0x0500 is used/reserved by IVT + BDA]

    mov ah, 0x02                            ; Read Sectors
    mov al, 1                               ; Number of Sectors
    mov ch, 0                               ; Cylinder
    mov cl, 2                               ; Sector (sector 2 is kernel)
    mov dh, 0                               ; Head
    mov dl, [bootDriveVariable]             ; Save Boot Drive
    mov bx, 0x0500                          ; ES:BX = 0x0000:0x0500
    int 0x13                                ; ES:BX = 0x0000:0x0500


; Changing System from Real Mode to Protected Mode

    cli                                     ; Disable Interrupts

; Load GDT
    lgdt [gdtDescriptor]

; Set PE bit in CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax                            ; PE bit enables Protected Mode

; Far Jump into Protected Mode
    jmp 0x08:protectedMode                  ; CS:IP = 0x08:0x0500







; 32-bit Code

BITS 32

protectedMode:
    jmp 0x08:0x0500                         ; Jump to Kernel







; Data Variables

BITS 16

bootDriveVariable:
    db 0


; GDT

gdtStart:

gdtNull:
    dq 0x0000000000000000                   ; 1st Entry is Null

gdtCode:                                    ; Code Segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00

gdtData:                                    ; Data Segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00

gdtEnd:

gdtDescriptor:
    dw gdtEnd - gdtStart - 1
    dd gdtStart


times 510 - ($ - $$) db 0

dw 0xAA55