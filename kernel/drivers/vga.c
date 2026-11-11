#include "hardware/io.h"
#include "drivers/vga.h"
#include "mm/memLayout.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile unsigned char *)VIRT_VGA_BUFFER)

#define VGA_CRTC_DATA 0x3D5             // VGA Cathode Ray Tube Controller for Blinking Cursor - Data Register
#define VGA_CRTC_INDEX 0x3D4            // VGA Cathode Ray Tube Controller for Blinking Cursor - Index Register

#define VGA_CRTC_CURSOR_LOW 0x0F        // Blinking Cursor Position - Low Byte
#define VGA_CRTC_CURSOR_HIGH 0x0E       // Blinking Cursor Position - High Byte

static uint32_t row;
static uint32_t column;
static uint8_t vgaCurrentColor = 0x07;

void vgaInitialize(void){
    row = 0;
    column = 0;
    vgaCurrentColor = 0x07;

    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i * 2] = ' ';
        VGA_MEMORY[i * 2 + 1] = 0x07;
    }
}

void vgaSetColor(uint8_t color) {
    vgaCurrentColor = color;
}

uint8_t vgaGetColor(void) {
    return vgaCurrentColor;
}

void vgaPutChar(char c){   

    static enum {
        ANSI_NORMAL,
        ANSI_ESC,
        ANSI_BRACKET
    } ansiState = ANSI_NORMAL;

    static int ansiParam = 0;
    static int ansiBold = 0;

    if (ansiState == ANSI_NORMAL) {
        if (c == '\033') {
            ansiState = ANSI_ESC;
            return;
        }
    } else if (ansiState == ANSI_ESC) {
        if (c == '[') {
            ansiState = ANSI_BRACKET;
            ansiParam = 0;
            ansiBold = 0;
            return;
        }
        ansiState = ANSI_NORMAL;
    } else if (ansiState == ANSI_BRACKET) {
        if (c >= '0' && c <= '9') {
            ansiParam = ansiParam * 10 + (c - '0');
            return;
        } else if (c == ';') {
            if (ansiParam == 1) {
                ansiBold = 1;
            }
            ansiParam = 0;
            return;
        } else if (c == 'm') {
            if (ansiParam == 0) {
                vgaCurrentColor = 0x07;
                ansiBold = 0;
            } else if (ansiParam == 1) {
                ansiBold = 1;
            } else if (ansiParam >= 30 && ansiParam <= 37) {
                static const uint8_t fgColors[] = {
                    0x00, // 30: black
                    0x04, // 31: red
                    0x02, // 32: green
                    0x06, // 33: yellow
                    0x01, // 34: blue
                    0x05, // 35: magenta
                    0x03, // 36: cyan
                    0x07  // 37: light gray
                };
                uint8_t fg = fgColors[ansiParam - 30];
                if (ansiBold) {
                    fg |= 0x08; 
                }
                vgaCurrentColor = (vgaCurrentColor & 0xF0) | (fg & 0x0F);
            } else if (ansiParam >= 90 && ansiParam <= 97) {
                static const uint8_t brightFgColors[] = {
                    0x08, // 90: dark gray
                    0x0C, // 91: bright red
                    0x0A, // 92: bright green
                    0x0E, // 93: bright yellow
                    0x09, // 94: bright blue
                    0x0D, // 95: bright magenta
                    0x0B, // 96: bright cyan
                    0x0F  // 97: bright white
                };
                uint8_t fg = brightFgColors[ansiParam - 90];
                vgaCurrentColor = (vgaCurrentColor & 0xF0) | (fg & 0x0F);
            }
            ansiState = ANSI_NORMAL;
            return;
        } else if (c == 'J') {
            if (ansiParam == 2) {
                for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
                    VGA_MEMORY[i * 2] = ' ';
                    VGA_MEMORY[i * 2 + 1] = vgaCurrentColor;
                }
                row = 0;
                column = 0;
                vgaCursor();
            }
            ansiState = ANSI_NORMAL;
            return;
        } else if (c == 'H' || c == 'f') {
            // Reset cursor
            row = 0;
            column = 0;
            vgaCursor();
            ansiState = ANSI_NORMAL;
            return;
        } else {
            ansiState = ANSI_NORMAL;
            return;
        }
    }

    uint32_t pos = (row * VGA_WIDTH + column) * 2;
    
    if(c == '\n'){
        vgaClearChar();
        column = 0;
        row++;

        if(row >= VGA_HEIGHT){
            vgaScroll();
        }
        vgaCursor();
        return;
    }

    if(c == '\b'){
        vgaClearChar();
        if(column == 0){
            if(row != 0){
                row--;
                column = VGA_WIDTH - 1;
            }
        }else{
            column--;
        }
        vgaCursor();
        return;
    }
    
    if(c == '\t'){
        uint32_t k = 4;
        vgaClearChar();
        
        if(column + k >= VGA_WIDTH){
            column = 0;
            row++;
            if(row >= VGA_HEIGHT){
                vgaScroll();
            }
            vgaCursor();
            return;
        }
        column += k;
        vgaCursor();
        return;
    }

    VGA_MEMORY[pos] = c;
    VGA_MEMORY[pos + 1] = vgaCurrentColor;

    column++;

    if (column >= VGA_WIDTH) {
        column = 0;
        row++;

        if(row >= VGA_HEIGHT){
            vgaScroll();
        }
    }

    vgaCursor();
}

void vgaClearChar(void){
    uint32_t pos = (row * VGA_WIDTH + column) * 2;
    VGA_MEMORY[pos] = ' ';
    VGA_MEMORY[pos + 1] = vgaCurrentColor;
}

void vgaSetCursor(uint8_t row, uint8_t column) {
    uint16_t pos = row * VGA_WIDTH + column;

    // Low byte
    outB(VGA_CRTC_INDEX, VGA_CRTC_CURSOR_LOW);
    outB(VGA_CRTC_DATA, (uint8_t)(pos & 0xFF));

    // High byte
    outB(VGA_CRTC_INDEX, VGA_CRTC_CURSOR_HIGH);
    outB(VGA_CRTC_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void vgaCursor(void){
    vgaSetCursor(row, column);
}

void vgaPutUnsignedInt(uint32_t n){
    if(n == 0){
        vgaPutChar('0');
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
        vgaPutChar(buffer[i]);
        i--;
    }
}

void vgaPutUnsignedInt64(uint64_t n){
    if(n == 0){
        vgaPutChar('0');
        return;
    }

    int i = 0;
    char buffer[22];

    while(n != 0){
        buffer[i] = (n % 10) + '0';
        n /= 10;
        i++;
    }

    i--;
    while(i >= 0){
        vgaPutChar(buffer[i]);
        i--;
    }
}

void vgaPutInt(int32_t n){
    if(n == 0){
        vgaPutChar('0');
        return;
    }
    if(n < 0){
        vgaPutChar('-');
        n = -n;
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
        vgaPutChar(buffer[i]);
        i--;
    }
}

void vgaPutInt64(int64_t n){
    if(n == 0){
        vgaPutChar('0');
        return;
    }
    if(n < 0){
        vgaPutChar('-');
        n = -n;
    }

    int i = 0;
    char buffer[22];

    while(n != 0){
        buffer[i] = (n % 10) + '0';
        n /= 10;
        i++;
    }

    i--;
    while(i >= 0){
        vgaPutChar(buffer[i]);
        i--;
    }
}

void vgaPutHex(uint32_t num){
    if(num == 0){
        vgaPutChar('0');
        return;
    }

    char hexDigits[] = "0123456789ABCDEF";
    char buffer[9];
    int i = 0;

    while(num != 0){
        buffer[i++] = hexDigits[num & 0x0F];
        num >>= 4;
    }

    while(--i >= 0){
        vgaPutChar(buffer[i]);
    }
}

void vgaWrite(const char* c){
    while(*c != '\0'){
        vgaPutChar(*c);
        c++;
    }
}

void vgaWriteColor(const char* c, uint8_t color){
    uint8_t old = vgaCurrentColor;
    vgaCurrentColor = color;
    vgaWrite(c);
    vgaCurrentColor = old;
}

void vgaScroll(void){
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
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH * 2 + j * 2 + 1] = vgaCurrentColor;
    }

    row = VGA_HEIGHT - 1;
    column = 0;
}
