#ifndef KANHA_BCACHE_H
#define KANHA_BCACHE_H

#include <stdint.h>
#include <stddef.h>

#define BCACHE_BLOCK_SIZE  1024
#define BCACHE_NUM_BUFFERS 32

struct Buffer {
    uint8_t data[BCACHE_BLOCK_SIZE]; 
    uint32_t blockNum;               // Disk block number
    uint8_t drive;                   // Drive index
    uint8_t valid;                   // 1 if buffer holds valid data from disk
    uint8_t dirty;                   // 1 if buffer has been modified in RAM
    uint32_t refCount;               // Number of active users holding this buffer
    uint32_t lastAccess;             // Clock counter
};

void bcacheInitialize(void);

// Get a cached block from disk
struct Buffer *bcacheGet(uint8_t drive, uint32_t blockNum);

// Releases a reference to a previously obtained buffer
void bcacheRelease(struct Buffer *buf);

// Flushes a modified buffer to disk
void bcacheWrite(struct Buffer *buf);

// Flushes all dirty buffers to disk
void bcacheSync(void);

// Reads a 1024-byte block directly into destination memory
int bcacheReadBlock(uint8_t drive, uint32_t blockNum, void *dest);

// Writes a 1024-byte block directly from source memory to disk
int bcacheWriteBlock(uint8_t drive, uint32_t blockNum, const void *src);

#endif

