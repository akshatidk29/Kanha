#include "syscall.h"
#include "serial.h"
#include "vga.h"
#include "printf.h"
#include "thread.h"
#include "process.h"
#include "scheduler.h"

static int sysWrite(int fd, const char *buf, uint32_t len) {

    if (!buf)
        return -1;

    if (fd == 1 || fd == 2) {
        for (uint32_t i = 0; i < len; i++) {
            serialWriteChar(buf[i]);
            vgaPutChar(buf[i]);
        }
        return len;
    }

    return -1; 
}

void syscallHandler(struct SyscallFrame *frame) {

    uint32_t syscallNumber = frame->eax;

    switch (syscallNumber) {
        
        case SYS_EXIT: {
            int status = (int)frame->ebx;
            struct Process *proc = currentProcess();
            if (proc) {
                printf("Process %d exited with code %d\n", proc->pid, status);
                processExit(status);
            } else {
                printf("Thread %d exited with code %d\n", scheduledThread()->tid, status);
                threadExit();
            }
            break;
        }


        case SYS_WRITE: {
            int fd = (int)frame->ebx;
            const char *buf = (const char *)frame->ecx;
            uint32_t len = (uint32_t)frame->edx;
            frame->eax = sysWrite(fd, buf, len);
            break;
        }

        case SYS_YIELD: {
            yield();
            frame->eax = 0;
            break;
        }

        case SYS_SLEEP: {
            uint32_t ms = (uint32_t)frame->ebx;
            threadSleep(ms);
            frame->eax = 0;
            break;
        }

        default: {
            printf("[SYSCALL] Unknown syscall number: %d\n", syscallNumber);
            frame->eax = -1;
            break;
        }
    }
}
