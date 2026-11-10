#include <stddef.h>
#include <stdint.h>

#include "fs/vfs.h"
#include "lib/string.h"
#include "lib/kprintf.h"
#include "task/process.h"

struct VfsNode *vfsRoot = NULL;

int vfsResolvePath(const char *inPath, char *outPath, size_t maxLen) {
    if (!inPath || !outPath || maxLen == 0)
        return -1;

    char resolved[128];
    resolved[0] = '\0';

    if (inPath[0] == '/') {
        strcpy(resolved, "/");
        inPath++;
    } else {
        struct Process *proc = currentProcess();
        if (proc && proc->cwd[0] != '\0') {
            strncpy(resolved, proc->cwd, sizeof(resolved) - 1);
            resolved[sizeof(resolved) - 1] = '\0';
        } else {
            strcpy(resolved, "/");
        }
    }

    const char *p = inPath;
    while (*p != '\0') {
        while (*p == '/')
            p++;
        if (*p == '\0')
            break;

        char token[32];
        size_t tlen = 0;
        while (*p != '/' && *p != '\0' && tlen < sizeof(token) - 1)
            token[tlen++] = *p++;

        token[tlen] = '\0';

        if (strcmp(token, ".") == 0) {
            continue;
        } else if (strcmp(token, "..") == 0) {
            if (strcmp(resolved, "/") != 0) {
                char *lastSlash = NULL;
                for (char *s = resolved; *s != '\0'; s++) {
                    if (*s == '/')
                        lastSlash = s;
                }
                if (lastSlash == resolved) {
                    resolved[1] = '\0';
                } else if (lastSlash) {
                    *lastSlash = '\0';
                }
            }
        } else {
            size_t rlen = strlen(resolved);
            if (rlen == 1 && resolved[0] == '/') {
                if (rlen + tlen < sizeof(resolved)) {
                    strcat(resolved, token);
                }
            } else {
                if (rlen + 1 + tlen < sizeof(resolved)) {
                    resolved[rlen] = '/';
                    resolved[rlen + 1] = '\0';
                    strcat(resolved, token);
                }
            }
        }
    }

    if (resolved[0] == '\0') {
        strcpy(resolved, "/");
    }

    strncpy(outPath, resolved, maxLen - 1);
    outPath[maxLen - 1] = '\0';
    return 0;
}

struct VfsNode* vfsOpen(const char* path){
    
    if(!path || !vfsRoot)
        return NULL;

    char resolvedPath[128];
    if (vfsResolvePath(path, resolvedPath, sizeof(resolvedPath)) != 0)
        return NULL;

    if (strcmp(resolvedPath, "/") == 0)
        return vfsRoot;

    struct VfsNode *currentNode = vfsRoot;
    const char *p = resolvedPath;
    if (*p == '/')
        p++;
    
    char token[32];

    while (*p != '\0') {
        size_t len = 0;
        while (*p != '/' && *p != '\0' && len < sizeof(token) - 1)
            token[len++] = *p++;
        token[len] = '\0';

        currentNode = vfsFindDir(currentNode, token);
        if (!currentNode)
            return NULL;

        if (*p == '/')
            p++;

        if (*p != '\0' && currentNode->type != DIR)
            return NULL;
    }

    return currentNode;
}

struct VfsNode *ext2Create(struct VfsNode *parentDir, const char *name, uint32_t mode);

struct VfsNode* vfsCreate(const char* path, uint32_t mode){
    if(!path || !vfsRoot)
        return NULL;

    char resolvedPath[128];
    if (vfsResolvePath(path, resolvedPath, sizeof(resolvedPath)) != 0)
        return NULL;

    struct VfsNode *existing = vfsOpen(resolvedPath);
    if(existing)
        return existing;

    char parentPath[128];
    const char *lastSlash = NULL;
    for (const char *p = resolvedPath; *p != '\0'; p++) {
        if (*p == '/') 
            lastSlash = p;
    }

    const char *fileName = NULL;
    if (!lastSlash || lastSlash == resolvedPath) {
        strcpy(parentPath, "/");
        fileName = (lastSlash == resolvedPath) ? (lastSlash + 1) : resolvedPath;
    } else {
        size_t dlen = lastSlash - resolvedPath;
        if (dlen > sizeof(parentPath) - 1) 
            dlen = sizeof(parentPath) - 1;
        memcpy(parentPath, resolvedPath, dlen);
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
