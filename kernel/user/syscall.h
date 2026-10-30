#ifndef KANHA_SYSCALL_H
#define KANHA_SYSCALL_H

#include <stdint.h>
#include "syscallNum.h"

struct SyscallFrame {
    uint32_t edi, esi, ebp, espDummy, ebx, edx, ecx, eax;       // Pushed by pusha
    uint32_t gs, fs, es, ds;                                    // Pushed segment registers
    uint32_t eip, cs, eflags, userEsp, userSS;                  // Pushed by CPU on interrupt
};  

void syscallHandler(struct SyscallFrame *frame);

#endif
