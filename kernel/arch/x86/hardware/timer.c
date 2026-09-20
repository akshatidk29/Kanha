#include "io.h"
#include "pic.h"
#include "timer.h"

void pit_initialize(uint32_t freq){
    uint32_t divisor = 1193182 / freq;

    outb(PIT_COMMAND,  PIT_MODE_SQWAVE);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void irq_handler_timer(void){
    ticks++;
    pic_send_eoi(0);
}