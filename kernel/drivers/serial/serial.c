#include "io.h"
#include "serial.h"

int serialInitialize(void) {
    
    outB(SERIAL_COM1 + 1, 0x00);  // Disable all interrupts
    outB(SERIAL_COM1 + 3, 0x80);  // Enable DLAB 
    outB(SERIAL_COM1 + 0, 0x03);  // Set divisor to 3 (38400 baud)
    outB(SERIAL_COM1 + 1, 0x00);  // Divisor high byte
    outB(SERIAL_COM1 + 3, 0x03);  // 8 bits, no parity, one stop bit
    outB(SERIAL_COM1 + 2, 0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    outB(SERIAL_COM1 + 4, 0x0B);  // IRQs enabled, RTS/DSR set
    outB(SERIAL_COM1 + 4, 0x1E);  // Enable loopback mode
    outB(SERIAL_COM1 + 0, 0xAE);  // Send test byte

    if (inB(SERIAL_COM1 + 0) != 0xAE) {
        return -1;                // Faulty serial chip or not present
    }
    outB(SERIAL_COM1 + 4, 0x0F);  // Normal operation mode
    return 0;
}

int serialTransmitReady(void) {
    return inB(SERIAL_COM1 + 5) & 0x20;
}

void serialWriteChar(char c) {  
    
    if (c == '\n') {
        while (!serialTransmitReady());
        outB(SERIAL_COM1, '\r');
    }

    while (!serialTransmitReady());
    outB(SERIAL_COM1, (uint8_t)c);
}
void serialWriteString(const char *str) {

    if (!str) 
        return;

    for (size_t i = 0; str[i] != '\0'; i++) {
        serialWriteChar(str[i]);
    }
}

void serialWriteUnsignedInt(uint32_t num){

    if(num == 0){
        serialWriteChar('0');
        return;
    }

    int i = 0;
    char buffer[12];

    while(num != 0){
        buffer[i] = (num % 10) + '0';
        num /= 10;
        i++;
    }

    i--;
    while(i >= 0){
        serialWriteChar(buffer[i]);
        i--;
    }
}
void serialWriteUnsignedInt64(uint64_t num){
    
    if(num == 0){
        serialWriteChar('0');
        return;
    }

    int i = 0;
    char buffer[22];

    while(num != 0){
        buffer[i] = (num % 10) + '0';
        num /= 10;
        i++;
    }

    i--;
    while(i >= 0){
        serialWriteChar(buffer[i]);
        i--;
    }
}

void serialWriteInt(int32_t num){

    if(num == 0){
        serialWriteChar('0');
        return;
    }
    
    if(num < 0){
        serialWriteChar('-');
        num = -num;
    }

    int i = 0;
    char buffer[22];

    while(num != 0){
        buffer[i] = (num % 10) + '0';
        num /= 10;
        i++;
    }

    i--;
    while(i >= 0){
        serialWriteChar(buffer[i]);
        i--;
    }
}
void serialWriteInt64(int64_t n){

    if(n == 0){
        serialWriteChar('0');
        return;
    }

    if(n < 0){
        serialWriteChar('-');
        n = -n;
    }

    int i = 0;
    char buffer[22];

    while(n != 0){
        buffer[i] = (n % 10) + '0';
        n /= 10;
        i++;
    }

    i--;
    while(i >= 0){
        serialWriteChar(buffer[i]);
        i--;
    }
}

void serialWriteHex(uint32_t num){
    if(num == 0){
        serialWriteChar('0');
        return;
    }

    char hexDigits[] = "0123456789ABCDEF";
    char buffer[9];
    int i = 0;

    while(num != 0){
        buffer[i++] = hexDigits[num & 0x0F];
        num >>= 4;
    }

    while(--i >= 0){
        serialWriteChar(buffer[i]);
    }
}


int serialReceived(void) {
    return inB(SERIAL_COM1 + 5) & 0x01;
}

char serialReadChar(void) {
    while (!serialReceived());
    return (char)inB(SERIAL_COM1);
}
