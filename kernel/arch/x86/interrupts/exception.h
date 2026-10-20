#ifndef KANHA_EXCEPTION_H
#define KANHA_EXCEPTION_H

#include "vga.h"

void exceptionHandlerDivideByZero(void);
void exceptionHandlerPageFault(uint32_t errorCode, uint32_t faultAddress);

#endif