; Interrupt Service Routine

BITS 32

global isr0
global isr32
global isr33

extern exception_handler_divide_by_zero
extern irq_handler_timer
extern irq_handler_keyboard

isr0:
    pusha
    call exception_handler_divide_by_zero       ; Divide by Zero Exception
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
    