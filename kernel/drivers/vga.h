#ifndef KANHA_VGA_H
#define KANHA_VGA_H

#include <stdint.h>

enum VgaColor {
    VGA_BLACK         = 0x00,
    VGA_BLUE          = 0x01,
    VGA_GREEN         = 0x02,
    VGA_CYAN          = 0x03,
    VGA_RED           = 0x04,
    VGA_MAGENTA       = 0x05,
    VGA_BROWN         = 0x06,
    VGA_LIGHT_GRAY    = 0x07,
    VGA_DARK_GRAY     = 0x08,
    VGA_LIGHT_BLUE    = 0x09,
    VGA_LIGHT_GREEN   = 0x0A,
    VGA_LIGHT_CYAN    = 0x0B,
    VGA_LIGHT_RED     = 0x0C,
    VGA_LIGHT_MAGENTA = 0x0D,
    VGA_YELLOW        = 0x0E,
    VGA_WHITE         = 0x0F,
};

void vgaInitialize(void);
void vgaSetColor(uint8_t color);
uint8_t vgaGetColor(void);

void vgaSetCursor(uint8_t row, uint8_t column);
void vgaCursor(void);

void vgaPutChar(char c);
void vgaClearChar(void);

void vgaWrite(const char* c);
void vgaWriteColor(const char* c, uint8_t color);

void vgaPutInt(int32_t n);
void vgaPutInt64(int64_t n);

void vgaPutHex(uint32_t num);

void vgaPutUnsignedInt(uint32_t n);
void vgaPutUnsignedInt64(uint64_t n);

void vgaScroll(void);

#endif
