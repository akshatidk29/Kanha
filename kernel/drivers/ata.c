#include "drivers/ata.h"
#include "hardware/io.h"
#include "hardware/pic.h"
#include "task/lock.h"
#include "lib/kprintf.h"
#include "lib/string.h"

#define ATA_PORT_DATA           0x1F0
#define ATA_PORT_ERROR_FEATURES 0x1F1
#define ATA_PORT_SECTOR_COUNT   0x1F2
#define ATA_PORT_LBA_LOW        0x1F3
#define ATA_PORT_LBA_MID        0x1F4
#define ATA_PORT_LBA_HIGH       0x1F5
#define ATA_PORT_DRIVE          0x1F6
#define ATA_PORT_COMMAND        0x1F7
#define ATA_PORT_STATUS         0x1F7
#define ATA_PORT_CONTROL        0x3F6
#define ATA_PORT_ALT_STATUS     0x3F6 

#define ATA_SR_ERR  0x01
#define ATA_SR_IDX  0x02
#define ATA_SR_CORR 0x04
#define ATA_SR_DRQ  0x08
#define ATA_SR_DSC  0x10
#define ATA_SR_DF   0x20
#define ATA_SR_DRDY 0x40
#define ATA_SR_BSY  0x80 

#define ATA_CMD_READ_PIO    0x20
#define ATA_CMD_WRITE_PIO   0x30
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_IDENTIFY    0xEC

static struct AtaDevice devices[2];
static struct SpinLock ataLock;

static void ataDelay(void) {
    inB(ATA_PORT_ALT_STATUS);
    inB(ATA_PORT_ALT_STATUS);
    inB(ATA_PORT_ALT_STATUS);
    inB(ATA_PORT_ALT_STATUS);
}

static int ataWaitBsy(void) {
    uint32_t timeout = 100000;
    while ((inB(ATA_PORT_STATUS) & ATA_SR_BSY) && --timeout) {
    }
    return (timeout > 0) ? 0 : -1;
}

static int ataWaitDrq(void) {
    uint32_t timeout = 100000;
    while (--timeout) {
        uint8_t status = inB(ATA_PORT_STATUS);
        if (status & (ATA_SR_ERR | ATA_SR_DF)) {
            return -1;
        }
        if (status & ATA_SR_DRQ) {
            return 0;
        }
    }
    return -1;
}

static int ataProbe(uint8_t drive) {
    devices[drive].present = 0;
    devices[drive].driveIndex = drive;
    devices[drive].sectorCount = 0;
    memset(devices[drive].model, 0, sizeof(devices[drive].model));

    uint8_t driveByte = (drive == 0) ? 0xA0 : 0xB0;
    outB(ATA_PORT_DRIVE, driveByte);
    ataDelay();

    uint8_t status = inB(ATA_PORT_STATUS);
    if (status == 0xFF) {
        return -1;
    }

    outB(ATA_PORT_SECTOR_COUNT, 0);
    outB(ATA_PORT_LBA_LOW, 0);
    outB(ATA_PORT_LBA_MID, 0);
    outB(ATA_PORT_LBA_HIGH, 0);

    outB(ATA_PORT_COMMAND, ATA_CMD_IDENTIFY);
    ataDelay();

    status = inB(ATA_PORT_STATUS);
    if (status == 0) {
        return -1;
    }

    if (ataWaitBsy() != 0) {
        return -1;
    }

    uint8_t mid = inB(ATA_PORT_LBA_MID);
    uint8_t high = inB(ATA_PORT_LBA_HIGH);
    if (mid != 0 || high != 0) {
        return -1;
    }

    if (ataWaitDrq() != 0) {
        return -1;
    }

    uint16_t idData[256];
    insW(ATA_PORT_DATA, idData, 256);

    devices[drive].present = 1;
    devices[drive].sectorCount = (uint32_t)idData[60] | ((uint32_t)idData[61] << 16);

    for (int i = 0; i < 20; i++) {
        uint16_t w = idData[27 + i];
        devices[drive].model[i * 2] = (char)(w >> 8);
        devices[drive].model[i * 2 + 1] = (char)(w & 0xFF);
    }
    devices[drive].model[40] = '\0';

    for (int i = 39; i >= 0 && devices[drive].model[i] == ' '; i--) {
        devices[drive].model[i] = '\0';
    }

    uint32_t sizeMb = (devices[drive].sectorCount * ATA_SECTOR_SIZE) / (1024 * 1024);
    // kprintf("ATA: %s drive detected: %s (%d MB, %d sectors)\n", (drive == 0) ? "Master" : "Slave", devices[drive].model, sizeMb, devices[drive].sectorCount);

    return 0;
}

void ataInitialize(void) {
    spinLockInit(&ataLock);

    // Mask ATA IRQs on PIC and disable ATA interrupts on primary bus
    picSetMask(14);
    picSetMask(15);
    outB(ATA_PORT_CONTROL, 0x02);

    // kprintf("Initializing Primary ATA bus\n");
    ataProbe(ATA_DRIVE_MASTER);
    ataProbe(ATA_DRIVE_SLAVE);
}

const struct AtaDevice *ataGetDevice(uint8_t drive) {
    if (drive > 1 || !devices[drive].present) {
        return NULL;
    }
    return &devices[drive];
}

int ataReadSector(uint8_t drive, uint32_t lba, uint8_t *buffer) {
    if (!buffer || drive > 1 || !devices[drive].present) {
        return -1;
    }

    spinLockAcquire(&ataLock);

    if (ataWaitBsy() != 0) {
        spinLockRelease(&ataLock);
        return -1;
    }

    uint8_t head = (drive == 0 ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F);
    outB(ATA_PORT_DRIVE, head);
    ataDelay();

    outB(ATA_PORT_SECTOR_COUNT, 1);
    outB(ATA_PORT_LBA_LOW, (uint8_t)(lba & 0xFF));
    outB(ATA_PORT_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outB(ATA_PORT_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));
    outB(ATA_PORT_COMMAND, ATA_CMD_READ_PIO);

    if (ataWaitBsy() != 0 || ataWaitDrq() != 0) {
        spinLockRelease(&ataLock);
        return -1;
    }

    insW(ATA_PORT_DATA, buffer, 256);

    spinLockRelease(&ataLock);
    return 0;
}

int ataWriteSector(uint8_t drive, uint32_t lba, const uint8_t *buffer) {
    if (!buffer || drive > 1 || !devices[drive].present) {
        return -1;
    }

    spinLockAcquire(&ataLock);

    if (ataWaitBsy() != 0) {
        spinLockRelease(&ataLock);
        return -1;
    }

    uint8_t head = (drive == 0 ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F);
    outB(ATA_PORT_DRIVE, head);
    ataDelay();

    outB(ATA_PORT_SECTOR_COUNT, 1);
    outB(ATA_PORT_LBA_LOW, (uint8_t)(lba & 0xFF));
    outB(ATA_PORT_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outB(ATA_PORT_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));
    outB(ATA_PORT_COMMAND, ATA_CMD_WRITE_PIO);

    if (ataWaitBsy() != 0 || ataWaitDrq() != 0) {
        spinLockRelease(&ataLock);
        return -1;
    }

    outsW(ATA_PORT_DATA, buffer, 256);

    outB(ATA_PORT_COMMAND, ATA_CMD_CACHE_FLUSH);
    if (ataWaitBsy() != 0) {
        spinLockRelease(&ataLock);
        return -1;
    }

    spinLockRelease(&ataLock);
    return 0;
}

int ataRead(uint8_t drive, uint32_t lba, uint32_t count, uint8_t *buffer) {
    for (uint32_t i = 0; i < count; i++) {
        if (ataReadSector(drive, lba + i, buffer + (i * ATA_SECTOR_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}

int ataWrite(uint8_t drive, uint32_t lba, uint32_t count, const uint8_t *buffer) {
    for (uint32_t i = 0; i < count; i++) {
        if (ataWriteSector(drive, lba + i, buffer + (i * ATA_SECTOR_SIZE)) != 0) {
            return -1;
        }
    }
    return 0;
}
