#include <stdarg.h>
#include <stdint.h>

#include "serial.h"
#include "kprintf.h"

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
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    serialWriteString(str);
                    break;
                }
                case 'd': {
                    int32_t num = va_arg(args, int32_t);
                    serialWriteInt(num);
                    break;
                }
                case 'x': {
                    uint32_t num = va_arg(args, uint32_t);
                    serialWriteHex(num);
                    break;
                }
                case '%': {
                    serialWriteChar('%');
                    break;
                }
                default: {
                    serialWriteChar('%');
                    serialWriteChar(*fmt);
                    break;
                }
            }
        }else{
            serialWriteChar(*fmt);
        }
        fmt++;
    }
    va_end(args);
}