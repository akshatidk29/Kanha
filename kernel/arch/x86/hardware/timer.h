#ifndef KANHA_TIMER_H
#define KANHA_TIMER_H

#include <stdint.h>

#define PIT_MODE_SQWAVE 0x36             // Mode 3 (Square Wave), Binary Mode, Channel 0
#define PIT_FREQ        1193182          // PIT Clock Frequency (1.193182 MHz)
#define PIT_CHANNEL0    0x40             // PIT Channel 0
#define PIT_COMMAND     0x43             // PIT Command Port

extern volatile uint32_t ticks;

void pit_initialize(uint32_t freq);
void irq_handler_timer(void);

#endif
