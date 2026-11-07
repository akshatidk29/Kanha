#ifndef KANHA_STDIO_H
#define KANHA_STDIO_H

#include <stddef.h>
#include <stdint.h>

#define COLOR_RESET          "\033[0m"
#define COLOR_BLACK          "\033[30m"
#define COLOR_RED            "\033[31m"
#define COLOR_GREEN          "\033[32m"
#define COLOR_YELLOW         "\033[33m"
#define COLOR_BLUE           "\033[34m"
#define COLOR_MAGENTA        "\033[35m"
#define COLOR_CYAN           "\033[36m"
#define COLOR_WHITE          "\033[37m"

#define COLOR_BRIGHT_RED     "\033[1;31m"
#define COLOR_BRIGHT_GREEN   "\033[1;32m"
#define COLOR_BRIGHT_YELLOW  "\033[1;33m"
#define COLOR_BRIGHT_BLUE    "\033[1;34m"
#define COLOR_BRIGHT_MAGENTA "\033[1;35m"
#define COLOR_BRIGHT_CYAN    "\033[1;36m"
#define COLOR_BRIGHT_WHITE   "\033[1;37m"

int putchar(char c);
int puts(const char *s);

int printf(const char *fmt, ...);

char getchar(void);
int getline(char *buf, size_t maxLen);

#endif
