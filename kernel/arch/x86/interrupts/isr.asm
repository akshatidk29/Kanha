; Interrupt Service Routine

BITS 32

global isr0
global isr14
global isr32
global isr33
global isr44

extern exceptionHandlerDivideByZero
extern exceptionHandlerPageFault

extern irqHandlerTimer
extern irqHandlerKeyboard
extern irqHandlerMouse

isr0:
    pusha
    call exceptionHandlerDivideByZero           ; Divide by Zero Exception
    popa
    iret

isr14:
    pusha
    mov eax, cr2                                ; Page fault linear address
    push dword[esp + 32]                        ; Error code
    push eax
    call exceptionHandlerPageFault              ; Page Fault Exception with args fault_address then error core
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
    
isr44:
    pusha
    call irqHandlerMouse                      ; Mouse Interrupt
    popa
    iret