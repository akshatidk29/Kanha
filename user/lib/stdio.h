#ifndef KANHA_STDIO_H
#define KANHA_STDIO_H

#include <stddef.h>
#include <stdint.h>

int putchar(char c);
int puts(const char *s);

int printf(const char *fmt, ...);

char getchar(void);
int getline(char *buf, size_t maxLen);

#endif
