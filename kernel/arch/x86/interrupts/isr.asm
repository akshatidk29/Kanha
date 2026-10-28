; Interrupt Service Routine

BITS 32

global isr0
global isr8
global isr13
global isr14
global isr32
global isr33
global isr36
global isr44
global isr128

extern exceptionHandlerDivideByZero
extern exceptionHandlerDoubleFault
extern exceptionHandlerGPF
extern exceptionHandlerPageFault

extern syscallHandler

extern irqHandlerTimer
extern irqHandlerKeyboard
extern irqHandlerSerial
extern irqHandlerMouse

isr0:
    pusha
    call exceptionHandlerDivideByZero           ; Divide by Zero Exception
    popa
    iret

isr8:
    pusha
    push dword[esp + 32]                        ; Error code
    call exceptionHandlerDoubleFault
    add esp, 4
    popa
    add esp, 4                                  ; Remove error code from stack
    iret

isr13:
    pusha
    push dword[esp + 32]                        ; Error code
    call exceptionHandlerGPF
    add esp, 4
    popa
    add esp, 4                                  ; Remove error code from stack
    iret

isr14:
    pusha
    mov eax, cr2                                ; Page fault linear address
    push dword[esp + 32]                        ; Error code
    push eax
    call exceptionHandlerPageFault              ; Page Fault Exception with args fault_address then error code
    add esp, 8
    popa
    add esp, 4                                  ; Remove error code from stack
    iret

isr32:
    pusha
    call irqHandlerTimer                      ; Timer Interrupt
    popa
    iret

isr33:                                        ; Keyboard Interrupt
    pusha
    call irqHandlerKeyboard
    popa
    iret

isr36:
    pusha
    call irqHandlerSerial                     ; Serial Interrupt 
    popa
    iret

isr44:
    pusha
    call irqHandlerMouse                      ; Mouse Interrupt
    popa
    iret

isr128:
    push ds
    push es
    push fs
    push gs
    pusha
    
    mov ax, 0x10                             ; Switch to Kernel Data Segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp                                 ; Pass pointer to SyscallFrame

    call syscallHandler
    
    add esp, 4
    popa
    pop gs
    pop fs
    pop es
    pop ds
    iret
