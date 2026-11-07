#include <stddef.h>
#include <stdint.h>

#include "fs/vfs.h"
#include "lib/string.h"
#include "lib/kprintf.h"

struct VfsNode *vfsRoot = NULL;

struct VfsNode* vfsOpen(const char* path){
    
    if(!path || !vfsRoot)
        return NULL;

    struct VfsNode *currentNode = vfsRoot;
    
    while(*path == '/'){
        path++;
    }
    
    char token[32];

    while (*path != '\0'){

        size_t len = 0;

        while(*path != '/' && *path != '\0' && len < 31)
            token[len++] = *path++;
        
        while(*path == '/')
            path++;

        token[len] = '\0';

        currentNode = vfsFindDir(currentNode, token);
        if(!currentNode)
            return NULL;

        if(*path != '\0' && currentNode->type != DIR)
            return NULL;
    }

    return currentNode;
}

struct VfsNode *ext2Create(struct VfsNode *parentDir, const char *name, uint32_t mode);

struct VfsNode* vfsCreate(const char* path, uint32_t mode){
    if(!path || !vfsRoot)
        return NULL;

    struct VfsNode *existing = vfsOpen(path);
    if(existing)
        return existing;

    char parentPath[64];
    const char *lastSlash = NULL;
    for (const char *p = path; *p != '\0'; p++) {
        if (*p == '/') lastSlash = p;
    }

    const char *fileName = NULL;
    if (!lastSlash) {
        strcpy(parentPath, "/");
        fileName = path;
    } else if (lastSlash == path) {
        strcpy(parentPath, "/");
        fileName = lastSlash + 1;
    } else {
        size_t dlen = lastSlash - path;
        if (dlen > 63) dlen = 63;
        memcpy(parentPath, path, dlen);
        parentPath[dlen] = '\0';
        fileName = lastSlash + 1;
    }

    struct VfsNode *parentDir = vfsOpen(parentPath);
    if (!parentDir || parentDir->type != DIR)
        return NULL;

    return ext2Create(parentDir, fileName, mode);
}

int vfsRead(struct VfsNode* node,  uint32_t offset, uint8_t* buffer, size_t size){
    if(!node || !node->read)
        return -1;

    return node->read(node, offset, buffer, size);
}

int vfsWrite(struct VfsNode* node,  uint32_t offset, uint8_t* buffer, size_t size){
    if(!node || !node->write)
        return -1;

    return node->write(node, offset, buffer, size);
}


struct VfsNode* vfsReadDir(struct VfsNode* dir, uint32_t idx){

    if(!dir || dir->type != DIR)
        return NULL;

    uint32_t i = 0;
    for(struct VfsNode *c = dir->firstChild; c != NULL; c = c->nextSibling, i++){
        if(i == idx)
            return c;
    }
    return NULL;
}

struct VfsNode* vfsFindDir(struct VfsNode* dir, const char* name){
    
    if (!dir || dir->type != DIR)
        return NULL;

    for(struct VfsNode *c = dir->firstChild; c != NULL; c = c->nextSibling){
        if(strcmp(c->name, name) == 0)
            return c;
    }

    return NULL;
}
