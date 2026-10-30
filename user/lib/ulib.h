#ifndef KANHA_ULIB_H
#define KANHA_ULIB_H

#include <stdint.h>
#include <stddef.h>

#include "syscallNum.h"


void exit(int status) __attribute__((noreturn));
int write(int fd, const void *buf, size_t count);
int read(int fd, void *buf, size_t count);
void yield(void);
void sleep(uint32_t ms);

#endif