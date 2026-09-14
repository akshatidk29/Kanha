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

        if(row >= VGA_HEIGHT){
            vga_scroll();
        }
        return;
    }

    VGA_MEMORY[(row * VGA_WIDTH + column) * 2] = c;
    VGA_MEMORY[(row * VGA_WIDTH + column) * 2 + 1] = 0x07;

    column++;

    if (column >= VGA_WIDTH) {
        column = 0;
        row++;

        if(row >= VGA_HEIGHT){
            vga_scroll();
        }
    }
}

void vga_write(const char* c){
    while(*c != '\0'){
        vga_put_char(*c);
        c++;
    }
}


void vga_scroll(void){
    
    for(int i = 1; i < VGA_HEIGHT; i++){
        for(int j = 0; j < VGA_WIDTH; j++){

            unsigned int src = (i * VGA_WIDTH + j) * 2;
            unsigned int dst = ((i - 1) * VGA_WIDTH + j) * 2;

            VGA_MEMORY[dst] = VGA_MEMORY[src];
            VGA_MEMORY[dst + 1] = VGA_MEMORY[src + 1];
        }
    }

    for(int j = 0; j < VGA_WIDTH; j++){
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH * 2 + j * 2] = ' ';
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH * 2 + j * 2 + 1] = 0x07;
    }

    row = VGA_HEIGHT - 1;
    column = 0;
}