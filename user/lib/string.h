#ifndef KANHA_USER_STRING_H
#define KANHA_USER_STRING_H

#include <stddef.h>
#include <stdint.h>

size_t strlen(const char *str);

int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);

#endif
