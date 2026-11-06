#include <stddef.h>
#include <stdint.h>

#include "fs/tarfs.h"
#include "lib/string.h"
#include "lib/kprintf.h"
#include "mm/heap.h"

#define MAX_TAR_FILES 32

static struct VfsNode tarRoot;
static struct VfsNode tarBinDir;

static size_t tarFileCount = 0;

static int tarfsRead(struct VfsNode *node, uint32_t offset, uint8_t *buffer, size_t size) {
    if (!node || !buffer || offset >= node->size) 
        return 0;
    
    size_t toRead = (size > node->size - offset) ? (node->size - offset) : size;
    memcpy(buffer, (uint8_t *)node->deviceData + offset, toRead);
    return (int)toRead;
}

static int tarfsWrite(struct VfsNode *node, uint32_t offset, uint8_t *buffer, size_t size) {
    if (!node || !buffer || offset >= node->size) 
        return 0;
    
    size_t toWrite = (size > node->size - offset) ? (node->size - offset) : size;
    memcpy((uint8_t *)node->deviceData + offset, buffer, toWrite);
    return (int)toWrite;
}

static uint32_t parseOctal(const char *str, size_t len) {
    uint32_t val = 0;
    for (size_t i = 0; i < len && str[i] >= '0' && str[i] <= '7'; i++) {
        val = (val << 3) | (uint32_t)(str[i] - '0');
    }
    return val;
}

int tarfsInitialize(void *archiveAddress, size_t archiveSize) {
    tarFileCount = 0;

    // Initialize root directory 
    memset(&tarRoot, 0, sizeof(tarRoot));
    strcpy(tarRoot.name, "/");
    tarRoot.type = DIR;

    // Initialize "/bin" directory 
    memset(&tarBinDir, 0, sizeof(tarBinDir));
    strcpy(tarBinDir.name, "bin");
    tarBinDir.type = DIR;

    vfsRoot = &tarRoot;

    // Initialize root children list
    tarRoot.firstChild = &tarBinDir;
    tarRoot.nextSibling = NULL;
    tarRoot.parent = NULL;
    tarRoot.numChild = 1;

    tarBinDir.firstChild = NULL;
    tarBinDir.nextSibling = NULL;
    tarBinDir.parent = &tarRoot;
    tarBinDir.numChild = 0;

    uint8_t *ptr = (uint8_t *)archiveAddress;
    uint8_t *end = ptr + archiveSize;

    while (ptr + 512 <= end) {

        struct TarHeader *header = (struct TarHeader *)ptr;

        // End of tar
        if (header->name[0] == '\0') {
            break;                                              
        }

        uint32_t size = parseOctal(header->size, 12);

        if (header->typeflag != '5' && tarFileCount < MAX_TAR_FILES) {

            tarFileCount++;
            tarBinDir.numChild++;

            struct VfsNode *node = (struct VfsNode*)kMalloc(sizeof(struct VfsNode));
            if(!node)
                continue;
            memset(node, 0, sizeof(struct VfsNode));

            // Store file name
            const char *name = header->name;
            for (const char *p = header->name; *p != '\0'; p++) {
                if (*p == '/') {
                    name = p + 1;
                }
            }
            
            strncpy(node->name, name, sizeof(node->name) - 1);
            node->type = REGULAR;
            node->size = size;
            node->read = tarfsRead;
            node->write = tarfsWrite;
            node->deviceData = ptr + 512;
            node->firstChild = NULL;
            node->nextSibling = NULL;
            node->parent = &tarBinDir;
            node->numChild = 0;

            struct VfsNode *tarBinLastChild = tarBinDir.firstChild;

            while (tarBinLastChild && tarBinLastChild->nextSibling)
                tarBinLastChild = tarBinLastChild->nextSibling;
            
            if(!tarBinLastChild){
                tarBinDir.firstChild = node;
                tarBinLastChild = node;
            }else{
                tarBinLastChild->nextSibling = node;
                tarBinLastChild = node;
            }
        }

        ptr += 512 + ((size + 511) / 512) * 512;
    }

    kprintf("TarFS Loaded %d files\n", tarFileCount);

    return 0;
}
