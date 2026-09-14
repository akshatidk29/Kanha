#ifndef VGA_H
#define VGA_H

void vga_initialize();

void vga_put_char(char c);
void vga_write(const char* c);

void vga_scroll(void);

#endif

