#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void vga_initialize();

void vga_put_char(char c);
void vga_write(const char* c);

void vga_put_int(uint32_t n);

void vga_scroll(void);

#endif

