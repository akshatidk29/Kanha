#include "fs/bcache.h"
#include "drivers/ata.h"
#include "task/lock.h"
#include "lib/kprintf.h"
#include "lib/string.h"

#define SECTORS_PER_BLOCK (BCACHE_BLOCK_SIZE / ATA_SECTOR_SIZE)

static struct Buffer buffers[BCACHE_NUM_BUFFERS];
static struct SpinLock bcacheLock;
static uint32_t bcacheClock = 0;

void bcacheInitialize(void) {
    spinLockInit(&bcacheLock);
    memset(buffers, 0, sizeof(buffers));
    bcacheClock = 0;
    kprintf("Initialized %d BCache buffers\n", BCACHE_NUM_BUFFERS);
}

struct Buffer *bcacheGet(uint8_t drive, uint32_t blockNum) {
    spinLockAcquire(&bcacheLock);
    bcacheClock++;

    // Check if the requested block is already in the cache
    for (int i = 0; i < BCACHE_NUM_BUFFERS; i++) {
        if (buffers[i].valid && buffers[i].drive == drive && buffers[i].blockNum == blockNum) {
            buffers[i].refCount++;
            buffers[i].lastAccess = bcacheClock;
            spinLockRelease(&bcacheLock);
            return &buffers[i];
        }
    }

    // Cache miss
    int victim = -1;
    uint32_t oldestTime = 0xFFFFFFFF;

    for (int i = 0; i < BCACHE_NUM_BUFFERS; i++) {
        if (buffers[i].refCount == 0) {
            // Unused buffers take immediate precedence
            if (!buffers[i].valid) {
                victim = i;
                break;
            }
            if (buffers[i].lastAccess < oldestTime) {
                oldestTime = buffers[i].lastAccess;
                victim = i;
            }
        }
    }

    if (victim == -1) {
        kprintf("BCACHE ERROR: Out of buffers!");
        spinLockRelease(&bcacheLock);
        return NULL;
    }

    struct Buffer *b = &buffers[victim];

    // If the evicted buffer has unsaved changes, flush it to disk first
    if (b->valid && b->dirty) {
        uint32_t oldLba = b->blockNum * SECTORS_PER_BLOCK;
        ataWriteSector(b->drive, oldLba, b->data);
        ataWriteSector(b->drive, oldLba + 1, b->data + ATA_SECTOR_SIZE);
        b->dirty = 0;
    }

    // Set up new buffer metadata
    b->drive = drive;
    b->blockNum = blockNum;
    b->valid = 0;
    b->dirty = 0;
    b->refCount = 1;
    b->lastAccess = bcacheClock;

    // Read block from the drive
    uint32_t lba = blockNum * SECTORS_PER_BLOCK;
    if (ataReadSector(drive, lba, b->data) != 0 ||
        ataReadSector(drive, lba + 1, b->data + ATA_SECTOR_SIZE) != 0) {
        kprintf("BCACHE ERROR: Disk read failed for block %d on drive %d\n", blockNum, drive);
        b->refCount = 0;
        spinLockRelease(&bcacheLock);
        return NULL;
    }

    b->valid = 1;
    spinLockRelease(&bcacheLock);
    return b;
}

void bcacheRelease(struct Buffer *buf) {
    if (!buf)
        return;

    spinLockAcquire(&bcacheLock);
    if (buf->refCount > 0)
        buf->refCount--;
    spinLockRelease(&bcacheLock);
}

void bcacheWrite(struct Buffer *buf) {
    if (!buf)
        return;

    spinLockAcquire(&bcacheLock);
    buf->dirty = 1;

    // Write-through
    uint32_t lba = buf->blockNum * SECTORS_PER_BLOCK;
    ataWriteSector(buf->drive, lba, buf->data);
    ataWriteSector(buf->drive, lba + 1, buf->data + ATA_SECTOR_SIZE);

    buf->dirty = 0;
    spinLockRelease(&bcacheLock);
}

void bcacheSync(void) {
    spinLockAcquire(&bcacheLock);
    for (int i = 0; i < BCACHE_NUM_BUFFERS; i++) {
        if (buffers[i].valid && buffers[i].dirty) {
            uint32_t lba = buffers[i].blockNum * SECTORS_PER_BLOCK;
            ataWriteSector(buffers[i].drive, lba, buffers[i].data);
            ataWriteSector(buffers[i].drive, lba + 1, buffers[i].data + ATA_SECTOR_SIZE);
            buffers[i].dirty = 0;
        }
    }
    spinLockRelease(&bcacheLock);
}

int bcacheReadBlock(uint8_t drive, uint32_t blockNum, void *dest) {
    if (!dest)
        return -1;

    struct Buffer *b = bcacheGet(drive, blockNum);
    if (!b)
        return -1;

    memcpy(dest, b->data, BCACHE_BLOCK_SIZE);
    bcacheRelease(b);
    return 0;
}

int bcacheWriteBlock(uint8_t drive, uint32_t blockNum, const void *src) {
    if (!src) {
        return -1;
    }

    struct Buffer *b = bcacheGet(drive, blockNum);
    if (!b) {
        return -1;
    }

    memcpy(b->data, src, BCACHE_BLOCK_SIZE);
    bcacheWrite(b);
    bcacheRelease(b);
    return 0;
}

