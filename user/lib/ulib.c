#include "ulib.h"

static inline int syscall0(int syscallNum) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(syscallNum) : "memory");
    return ret;
}

static inline int syscall1(int syscallNum, int arg1) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(syscallNum), "b"(arg1) : "memory");
    return ret;
}

static inline int syscall2(int syscallNum, int arg1, int arg2) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(syscallNum), "b"(arg1), "c"(arg2) : "memory");
    return ret;
}

static inline int syscall3(int syscallNum, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(syscallNum), "b"(arg1), "c"(arg2), "d"(arg3) : "memory");
    return ret;
}


void exit(int status) {
    syscall1(SYS_EXIT, status);
    while (1);
}

int write(int fd, const void *buf, size_t count) {
    return syscall3(SYS_WRITE, fd, (int)buf, (int)count);
}

int read(int fd, void *buf, size_t count) {
    return syscall3(SYS_READ, fd, (int)buf, (int)count);
}

void yield(void) {
    syscall0(SYS_YIELD);
}

void sleep(uint32_t ms) {
    syscall1(SYS_SLEEP, (int)ms);
}

int spawn(const char *path) {
    return syscall1(SYS_SPAWN, (int)path);
}

int fork(void) {
    return syscall0(SYS_FORK);
}

int exec(const char *path) {
    return syscall1(SYS_EXEC, (int)path);
}

int readdir(const char *path, char *buf, size_t maxLen) {
    return syscall3(SYS_READDIR, (int)path, (int)buf, (int)maxLen);
}
