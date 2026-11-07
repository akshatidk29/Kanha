#ifndef KANHA_ATA_H
#define KANHA_ATA_H

#include <stdint.h>
#include <stddef.h>

#define ATA_SECTOR_SIZE 512

#define ATA_DRIVE_MASTER 0
#define ATA_DRIVE_SLAVE  1

struct AtaDevice {
    uint8_t present;
    uint8_t driveIndex;    // 0: Master, 1: Slave
    uint32_t sectorCount;  // Total addressable LBA28 sectors
    char model[41];        // Model name string
};

void ataInitialize(void);

const struct AtaDevice *ataGetDevice(uint8_t drive);

int ataReadSector(uint8_t drive, uint32_t lba, uint8_t *buffer);
int ataWriteSector(uint8_t drive, uint32_t lba, const uint8_t *buffer);

int ataRead(uint8_t drive, uint32_t lba, uint32_t count, uint8_t *buffer);
int ataWrite(uint8_t drive, uint32_t lba, uint32_t count, const uint8_t *buffer);

#endif

