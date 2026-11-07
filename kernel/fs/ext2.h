#ifndef KANHA_EXT2_H
#define KANHA_EXT2_H

#include <stdint.h>
#include <stddef.h>
#include "fs/vfs.h"

#define EXT2_MAGIC 0xEF53

// File Types
#define EXT2_S_IFREG  0x8000
#define EXT2_S_IFDIR  0x4000

// Directory Entry File Types
#define EXT2_FT_UNKNOWN  0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR      2

// Superblock
struct Ext2Superblock {
    uint32_t inodesCount;            // Inodes count
    uint32_t blocksCount;            // Blocks count
    uint32_t reservedBlocksCount;    // Reserved blocks count
    uint32_t freeBlocksCount;        // Free blocks count
    uint32_t freeInodesCount;        // Free inodes count
    uint32_t firstDataBlock;         // First data block
    uint32_t logBlockSize;           // Block size
    uint32_t logFragSize;            // Fragment size
    uint32_t blocksPerGroup;         // Blocks per block group
    uint32_t fragsPerGroup;          // Fragments per group
    uint32_t inodesPerGroup;         // Inodes per group
    uint8_t  reserved1[12];          // Unused
    uint16_t magic;                  // Magic signature
    uint16_t state;                  // Filesystem state
    uint16_t errors;                 // Error behavior
    uint16_t minorRevLevel;          // Minor revision
    uint32_t lastCheck;              // Last check timestamp
    uint32_t checkInterval;          // Check interval
    uint32_t creatorOs;              // Creator OS ID
    uint32_t revLevel;               // Revision level
    uint16_t defResUid;              // Default UID for reserved blocks
    uint16_t defResGid;              // Default GID for reserved blocks
    uint32_t firstIno;               // First non-reserved inode
    uint16_t inodeSize;              // Inode structure size
    uint16_t blockGroupNr;           // Block group number of this superblock
    uint32_t featureCompat;          // Compatible features
    uint32_t featureIncompat;        // Incompatible features
    uint32_t featureRoCompat;        // Read-only compatible features
    uint8_t  uuid[16];               // Volume UUID
    char     volumeName[16];         // Volume name
    uint8_t  reserved2[888];         // Unused extended attributes, journal, and hash padding
} __attribute__((packed));

// Block Group Descriptor
struct Ext2GroupDesc {
    uint32_t blockBitmap;     // Block address of block usage bitmap
    uint32_t inodeBitmap;     // Block address of inode usage bitmap
    uint32_t inodeTable;      // Starting block address of inode table
    uint16_t freeBlocksCount; // Number of free blocks in this group
    uint16_t freeInodesCount; // Number of free inodes in this group
    uint16_t usedDirsCount;   // Number of directories in this group
    uint16_t pad;             // Alignment padding
    uint8_t  reserved[12];    // Reserved for future extensions
} __attribute__((packed));

// Inode
struct Ext2Inode {
    uint16_t mode;               // File type and permissions
    uint16_t uid;                // Owner user ID
    uint32_t size;               // File size in bytes
    uint32_t atime;              // Access time
    uint32_t ctime;              // Creation time
    uint32_t mtime;              // Modification time
    uint32_t dtime;              // Deletion time
    uint16_t gid;                // Group ID
    uint16_t linksCount;         // Number of hard links
    uint32_t blocks;             // Number of 512-byte disk sectors used
    uint32_t flags;              // File flags
    uint32_t osd1;               // OS-dependent field
    uint32_t blockPointers[15];  // 0-11 direct, 12 single indirect, 13 double, 14 triple
    uint8_t  reserved[28];       // Unused ACLs and file fragments
} __attribute__((packed));

// Directory Entry Header
struct Ext2DirEntry {
    uint32_t inode;              // Inode number
    uint16_t recLen;             // Length of this directory record in bytes
    uint8_t  nameLen;            // Length of the filename
    uint8_t  fileType;           // File type
    char     name[];             // Filename characters
} __attribute__((packed));



int ext2Mount(uint8_t drive);

// Reads an inode
int ext2ReadInode(uint32_t inodeNum, struct Ext2Inode *outInode);

// Writes an inode
int ext2WriteInode(uint32_t inodeNum, const struct Ext2Inode *inInode);

// Allocates a free block
uint32_t ext2AllocBlock(void);

// Allocates a free inode
uint32_t ext2AllocInode(void);

// Creates a new file or directory
struct VfsNode *ext2Create(struct VfsNode *parentDir, const char *name, uint32_t mode);

// Reads bytes from a file
int ext2Read(struct VfsNode *node, uint32_t offset, uint8_t *buffer, size_t size);

// Writes bytes to a file
int ext2Write(struct VfsNode *node, uint32_t offset, uint8_t *buffer, size_t size);

#endif
