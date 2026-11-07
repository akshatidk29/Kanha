#ifndef KANHA_ULIB_H
#define KANHA_ULIB_H

#include <stdint.h>
#include <stddef.h>

#include "syscallNum.h"


#define WNOHANG 1

int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int spawn(const char *path);
int fork(void);
int exec(const char *path, char *const argv[]);
int waitpid(int pid, int *status, int options);
int wait(int *status);
int readDir(const char *path, char *buf, size_t maxLen);

void yield(void);
void sleep(uint32_t ms);
void exit(int status) __attribute__((noreturn));

#endif