#include "hardware/io.h"
#include "hardware/pic.h"
#include "drivers/vga.h"
#include "drivers/serial.h"
#include "drivers/keyboard.h"
#include "drivers/stdin.h"
#include "task/process.h"

static const char codeToAscii[128] = {
    [0x01] = 0,     // Escape
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',
    [0x0F] = '\t',

    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = '\n',
    [0x1D] = 0,     // Left Ctrl

    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    [0x2A] = 0,     // Left Shift
    [0x2B] = '\\',

    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x36] = 0,     // Right Shift
    [0x37] = '*',   // Keypad *
    [0x38] = 0,     // Left Alt
    [0x39] = ' ',   // Space
    [0x3A] = 0,     // Caps Lock

    // Numeric Keypad
    [0x47] = '7',
    [0x48] = '8',
    [0x49] = '9',
    [0x4A] = '-',
    [0x4B] = '4',
    [0x4C] = '5',
    [0x4D] = '6',
    [0x4E] = '+',
    [0x4F] = '1',
    [0x50] = '2',
    [0x51] = '3',
    [0x52] = '0',
    [0x53] = '.',
};

static int shiftPressed = 0;   
static int ctrlPressed  = 0;   
static int altPressed   = 0;   
static int capsLock     = 0;   
static int numLock      = 1;   // Default NumLock to enabled
static int extended     = 0; 

void keyboardInitialize(void) {
    shiftPressed = 0;
    ctrlPressed = 0;
    altPressed = 0;
    capsLock = 0;
    numLock = 1;
    extended = 0;
}

void irqHandlerKeyboard(void) {
    uint8_t status = inB(0x64);
    if (!(status & 0x01)) {
        picSendEOI(1);
        return;
    }

    uint8_t scanByte = inB(0x60);

    // Extended scancode prefix
    if (scanByte == 0xE0) {
        extended = 1;
        picSendEOI(1);
        return;
    }

    // Extended keys
    if (extended) {
        extended = 0;
        if (!(scanByte & 0x80)) {
            // Extended key make codes
            if (scanByte == 0x48) {        // Up Arrow
                stdinPutChar('\033');
                stdinPutChar('[');
                stdinPutChar('A');
            } else if (scanByte == 0x50) { // Down Arrow
                stdinPutChar('\033');
                stdinPutChar('[');
                stdinPutChar('B');
            } else if (scanByte == 0x4D) { // Right Arrow
                stdinPutChar('\033');
                stdinPutChar('[');
                stdinPutChar('C');
            } else if (scanByte == 0x4B) { // Left Arrow
                stdinPutChar('\033');
                stdinPutChar('[');
                stdinPutChar('D');
            } else if (scanByte == 0x35) { // Keypad /
                vgaPutChar('/');
                serialWriteChar('/');
                stdinPutChar('/');
            } else if (scanByte == 0x1C) { // Keypad Enter
                vgaPutChar('\n');
                serialWriteChar('\n');
                stdinPutChar('\n');
            } else if (scanByte == 0x1D) { // Right Ctrl
                ctrlPressed = 1;
            } else if (scanByte == 0x38) { // Right Alt
                altPressed = 1;
            }
        } else {
            // Extended key break codes
            uint8_t rel = scanByte & 0x7F;
            if (rel == 0x1D) {
                ctrlPressed = 0;
            } else if (rel == 0x38) {
                altPressed = 0;
            }
        }
        picSendEOI(1);
        return;
    }

    // Normal key release
    if (scanByte & 0x80) {
        uint8_t releaseCode = scanByte & 0x7F;
        if (releaseCode == 0x2A || releaseCode == 0x36) {
            shiftPressed = 0;
        } else if (releaseCode == 0x1D) {
            ctrlPressed = 0;
        } else if (releaseCode == 0x38) {
            altPressed = 0;
        }
        picSendEOI(1);
        return;
    }

    // Normal key press
    if (scanByte == 0x2A || scanByte == 0x36) {
        shiftPressed = 1;
        picSendEOI(1);
        return;
    }
    if (scanByte == 0x1D) {
        ctrlPressed = 1;
        picSendEOI(1);
        return;
    }
    if (scanByte == 0x38) {
        altPressed = 1;
        picSendEOI(1);
        return;
    }
    if (scanByte == 0x3A) {
        capsLock = !capsLock;
        picSendEOI(1);
        return;
    }
    if (scanByte == 0x45) { // NumLock toggle
        numLock = !numLock;
        picSendEOI(1);
        return;
    }

    // Numpad keys when NumLock is OFF
    if (!numLock && scanByte >= 0x47 && scanByte <= 0x53) {
        if (scanByte == 0x48) {        // Keypad 8 -> Up Arrow
            stdinPutChar('\033');
            stdinPutChar('[');
            stdinPutChar('A');
        } else if (scanByte == 0x50) { // Keypad 2 -> Down Arrow
            stdinPutChar('\033');
            stdinPutChar('[');
            stdinPutChar('B');
        } else if (scanByte == 0x4B) { // Keypad 4 -> Left Arrow
            stdinPutChar('\033');
            stdinPutChar('[');
            stdinPutChar('D');
        } else if (scanByte == 0x4D) { // Keypad 6 -> Right Arrow
            stdinPutChar('\033');
            stdinPutChar('[');
            stdinPutChar('C');
        }
        picSendEOI(1);
        return;
    }

    // Unmapped scancode
    if (scanByte >= 128) {
        picSendEOI(1);
        return;
    }

    char c = codeToAscii[scanByte];
    if (c == 0) {
        picSendEOI(1);
        return;
    }

    // Ctrl key combinations
    if (ctrlPressed) {
        if (c >= 'a' && c <= 'z') {
            c = c - 'a' + 1;
        } else if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 1;
        }

        if (c == 3) { // Ctrl+C
            int fg = processGetForeground();
            if (fg > 1) { 
                vgaWrite("^C\n");
                serialWriteString("^C\n");
                stdinClear();
                processKill(fg, 130);
                picSendEOI(1);
                return;
            }
        }

        // Send directly to stdin without echoing control characters to VGA
        stdinPutChar(c);
        picSendEOI(1);
        return;
    }

    // Shift and Caps Lock transformations
    if ((shiftPressed ^ capsLock) && (c >= 'a' && c <= 'z')) {
        c -= 32;
    } else if (shiftPressed) {
        switch (c) {
            case '1': c = '!'; break;
            case '2': c = '@'; break;
            case '3': c = '#'; break;
            case '4': c = '$'; break;
            case '5': c = '%'; break;
            case '6': c = '^'; break;
            case '7': c = '&'; break;
            case '8': c = '*'; break;
            case '9': c = '('; break;
            case '0': c = ')'; break;
            case '-': c = '_'; break;
            case '=': c = '+'; break;
            case '[': c = '{'; break;
            case ']': c = '}'; break;
            case ';': c = ':'; break;
            case '\'': c = '"'; break;
            case '`': c = '~'; break;
            case '\\': c = '|'; break;
            case ',': c = '<'; break;
            case '.': c = '>'; break;
            case '/': c = '?'; break;
            default: break;
        }
    }

    vgaPutChar(c);
    serialWriteChar(c);
    stdinPutChar(c);

    picSendEOI(1);
}
