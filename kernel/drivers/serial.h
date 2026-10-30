#ifndef KANHA_SERIAL_H
#define KANHA_SERIAL_H

#include <stdint.h>
#include <stddef.h>

#define SERIAL_COM1 0x3F8

void irqHandlerSerial(void);

int serialInitialize(void);
int serialTransmitReady(void);

void serialWriteChar(char c);
void serialWriteString(const char *str);

void serialWriteUnsignedInt(uint32_t num);
void serialWriteUnsignedInt64(uint64_t num);

void serialWriteInt(int32_t num);
void serialWriteInt64(int64_t num);

void serialWriteHex(uint32_t num);

int serialReceived(void);
char serialReadChar(void);

#endif
