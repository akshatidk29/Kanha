#include <stddef.h>
#include <stdint.h>

#include "fs/vfs.h"
#include "lib/string.h"
#include "lib/kprintf.h"

struct VfsNode *vfsRoot = NULL;

struct VfsNode* vfsOpen(const char* path){
    
    if(!path || !vfsRoot)
        return NULL;

    if(strcmp(path, "/") == 0)
        return vfsRoot;

    if(path[0] == '/')
        path += 1;
    
    char token[32];
    struct VfsNode *currentNode = vfsRoot;

    while (*path != '\0'){

        size_t len = 0;

        while(*path != '/' && *path != '\0' && len < 31)
            token[len++] = *path++;
        
        token[len] = '\0';

        while(*path == '/')
            path++;

        currentNode = vfsFindDir(currentNode, token);
        if(!currentNode)
            return NULL;

        if(*path != '\0' && currentNode->type != DIR)
            return NULL;
    }

    return currentNode;
}

int vfsRead(struct VfsNode* node,  uint32_t offset, uint8_t* buffer, size_t size){

    if(!node || !node->read)
        return -1;

    return node->read(node, offset, buffer, size);
}

struct VfsNode* vfsReadDir(struct VfsNode* dir, uint32_t idx){

    if(!dir || dir->type != DIR || !dir->readDir)
        return NULL;

    return dir->readDir(dir, idx);
}

struct VfsNode* vfsFindDir(struct VfsNode* dir, const char* name){
    
    if (!dir || dir->type != DIR || !dir->findDir)
        return NULL;

    return dir->findDir(dir, name);
}
