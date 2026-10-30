#include "syscall.h"
#include "serial.h"
#include "vga.h"
#include "kprintf.h"
#include "thread.h"
#include "process.h"
#include "scheduler.h"
#include "stdin.h"

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

static int sysRead(int fd, char *buf, uint32_t len) {

    if (!buf || len == 0)
        return -1;

    if (fd == 0) {
        uint32_t bytesRead = 0;
        while (bytesRead < len) {
            int c = stdinGetChar();
            if (c == -1) {
                if (bytesRead > 0) {
                    break;
                }
                yield();
                continue;
            }
            buf[bytesRead++] = (char)c;
            if ((char)c == '\n') {
                break;
            }
        }
        return (int)bytesRead;
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
                kprintf("Process %d exited with code %d\n", proc->pid, status);
                processExit(status);
            } else {
                kprintf("Thread %d exited with code %d\n", scheduledThread()->tid, status);
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

        case SYS_READ: {
            int fd = (int)frame->ebx;
            char *buf = (char *)frame->ecx;
            uint32_t len = (uint32_t)frame->edx;
            frame->eax = sysRead(fd, buf, len);
            break;
        }

        default: {
            kprintf("Unknown Syscall number: %d\n", syscallNumber);
            frame->eax = -1;
            break;
        }
    }
}
