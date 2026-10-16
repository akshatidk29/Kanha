#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "vga.h"

void exception_handler_divide_by_zero(void);
void exception_handler_page_fault(uint32_t errorCode, uint32_t faultAddress);

#endif