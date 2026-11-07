#ifndef KANHA_BCACHE_H
#define KANHA_BCACHE_H

#include <stdint.h>
#include <stddef.h>

#define BCACHE_BLOCK_SIZE  1024
#define BCACHE_NUM_BUFFERS 32

struct Buffer {
    uint8_t data[BCACHE_BLOCK_SIZE]; 
    uint32_t blockNum;               // Block number
    uint8_t drive;                   // Drive index
    uint8_t valid;                   // 1 if buffer holds valid data
    uint8_t dirty;                   // 1 if buffer has been modified
    uint32_t refCount;               // Number of active users
    uint32_t lastAccess;             // Clock counter
};

void bcacheInitialize(void);

// Get a cached block
struct Buffer *bcacheGet(uint8_t drive, uint32_t blockNum);

// Releases a reference
void bcacheRelease(struct Buffer *buf);

// Flushes a modified buffer
void bcacheWrite(struct Buffer *buf);

// Flushes all dirty buffers
void bcacheSync(void);

// Reads a block
int bcacheReadBlock(uint8_t drive, uint32_t blockNum, void *dest);

// Writes a block
int bcacheWriteBlock(uint8_t drive, uint32_t blockNum, const void *src);

#endif

