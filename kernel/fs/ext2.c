#include "fs/ext2.h"
#include "fs/bcache.h"
#include "drivers/ata.h"
#include "lib/kprintf.h"
#include "lib/string.h"
#include "mm/heap.h"

static uint8_t ext2Drive = ATA_DRIVE_SLAVE;

static struct Ext2Superblock sb;
static struct Ext2GroupDesc *groupDescs = NULL;

static uint32_t blockSize = 1024;
static uint32_t numGroups = 0;

static struct VfsNode ext2RootDir;

int ext2ReadInode(uint32_t inodeNum, struct Ext2Inode *outInode) {
    if (!outInode || inodeNum == 0) 
        return -1;

    uint32_t groupIndex = (inodeNum - 1) / sb.inodesPerGroup;
    uint32_t indexInGroup = (inodeNum - 1) % sb.inodesPerGroup;
    uint32_t inodeSize = sb.inodeSize ? sb.inodeSize : sizeof(struct Ext2Inode);

    if (groupIndex >= numGroups) 
        return -1;

    uint32_t blockNum = groupDescs[groupIndex].inodeTable + (indexInGroup * inodeSize) / blockSize;
    uint32_t offsetInBlock = (indexInGroup * inodeSize) % blockSize;

    struct Buffer *b = bcacheGet(ext2Drive, blockNum);
    if (!b) 
        return -1;

    memcpy(outInode, b->data + offsetInBlock, sizeof(struct Ext2Inode));
    bcacheRelease(b);
    return 0;
}

int ext2WriteInode(uint32_t inodeNum, const struct Ext2Inode *inInode) {
    if (!inInode || inodeNum == 0) 
        return -1;

    uint32_t groupIndex = (inodeNum - 1) / sb.inodesPerGroup;
    uint32_t indexInGroup = (inodeNum - 1) % sb.inodesPerGroup;
    uint32_t inodeSize = sb.inodeSize ? sb.inodeSize : sizeof(struct Ext2Inode);

    if (groupIndex >= numGroups) 
        return -1;

    uint32_t blockNum = groupDescs[groupIndex].inodeTable + (indexInGroup * inodeSize) / blockSize;
    uint32_t offsetInBlock = (indexInGroup * inodeSize) % blockSize;

    struct Buffer *b = bcacheGet(ext2Drive, blockNum);
    if (!b)
        return -1;

    memcpy(b->data + offsetInBlock, inInode, sizeof(struct Ext2Inode));
    bcacheWrite(b);
    bcacheRelease(b);
    return 0;
}

static void ext2SyncMetadata(void) {
    bcacheWriteBlock(ext2Drive, 1, &sb);

    uint32_t descTableBlock = (blockSize == 1024) ? 2 : 1;
    size_t descTableBytes = numGroups * sizeof(struct Ext2GroupDesc);
    struct Buffer *gb = bcacheGet(ext2Drive, descTableBlock);

    if (gb) {
        memcpy(gb->data, groupDescs, descTableBytes);
        bcacheWrite(gb);
        bcacheRelease(gb);
    }
}

uint32_t ext2AllocBlock(void) {
    for (uint32_t g = 0; g < numGroups; g++) {
        if (groupDescs[g].freeBlocksCount == 0)
            continue;

        struct Buffer *b = bcacheGet(ext2Drive, groupDescs[g].blockBitmap);
        if (!b)
            continue;

        for (uint32_t i = 0; i < blockSize; i++) {
            if (b->data[i] != 0xFF) {
                for (uint8_t bit = 0; bit < 8; bit++) {
                    if (!(b->data[i] & (1 << bit))) {
                        b->data[i] |= (1 << bit);
                        bcacheWrite(b);
                        bcacheRelease(b);

                        groupDescs[g].freeBlocksCount--;
                        sb.freeBlocksCount--;
                        ext2SyncMetadata();

                        uint32_t blockNum = g * sb.blocksPerGroup + (i * 8 + bit) + sb.firstDataBlock;

                        uint8_t zeroBlock[1024];
                        memset(zeroBlock, 0, sizeof(zeroBlock));
                        bcacheWriteBlock(ext2Drive, blockNum, zeroBlock);

                        return blockNum;
                    }
                }
            }
        }
        bcacheRelease(b);
    }
    kprintf("EXT2: Out of disk space.\n");
    return 0;
}

uint32_t ext2AllocInode(void) {
    for (uint32_t g = 0; g < numGroups; g++) {
        if (groupDescs[g].freeInodesCount == 0)
            continue;

        struct Buffer *b = bcacheGet(ext2Drive, groupDescs[g].inodeBitmap);
        if (!b)
            continue;

        uint32_t bytesToScan = sb.inodesPerGroup / 8;
        if (bytesToScan > blockSize)
            bytesToScan = blockSize;

        for (uint32_t i = 0; i < bytesToScan; i++) {
            if (b->data[i] != 0xFF) {
                for (uint8_t bit = 0; bit < 8; bit++) {
                    if (!(b->data[i] & (1 << bit))) {
                        b->data[i] |= (1 << bit);
                        bcacheWrite(b);
                        bcacheRelease(b);

                        groupDescs[g].freeInodesCount--;
                        sb.freeInodesCount--;
                        ext2SyncMetadata();

                        uint32_t inodeNum = g * sb.inodesPerGroup + (i * 8 + bit) + 1;
                        return inodeNum;
                    }
                }
            }
        }
        bcacheRelease(b);
    }
    kprintf("EXT2: Out of inodes.\n");
    return 0;
}

int ext2Read(struct VfsNode *node, uint32_t offset, uint8_t *buffer, size_t size) {
    if (!node || !buffer)
        return -1;

    uint32_t inodeNum = (uint32_t)node->deviceData;
    struct Ext2Inode inode;
    if (ext2ReadInode(inodeNum, &inode) != 0)
        return -1;

    if (offset >= inode.size)
        return 0; // End of file

    size_t toRead = size;
    if (toRead > inode.size - offset)
        toRead = inode.size - offset;

    size_t totalRead = 0;
    uint32_t ptrsPerBlock = blockSize / sizeof(uint32_t);

    while (totalRead < toRead) {

        uint32_t currentOffset = offset + totalRead;
        uint32_t fileBlock = currentOffset / blockSize;
        uint32_t blockOffset = currentOffset % blockSize;
        size_t chunk = blockSize - blockOffset;

        if (chunk > toRead - totalRead)
            chunk = toRead - totalRead;

        uint32_t diskBlock = 0;

        if (fileBlock < 12) {
            diskBlock = inode.blockPointers[fileBlock];
        } else if (fileBlock < 12 + ptrsPerBlock) {
            uint32_t indirectBlock = inode.blockPointers[12];
            if (indirectBlock != 0) {
                struct Buffer *ib = bcacheGet(ext2Drive, indirectBlock);
                if (ib) {
                    uint32_t *table = (uint32_t *)ib->data;
                    diskBlock = table[fileBlock - 12];
                    bcacheRelease(ib);
                }
            }
        }

        if (diskBlock == 0) {
            memset(buffer + totalRead, 0, chunk);
        } else {
            struct Buffer *b = bcacheGet(ext2Drive, diskBlock);
            if (!b) {
                break;
            }
            memcpy(buffer + totalRead, b->data + blockOffset, chunk);
            bcacheRelease(b);
        }

        totalRead += chunk;
    }

    return (int)totalRead;
}

int ext2Write(struct VfsNode *node, uint32_t offset, uint8_t *buffer, size_t size) {
    if (!node || !buffer || size == 0)
        return 0;

    uint32_t inodeNum = (uint32_t)node->deviceData;
    struct Ext2Inode inode;
    if (ext2ReadInode(inodeNum, &inode) != 0)
        return -1;

    size_t totalWritten = 0;
    while (totalWritten < size) {
        uint32_t currentOffset = offset + totalWritten;
        uint32_t fileBlock = currentOffset / blockSize;
        uint32_t blockOffset = currentOffset % blockSize;
        size_t chunk = blockSize - blockOffset;
        if (chunk > size - totalWritten)
            chunk = size - totalWritten;

        if (fileBlock >= 12)
            break;

        if (inode.blockPointers[fileBlock] == 0) {
            uint32_t newBlock = ext2AllocBlock();
            if (newBlock == 0)
                break;
            inode.blockPointers[fileBlock] = newBlock;
            inode.blocks += (blockSize / 512);
        }

        uint32_t diskBlock = inode.blockPointers[fileBlock];
        struct Buffer *b = bcacheGet(ext2Drive, diskBlock);
        if (!b)
            break;

        memcpy(b->data + blockOffset, buffer + totalWritten, chunk);
        bcacheWrite(b);
        bcacheRelease(b);

        totalWritten += chunk;
    }

    if (offset + totalWritten > inode.size) {
        inode.size = offset + totalWritten;
        node->size = inode.size;
    }

    ext2WriteInode(inodeNum, &inode);
    return (int)totalWritten;
}

struct VfsNode *ext2Create(struct VfsNode *parentDir, const char *name, uint32_t mode) {
    if (!parentDir || !name || name[0] == '\0')
        return NULL;

    size_t nameLen = strlen(name);
    if (nameLen > 63)
        return NULL;

    for (struct VfsNode *c = parentDir->firstChild; c != NULL; c = c->nextSibling) {
        if (strcmp(c->name, name) == 0)
            return c;
    }

    uint32_t parentInodeNum = (uint32_t)parentDir->deviceData;
    struct Ext2Inode parentInode;
    if (ext2ReadInode(parentInodeNum, &parentInode) != 0)
        return NULL;

    uint32_t newInodeNum = ext2AllocInode();
    if (newInodeNum == 0)
        return NULL;

    struct Ext2Inode newInode;
    memset(&newInode, 0, sizeof(newInode));
    newInode.mode = (uint16_t)mode;
    newInode.size = 0;
    newInode.linksCount = 1;
    newInode.blocks = 0;

    if (mode & EXT2_S_IFDIR) {

        uint32_t dirBlock = ext2AllocBlock();
        if (dirBlock == 0)
            return NULL;

        newInode.blockPointers[0] = dirBlock;
        newInode.size = blockSize;
        newInode.blocks = (blockSize / 512);
        newInode.linksCount = 2;

        uint8_t dirData[1024];
        memset(dirData, 0, sizeof(dirData));

        struct Ext2DirEntry *dot = (struct Ext2DirEntry *)dirData;
        dot->inode = newInodeNum;
        dot->recLen = 12;
        dot->nameLen = 1;
        dot->fileType = EXT2_FT_DIR;
        dot->name[0] = '.';

        struct Ext2DirEntry *dotdot = (struct Ext2DirEntry *)(dirData + 12);
        dotdot->inode = parentInodeNum;
        dotdot->recLen = (uint16_t)(blockSize - 12);
        dotdot->nameLen = 2;
        dotdot->fileType = EXT2_FT_DIR;
        dotdot->name[0] = '.';
        dotdot->name[1] = '.';

        bcacheWriteBlock(ext2Drive, dirBlock, dirData);

        parentInode.linksCount++;
        ext2WriteInode(parentInodeNum, &parentInode);
    }

    ext2WriteInode(newInodeNum, &newInode);

    uint8_t fileType = (mode & EXT2_S_IFDIR) ? EXT2_FT_DIR : EXT2_FT_REG_FILE;
    uint16_t neededLen = (uint16_t)((8 + nameLen + 3) & ~3);

    uint32_t pDirBlock = parentInode.blockPointers[0];
    struct Buffer *pb = bcacheGet(ext2Drive, pDirBlock);
    if (!pb)
        return NULL;

    uint32_t offset = 0;
    struct Ext2DirEntry *lastEntry = NULL;

    while (offset < blockSize) {
        struct Ext2DirEntry *curr = (struct Ext2DirEntry *)(pb->data + offset);
        if (curr->recLen == 0) {
            break;
        }
        lastEntry = curr;
        if (offset + curr->recLen >= blockSize) {
            break;
        }
        offset += curr->recLen;
    }

    if (!lastEntry) {
        bcacheRelease(pb);
        return NULL;
    }

    uint16_t actualLastLen = (uint16_t)((8 + lastEntry->nameLen + 3) & ~3);
    if (lastEntry->recLen < actualLastLen + neededLen) {
        kprintf("EXT2: Directory block full\n");
        bcacheRelease(pb);
        return NULL;
    }

    uint16_t remainingRecLen = lastEntry->recLen - actualLastLen;
    lastEntry->recLen = actualLastLen;

    struct Ext2DirEntry *newEntry = (struct Ext2DirEntry *)((uint8_t *)lastEntry + actualLastLen);
    newEntry->inode = newInodeNum;
    newEntry->recLen = remainingRecLen;
    newEntry->nameLen = (uint8_t)nameLen;
    newEntry->fileType = fileType;
    memcpy(newEntry->name, name, nameLen);

    bcacheWrite(pb);
    bcacheRelease(pb);

    struct VfsNode *child = (struct VfsNode *)kMalloc(sizeof(struct VfsNode));
    if (!child) {
        return NULL;
    }
    memset(child, 0, sizeof(struct VfsNode));
    strncpy(child->name, name, sizeof(child->name) - 1);
    child->size = newInode.size;
    child->type = (mode & EXT2_S_IFDIR) ? DIR : REGULAR;
    child->read = ext2Read;
    child->write = ext2Write;
    child->deviceData = (void *)newInodeNum;
    child->parent = parentDir;

    struct VfsNode *lastChild = parentDir->firstChild;
    while (lastChild && lastChild->nextSibling)
        lastChild = lastChild->nextSibling;

    if (!lastChild) {
        parentDir->firstChild = child;
    } else {
        lastChild->nextSibling = child;
    }
    parentDir->numChild++;

    return child;
}

static void ext2PopulateDirectory(struct VfsNode *dirNode, uint32_t dirInodeNum) {
    struct Ext2Inode dirInode;
    if (ext2ReadInode(dirInodeNum, &dirInode) != 0)
        return;

    uint8_t *dirData = (uint8_t *)kMalloc(dirInode.size);
    if (!dirData)
        return;

    struct VfsNode tempNode;
    memset(&tempNode, 0, sizeof(tempNode));
    tempNode.deviceData = (void *)dirInodeNum;
    ext2Read(&tempNode, 0, dirData, dirInode.size);

    uint32_t offset = 0;
    struct VfsNode *lastChild = NULL;

    while (offset < dirInode.size) {
        struct Ext2DirEntry *entry = (struct Ext2DirEntry *)(dirData + offset);
        if (entry->recLen == 0)
            break;

        if (entry->inode != 0) {
            char name[64];
            size_t nlen = entry->nameLen;
            if (nlen > 63)
                nlen = 63;

            memcpy(name, entry->name, nlen);
            name[nlen] = '\0';

            if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                struct Ext2Inode childInode;
                if (ext2ReadInode(entry->inode, &childInode) == 0) {
                    struct VfsNode *child = (struct VfsNode *)kMalloc(sizeof(struct VfsNode));
                    if (child) {
                        memset(child, 0, sizeof(struct VfsNode));
                        strncpy(child->name, name, sizeof(child->name) - 1);
                        child->size = childInode.size;
                        child->type = (childInode.mode & EXT2_S_IFDIR) ? DIR : REGULAR;
                        child->read = ext2Read;
                        child->write = ext2Write;
                        child->deviceData = (void *)entry->inode;
                        child->parent = dirNode;

                        if (!dirNode->firstChild) {
                            dirNode->firstChild = child;
                        } else if (lastChild) {
                            lastChild->nextSibling = child;
                        }
                        lastChild = child;
                        dirNode->numChild++;

                        if (child->type == DIR) {
                            ext2PopulateDirectory(child, entry->inode);
                        }
                    }
                }
            }
        }

        offset += entry->recLen;
    }

    kFree(dirData);
}

int ext2Mount(uint8_t drive) {
    ext2Drive = drive;

    if (bcacheReadBlock(drive, 1, &sb) != 0) {
        kprintf("EXT2: Failed to read superblock from drive %d\n", drive);
        return -1;
    }

    if (sb.magic != EXT2_MAGIC) {
        kprintf("EXT2: Invalid magic 0x%x (expected 0x%x)\n", sb.magic, EXT2_MAGIC);
        return -1;
    }

    blockSize = 1024 << sb.logBlockSize;
    numGroups = (sb.blocksCount + sb.blocksPerGroup - 1) / sb.blocksPerGroup;

    uint32_t descTableBlock = (blockSize == 1024) ? 2 : 1;
    size_t descTableBytes = numGroups * sizeof(struct Ext2GroupDesc);

    groupDescs = (struct Ext2GroupDesc *)kMalloc(descTableBytes);
    if (!groupDescs) {
        kprintf("EXT2: Failed to allocate memory for group descriptors\n");
        return -1;
    }

    struct Buffer *gb = bcacheGet(drive, descTableBlock);
    if (!gb) {
        kprintf("EXT2: Failed to read group descriptor table block %d\n", descTableBlock);
        kFree(groupDescs);
        groupDescs = NULL;
        return -1;
    }
    memcpy(groupDescs, gb->data, descTableBytes);
    bcacheRelease(gb);

    memset(&ext2RootDir, 0, sizeof(ext2RootDir));
    strcpy(ext2RootDir.name, "/");
    ext2RootDir.type = DIR;
    ext2RootDir.parent = NULL;
    ext2RootDir.deviceData = (void *)2;

    vfsRoot = &ext2RootDir;

    ext2PopulateDirectory(&ext2RootDir, 2);

    // kprintf("EXT2: Mounted as root filesystem (/)\n");
    return 0;
}
