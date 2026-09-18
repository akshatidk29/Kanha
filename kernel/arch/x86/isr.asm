; Interrupt Service Routine

BITS 32

global isr0
global isr33
extern error_handler_divide_by_zero
extern interrupt_handler_keyboard

isr0:
    pusha
    call error_handler_divide_by_zero
    popa
    iret


isr33:                                          ; Keyboard Interrupt
    pusha
    call interrupt_handler_keyboard
    popa
    iret
    