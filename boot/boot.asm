BITS 16
ORG 0x7C00

E820_BUFFER      equ 0x7000
E820_ENTRY_SIZE  equ 24
E820_MAX_ENTRIES equ 32


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

; Get BIOS Memory Map
    xor bx, bx                              ; Continuation Value
    xor bp, bp                              ; Index into Memory Map

    mov ax, 0x0000
    mov es, ax                              ; Set ES to 0 for BIOS call
    mov di, E820_BUFFER                     ; Set DI to buffer address for BIOS call

e820NextEntry:

    mov eax, 0xE820
    mov edx, 0x534D4150                     ; "SMAP"
    mov ecx, E820_ENTRY_SIZE                ; Size of each entry

    int 0x15                                ; Call BIOS to get memory map

    jc e820Done                             ; If carry flag set, error

    cmp eax, 0x534D4150                     ; Check if SMAP returned successfully
    jne diskErr

    inc ebp                                 ; Increment entry count
    cmp ebp, E820_MAX_ENTRIES
    jae e820Done

    add di, E820_ENTRY_SIZE                 ; Move to next entry
    cmp ebx, 0
    jne e820NextEntry                       ; Loop until ebx becomes 0


e820Done:
    mov dword [0x7400], ebp                 ; Number of memory map entries
    mov dword [0x7404], E820_BUFFER         ; Address of memory map entries



; Check BIOS EDD support
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [bootDriveVariable]
    int 0x13
    jc diskErr

    cmp bx, 0xAA55
    jne diskErr

    test cx, 0x0001
    jz diskErr

; Read sector 2 (LBA 1): KANHA header -> 0x8000
    mov word [diskPacketCount], 1
    mov word [diskPacketOffset], 0x0000
    mov word [diskPacketSegment], 0x0800
    mov dword [diskPacketLBA], 1
    mov dword [diskPacketLBA + 4], 0

    mov si, diskPacket
    mov dl, [bootDriveVariable]
    mov ah, 0x42
    int 0x13
    jc diskErr

; Validate KANHA magic number
    mov ax, 0x0800
    mov es, ax

    cmp dword [es:0x0000], 0x484E414B
    jne diskErr

    cmp byte [es:0x0004], 0x41
    jne diskErr

; Header bytes 5..8 contain the kernel byte size.
    mov eax, [es:0x0005]
    test eax, eax
    jz diskErr

; sectors = ceil(kernelSize / 512)
    add eax, 511
    jc diskErr
    shr eax, 9
    mov [kernelSectorsRemaining], eax

; Prepare kernel reads: LBA 2 -> physical 0x10000
; Read at most 32 sectors per BIOS request.
    mov word [diskPacketCount], 32
    mov word [diskPacketOffset], 0
    mov word [diskPacketSegment], 0x1000
    mov dword [diskPacketLBA], 2
    mov dword [diskPacketLBA + 4], 0

readKernelLoop:
    cmp dword [kernelSectorsRemaining], 0
    je kernelReadDone

    ; Select min(remaining, 32).
    mov eax, [kernelSectorsRemaining]
    cmp eax, 32
    jae .useFullChunk

    mov [diskPacketCount], ax
    jmp .readChunk

.useFullChunk:
    mov word [diskPacketCount], 32

.readChunk:
    ; Update remaining count before the BIOS call.
    movzx eax, word [diskPacketCount]
    sub [kernelSectorsRemaining], eax

    ; BIOS EDD read: DS:SI points to the DAP.
    mov si, diskPacket
    mov dl, [bootDriveVariable]
    mov ah, 0x42
    int 0x13
    jc diskErr

    ; Advance LBA by the number of sectors read.
    movzx eax, word [diskPacketCount]
    add dword [diskPacketLBA], eax
    adc dword [diskPacketLBA + 4], 0

    ; Advance destination segment:
    ; 512 bytes per sector = 32 paragraphs.
    shl eax, 5
    add word [diskPacketSegment], ax

    jmp readKernelLoop

kernelReadDone:


; Changing System from Real Mode to Protected Mode

    cli                                     ; Disable Maskable Externel Hardware Interrupts

; Load GDT
    lgdt [gdtDescriptor]

; Set PE bit in CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax                            ; PE bit enables Protected Mode

; Far Jump into Protected Mode
    jmp 0x08:protectedMode




diskErr:
    cli
    hlt
    jmp diskErr



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

kernelSectorsRemaining:
    dd 0

; BIOS Enhanced Disk Drive Disk Address Packet (16 bytes)

align 4
diskPacket:
    db 0x10, 0x00             ; Packet size, reserved
diskPacketCount:
    dw 0                      ; Number of sectors
diskPacketOffset:
    dw 0                      ; Destination offset
diskPacketSegment:
    dw 0                      ; Destination segment
diskPacketLBA:
    dq 0                      ; Starting LBA


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