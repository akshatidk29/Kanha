#ifndef KANHA_ULIB_H
#define KANHA_ULIB_H

#include <stdint.h>
#include <stddef.h>

#include "syscallNum.h"


int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int spawn(const char *path);
int readdir(const char *path, char *buf, size_t maxLen);

void yield(void);
void sleep(uint32_t ms);
void exit(int status) __attribute__((noreturn));

#endif