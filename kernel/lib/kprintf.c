#include <stdarg.h>
#include <stdint.h>

#include "drivers/serial.h"
#include "lib/kprintf.h"
#include "drivers/vga.h"

void kprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt) {

        if ((*fmt) == '%'){
            fmt++;
            switch(*fmt){
                case 'c': {
                    char c = (char)va_arg(args, int);
                    serialWriteChar(c);
                    vgaPutChar(c);
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    serialWriteString(str);
                    vgaWrite(str);
                    break;
                }
                case 'd': {
                    int32_t num = va_arg(args, int32_t);
                    serialWriteInt(num);
                    vgaPutInt(num);
                    break;
                }
                case 'x': {
                    uint32_t num = va_arg(args, uint32_t);
                    serialWriteHex(num);
                    vgaPutHex(num);
                    break;
                }
                case '%': {
                    serialWriteChar('%');
                    vgaPutChar('%');
                    break;
                }
                default: {
                    serialWriteChar('%');
                    serialWriteChar(*fmt);
                    vgaPutChar('%');
                    vgaPutChar(*fmt);
                    break;
                }
            }
        }else{
            serialWriteChar(*fmt);
            vgaPutChar(*fmt);
        }
        fmt++;
    }
    va_end(args);
}