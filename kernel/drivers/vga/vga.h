#ifndef KANHA_VGA_H
#define KANHA_VGA_H

#include <stdint.h>

void vgaInitialize();

void vgaSetCursor(uint8_t row, uint8_t column);
void vgaCursor(void);

void vgaPutChar(char c);
void vgaClearChar(void);

void vgaWrite(const char* c);

void vgaPutInt(uint32_t n);
void vgaPutInt64(uint64_t n);

void vgaScroll(void);

#endif

