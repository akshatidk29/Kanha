#include "io.h"
#include "vga.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile unsigned char *)0xB8000)


#define VGA_CRTC_DATA 0x3D5             // VGA Cathode Ray Tube Controller for Blinking Cursor - Data Register
#define VGA_CRTC_INDEX 0x3D4            // VGA Cathode Ray Tube Controller for Blinking Cursor - Index Register

#define VGA_CRTC_CURSOR_LOW 0x0F        // Blinking Cursor Position - Low Byte
#define VGA_CRTC_CURSOR_HIGH 0x0E       // Blinking Cursor Position - High Byte

static uint32_t row;
static uint32_t column;

void vga_initialize(void){
    row = 0;
    column = 0;

    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i * 2] = ' ';
        VGA_MEMORY[i * 2 + 1] = 0x07;
    }
}

void vga_put_char(char c){   

    uint32_t pos = (row * VGA_WIDTH + column) * 2;
    
    if(c == '\n'){

        vga_clear_char();
        
        column = 0;
        row++;

        if(row >= VGA_HEIGHT){
            vga_scroll();
        }
        vga_cursor();
        return;
    }
    
    VGA_MEMORY[pos] = c;
    VGA_MEMORY[pos + 1] = 0x07;

    column++;

    if (column >= VGA_WIDTH) {
        column = 0;
        row++;

        if(row >= VGA_HEIGHT){
            vga_scroll();
        }
    }

    vga_cursor();
}

void vga_clear_char(void){
    uint32_t pos = (row * VGA_WIDTH + column) * 2;
    VGA_MEMORY[pos] = ' ';
    VGA_MEMORY[pos + 1] = 0x07;
}


void vga_set_cursor(uint8_t row, uint8_t column) {
    uint16_t pos = row * VGA_WIDTH + column;

    // Low byte
    outb(VGA_CRTC_INDEX, VGA_CRTC_CURSOR_LOW);
    outb(VGA_CRTC_DATA, (uint8_t)(pos & 0xFF));

    // High byte
    outb(VGA_CRTC_INDEX, VGA_CRTC_CURSOR_HIGH);
    outb(VGA_CRTC_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_cursor(void){
    vga_set_cursor(row, column);
}
    

void vga_put_int(uint32_t n){

    if(n == 0){
        vga_put_char('0');
        return;
    }

    int i = 0;
    char buffer[12];

    while(n != 0){
        buffer[i] = (n % 10) + '0';
        n /= 10;
        i++;
    }

    i--;
    while(i >= 0){
        vga_put_char(buffer[i]);
        i--;
    }
}

void vga_write(const char* c){
    while(*c != '\0'){
        vga_put_char(*c);
        c++;
    }
}

void vga_scroll(void){
    
    for(uint32_t i = 1; i < VGA_HEIGHT; i++){
        for(uint32_t j = 0; j < VGA_WIDTH; j++){

            uint32_t src = (i * VGA_WIDTH + j) * 2;
            uint32_t dst = ((i - 1) * VGA_WIDTH + j) * 2;

            VGA_MEMORY[dst] = VGA_MEMORY[src];
            VGA_MEMORY[dst + 1] = VGA_MEMORY[src + 1];
        }
    }

    for(uint32_t j = 0; j < VGA_WIDTH; j++){
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH * 2 + j * 2] = ' ';
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH * 2 + j * 2 + 1] = 0x07;
    }

    row = VGA_HEIGHT - 1;
    column = 0;
}