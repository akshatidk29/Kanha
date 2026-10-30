#include "drivers/stdin.h"
#include "task/lock.h"

#define STDIN_BUF_SIZE 256

static char stdinBuf[STDIN_BUF_SIZE];
static volatile uint32_t stdinHead = 0;
static volatile uint32_t stdinTail = 0;

void stdinInitialize(void) {
    stdinHead = 0;
    stdinTail = 0;
}

void stdinPutChar(char c) {
    uint32_t next = (stdinHead + 1) % STDIN_BUF_SIZE;
    if (next != stdinTail) {
        stdinBuf[stdinHead] = c;
        stdinHead = next;
    }
}

int stdinGetChar(void) {
    uint32_t flags = disableInterrupts();
    if (stdinHead == stdinTail) {
        restoreInterrupts(flags);
        return -1;
    }
    char c = stdinBuf[stdinTail];
    stdinTail = (stdinTail + 1) % STDIN_BUF_SIZE;
    restoreInterrupts(flags);
    return (unsigned char)c;
}

int stdinIsEmpty(void) {
    uint32_t flags = disableInterrupts();
    int empty = (stdinHead == stdinTail);
    restoreInterrupts(flags);
    return empty;
}
