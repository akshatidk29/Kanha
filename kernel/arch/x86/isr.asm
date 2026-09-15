BITS 32

global isr0
extern divide_by_zero_handler

isr0:
    pusha
    call divide_by_zero_handler
    popa
    iret