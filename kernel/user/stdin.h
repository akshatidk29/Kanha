#ifndef KANHA_STDIN_H
#define KANHA_STDIN_H

#include <stdint.h>

void stdinInitialize(void);

void stdinPutChar(char c);
int stdinGetChar(void);

int stdinIsEmpty(void);

#endif
