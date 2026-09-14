#include "vga.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile unsigned char *)0xB8000)

static unsigned int row;
static unsigned int column;

void vga_initialize(void)
{
    row = 0;
    column = 0;

    for (unsigned int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i * 2] = ' ';
        VGA_MEMORY[i * 2 + 1] = 0x07;
    }
}

void vga_put_char(char c)
{   
    if(c == '\n'){
        column = 0;
        row++;
    }
    else {
        VGA_MEMORY[(row * VGA_WIDTH + column) * 2] = c;
        VGA_MEMORY[(row * VGA_WIDTH + column) * 2 + 1] = 0x07;

        column++;
    }
    

    if (column >= VGA_WIDTH) {
        column = 0;
        row++;
    }
}

void vga_write(char* c){
    while(*c != '\0'){
        vga_put_char(*c);
        c++;
    }
}