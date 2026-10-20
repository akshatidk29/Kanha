#include "io.h"
#include "pic.h"
#include "timer.h"

void pitInitialize(uint32_t freq){
    uint32_t divisor = 1193182 / freq;

    outB(PIT_COMMAND,  PIT_MODE_SQWAVE);
    outB(PIT_CHANNEL0, divisor & 0xFF);
    outB(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void irqHandlerTimer(void){
    ticks++;
    picSendEOI(0);
}