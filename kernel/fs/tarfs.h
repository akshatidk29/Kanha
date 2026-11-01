#ifndef KANHA_TARFS_H
#define KANHA_TARFS_H

#include <stddef.h>
#include <stdint.h>
#include "fs/vfs.h"

#define TAR_MAGIC "ustar"
#define TAR_MAGIC_LEN 5

struct TarHeader {
    char name[100];
    char unused1[24];
    char size[12];
    char unused2[20];
    char typeflag;
    char unused3[355];
};


int tarfsInitialize(void *archiveAddress, size_t archiveSize);

#endif
