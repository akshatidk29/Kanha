#ifndef KANHA_SYSCALL_H
#define KANHA_SYSCALL_H

#include <stdint.h>

#define SYS_EXIT   1
#define SYS_WRITE  2
#define SYS_YIELD  3
#define SYS_SLEEP  4

struct SyscallFrame {
    uint32_t edi, esi, ebp, espDummy, ebx, edx, ecx, eax;       // Pushed by pusha
    uint32_t gs, fs, es, ds;                                    // Pushed segment registers
    uint32_t eip, cs, eflags, userEsp, userSS;                  // Pushed by CPU on interrupt
};  

void syscallHandler(struct SyscallFrame *frame);

#endif
