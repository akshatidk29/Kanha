#ifndef KANHA_PIC_H
#define KANHA_PIC_H

// PIC is Programmable Interrupt Controller. It stands between the CPU and the Hardware Devices
// Responsible for handling the interrupts from the hardware devices.


#include <stdint.h>

#define PIC1_COMMAND  0x20          // Master PIC Command Port
#define PIC1_DATA     0x21          // Master PIC Data Port

#define PIC2_COMMAND  0xA0          // Slave PIC Command Port
#define PIC2_DATA     0xA1          // Slave PIC Data Port

#define PIC1_OFFSET   0x20          // Maps to IRQ 0
#define PIC2_OFFSET   0x28          // Maps to IRQ 8


void pic_initialize(void);      

void pic_set_mask(uint8_t irq);           // Blocks the IRQ
void pic_clear_mask(uint8_t irq);         // Unblocks the IRQ
void pic_send_eoi(uint8_t irq);           // Sends End Of Interrupt signal to the PIC

#endif