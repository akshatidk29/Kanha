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
typedef struct VfsNode* (*VfsFindDirFunc)(struct VfsNode *dir, const char *name);
typedef struct VfsNode* (*VfsReadDirFunc)(struct VfsNode *dir, uint32_t idx);

struct VfsNode {

    char name[32];
    uint32_t size;
    enum Type type;

    VfsReadFunc    read;
    VfsFindDirFunc findDir;
    VfsReadDirFunc readDir;

    void *deviceData;
};


struct VfsNode* vfsOpen(const char* path);

int vfsRead(struct VfsNode* node,  uint32_t offset, uint8_t* buffer, size_t size);

struct VfsNode* vfsReadDir(struct VfsNode* dir, uint32_t idx);
struct VfsNode* vfsFindDir(struct VfsNode* dir, const char* name);

#endif