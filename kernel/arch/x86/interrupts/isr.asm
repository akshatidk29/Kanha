; Interrupt Service Routine

BITS 32

global isr0
global isr14
global isr32
global isr33
global isr44

extern exception_handler_divide_by_zero
extern exception_handler_page_fault

extern irq_handler_timer
extern irq_handler_keyboard
extern irq_handler_mouse
isr0:
    pusha
    call exception_handler_divide_by_zero       ; Divide by Zero Exception
    popa
    iret

isr14:
    pusha
    mov eax, cr2                                ; Page fault linear address
    push dword[esp + 32]                        ; Error code
    push eax
    call exception_handler_page_fault           ; Page Fault Exception with args error_code then fault_address
    add esp, 8
    popa
    iret

isr32:
    pusha
    call irq_handler_timer                      ; Timer Interrupt
    popa
    iret

isr33:                                          ; Keyboard Interrupt
    pusha
    call irq_handler_keyboard
    popa
    iret
    
isr44:
    pusha
    call irq_handler_mouse                      ; Mouse Interrupt
    popa
    iret