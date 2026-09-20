#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void vga_initialize();

void vga_set_cursor(uint8_t row, uint8_t column);
void vga_cursor(void);

void vga_put_char(char c);
void vga_clear_char(void);

void vga_write(const char* c);

void vga_put_int(uint32_t n);

void vga_scroll(void);

#endif

