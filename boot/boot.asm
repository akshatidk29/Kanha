BITS 16
ORG 0x7C00

; Initialize Segments
    xor ax, ax
    mov ds, ax                              ; DS = 0 
    mov es, ax                              ; ES = 0

    mov [bootDriveVariable], dl             ; Save boot drive
    
; Enable A20 Line
    in al, 0x92
    or al, 0x02
    and al, 0xFE
    out 0x92, al

; Read Sector 2 into 0x1000:0x0000 => 0x10000
    mov ax, 0x1000
    mov es, ax                              ; ES = 0x1000

    mov ah, 0x02                            ; Read Sectors
    mov al, 1                               ; Number of Sectors to read
    mov ch, 0                               ; Cylinder 0
    mov cl, 2                               ; Sector 2 (kernel)
    mov dh, 0                               ; Head 0
    mov dl, [bootDriveVariable]             ; Boot drive number
    mov bx, 0x0000                          ; ES:BX = 0x1000:0x0000 => 0x10000
    int 0x13


; Changing System from Real Mode to Protected Mode

    cli                                     ; Disable Interrupts

; Load GDT
    lgdt [gdtDescriptor]

; Set PE bit in CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax                            ; PE bit enables Protected Mode

; Far Jump into Protected Mode
    jmp 0x08:protectedMode







; 32-bit Code

BITS 32

protectedMode:

    ; Reload all data segment registers with PM data selector
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Jump to Kernel at physical 0x10000
    jmp 0x08:0x10000




; Data Variables

BITS 16

bootDriveVariable:
    db 0



; GDT

gdtStart:

gdtNull:
    dq 0x0000000000000000                   ; 1st Entry is Null

gdtCode:                                    ; Code Segment Descriptor
    dw 0xFFFF                               ; Limit low
    dw 0x0000                               ; Base low
    db 0x00                                 ; Base mid
    db 0x9A                                 ; Access: present, ring0, code, executable, readable
    db 0xCF                                 ; Flags: 4KB gran, 32-bit + Limit high 0xF
    db 0x00                                 ; Base high

gdtData:                                    ; Data Segment Descriptor
    dw 0xFFFF                               ; Limit low
    dw 0x0000                               ; Base low
    db 0x00                                 ; Base mid
    db 0x92                                 ; Access: present, ring0, data, writable
    db 0xCF                                 ; Flags: 4KB gran, 32-bit + Limit high 0xF
    db 0x00                                 ; Base high

gdtEnd:

gdtDescriptor:
    dw gdtEnd - gdtStart - 1                ; GDT size - 1
    dd gdtStart                             ; Physical address of GDT


times 510 - ($ - $$) db 0

dw 0xAA55