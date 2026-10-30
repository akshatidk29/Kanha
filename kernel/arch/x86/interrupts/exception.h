#ifndef KANHA_EXCEPTION_H
#define KANHA_EXCEPTION_H

#include <stdint.h>
#include "drivers/vga.h"

void exceptionHandlerDivideByZero(void);
void exceptionHandlerDoubleFault(uint32_t errorCode);
void exceptionHandlerGPF(uint32_t errorCode);
void exceptionHandlerPageFault(uint32_t faultAddress, uint32_t errorCode);

#endif