#include "pic.h"
#include "timer.h"

void irq_handler_timer(void){
    ticks++;
    pic_send_eoi(0);
}