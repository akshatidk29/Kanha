#ifndef KANHA_VFS_H
#define KANHA_VFS_H

#include <stddef.h>
#include <stdint.h>

enum Type{
    REGULAR,
    DIR,
};

struct VfsNode;
extern struct VfsNode *vfsRoot;

typedef int (*VfsReadFunc)(struct VfsNode *node, uint32_t offset, uint8_t *buffer, size_t size);
typedef int (*VfsWriteFunc)(struct VfsNode *node, uint32_t offset, uint8_t *buffer, size_t size);

struct VfsNode {

    char name[32];
    uint32_t size;
    enum Type type;

    struct VfsNode *parent;

    uint32_t numChild;
    struct VfsNode *firstChild;
    struct VfsNode *nextSibling;

    VfsReadFunc    read;
    VfsWriteFunc   write;

    void *deviceData;
};


struct VfsNode* vfsOpen(const char* path);
struct VfsNode* vfsCreate(const char* path, uint32_t mode);

int vfsRead(struct VfsNode* node,  uint32_t offset, uint8_t* buffer, size_t size);
int vfsWrite(struct VfsNode *node, uint32_t offset, uint8_t *buffer, size_t size);

struct VfsNode* vfsReadDir(struct VfsNode* dir, uint32_t idx);
struct VfsNode* vfsFindDir(struct VfsNode* dir, const char* name);

#endif