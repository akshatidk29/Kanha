#ifndef KANHA_SYSCALL_H
#define KANHA_SYSCALL_H

#include <stdint.h>
#include "syscall/syscallNum.h"

struct SyscallFrame {                                 // Trap Frame
  
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t espDummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;                                     // Pushed by pusha

    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;                                      // Pushed segment registers

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t userEsp;
    uint32_t userSS;                                  // Pushed by CPU on interrupt
};  

void syscallHandler(struct SyscallFrame *frame);

#endif
