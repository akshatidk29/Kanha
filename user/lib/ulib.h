#ifndef KANHA_ULIB_H
#define KANHA_ULIB_H

#include <stdint.h>
#include <stddef.h>

#include "syscallNum.h"


#define WNOHANG 1

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR   0x0002
#define O_CREAT  0x0040

int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int open(const char *path, int flags);
int close(int fd);
int mkdir(const char *path);
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