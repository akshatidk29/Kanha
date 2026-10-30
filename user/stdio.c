#include "stdio.h"
#include "ulib.h"
#include <stdarg.h>
#include <stdint.h>

int putchar(char c) {
    return write(1, &c, 1);
}

int puts(const char *s) {
    if (!s) 
        return -1;
    while (*s) {
        putchar(*s++);
    }
    putchar('\n');
    return 0;
}

static void printStr(const char *str) {
    if (!str) {
        printStr("(null)");
        return;
    }
    while (*str) {
        putchar(*str++);
    }
}

static void printUint(uint32_t num) {
    if (num == 0) {
        putchar('0');
        return;
    }
    char buf[12];
    int i = 0;
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        putchar(buf[j]);
    }
}

static void printInt(int32_t num) {
    if (num == 0) {
        putchar('0');
        return;
    }
    if (num < 0) {
        putchar('-');
        printUint((uint32_t)(-num));
    } else {
        printUint((uint32_t)num);
    }
}

static void printHex(uint32_t num) {
    if (num == 0) {
        putchar('0');
        return;
    }
    const char hexDigits[] = "0123456789ABCDEF";
    char buf[9];
    int i = 0;
    while (num > 0) {
        buf[i++] = hexDigits[num & 0x0F];
        num >>= 4;
    }
    for (int j = i - 1; j >= 0; j--) {
        putchar(buf[j]);
    }
}

int printf(const char *fmt, ...) {

    if (!fmt) 
        return 0;

    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char *);
                    printStr(str);
                    break;
                }
                case 'd':
                case 'i': {
                    int32_t num = va_arg(args, int32_t);
                    printInt(num);
                    break;
                }
                case 'u': {
                    uint32_t num = va_arg(args, uint32_t);
                    printUint(num);
                    break;
                }
                case 'x':
                case 'X': {
                    uint32_t num = va_arg(args, uint32_t);
                    printHex(num);
                    break;
                }
                case 'p': {
                    uint32_t ptr = (uint32_t)va_arg(args, void *);
                    printStr("0x");
                    printHex(ptr);
                    break;
                }
                case '%': {
                    putchar('%');
                    break;
                }
                default: {
                    putchar('%');
                    putchar(*fmt);
                    break;
                }
            }
        } else {
            putchar(*fmt);
        }
        fmt++;
    }

    va_end(args);
    return 0;
}

char getchar(void) {
    char c = 0;
    while (read(0, &c, 1) <= 0) {
        yield();
    }
    return c;
}

int getline(char *buf, size_t maxLen) {
    if (!buf || maxLen <= 1) 
        return 0;
    size_t i = 0;
    while (i < maxLen - 1) {
        char c = getchar();
        if (c == '\r' || c == '\n') {
            break;
        } else if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
            }
        } else if (c >= 32 && c <= 126) {
            buf[i++] = c;
        }
    }
    buf[i] = '\0';
    return (int)i;
}
