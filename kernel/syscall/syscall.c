#include "syscall/syscall.h"
#include "drivers/serial.h"
#include "drivers/vga.h"
#include "drivers/stdin.h"
#include "lib/kprintf.h"
#include "task/thread.h"
#include "task/process.h"
#include "task/scheduler.h"
#include "fs/vfs.h"
#include "lib/string.h"

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

static int sysSpawn(const char* path){
    struct VfsNode* node = vfsOpen(path);

    if(!node || node->type != REGULAR)
        return -1;
    struct Process* proc = processCreate(node->deviceData, node->size);
    return proc ? proc->pid : -1;
}

static int sysReadDir(const char* path, char* buf, uint32_t maxLen){
    
    struct VfsNode* node = vfsOpen(path);

    if(!node || node->type != DIR || !buf || maxLen == 0)
        return -1;
    
    uint32_t i = 0;
    uint32_t offset = 0;

    while(1){

        struct VfsNode* entry = vfsReadDir(node, i);

        if(!entry)
            break;
        
        size_t nlen = strlen(entry->name);
        if (offset + nlen + 2 >= maxLen)
            break;

        memcpy(buf + offset, entry->name, nlen);
        offset += nlen;
        buf[offset++] = '\n';
        i++;
    }
    buf[offset] = '\0';
    return (int)i;
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

        case SYS_SPAWN: {
            const char *path = (const char*)frame->ebx;
            if(!path){
                frame->eax = -1;
                break;
            }
            frame->eax = sysSpawn(path);
            break; 
        }

        case SYS_READDIR: {
            const char *path = (const char*)frame->ebx;
            char *buf = (char *)frame->ecx;
            uint32_t maxLen = (uint32_t)frame->edx;
            frame->eax = sysReadDir(path, buf, maxLen);
            break; 
        }

        default: {
            kprintf("Unknown Syscall number: %d\n", syscallNumber);
            frame->eax = -1;
            break;
        }
    }
}
