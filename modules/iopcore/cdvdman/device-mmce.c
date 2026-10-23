/*
  Copyright 2024-2025, qnox32 and BBsan (ps2-mmce/Open-PS2-Loader)
  Licenced under Academic Free License version 3.0
  Review Open PS2 Loader README & LICENSE files for further details.

  CDVDMAN back-end for MMCE memory-card emulators (SD2PSX, MemCard PRO 2, PSXMemCard Gen2): the ISO is a
  file on the card, opened by the menu and kept open by the card across the IOP reset, and it is read in
  game through mmcedrv.irx (ps2-mmce/mmceman) over SIO2.
*/

#include "internal.h"

#include "device.h"
#include "ioplib_util.h"
#include "mmcedrv_config.h"

extern struct cdvdman_settings_mmce cdvdman_settings;

// Entries 4 to 6 of the export table of mmcedrv.irx (ps2-mmce/mmceman, mmceman/src/exports.tab). The
// sector number and the count travel as 24-bit values: an image can have at most 0x1000000 sectors (32 GiB).
static s64 (*mmcedrv_get_size)(int fd) = NULL;
static int (*mmcedrv_read_sector)(int fd, u32 sector, u32 count, void *buffer) = NULL;
static void (*mmcedrv_config_set)(int setting, int value) = NULL;

#define MMCE_MAX_SECTORS 0x1000000
#define MMCE_READY_TRIES 100 // x 100 ms: how long DeviceFSInit() waits for the card to answer
#define MMCE_READ_TRIES  3

static int mmce_io_sema;
static int mmce_ready = 0; // the card answered mmcedrv_get_size() for the ISO

//
// cdvdman "Device" functions
//

void DeviceInit(void)
{
    iop_sema_t smp;

    DPRINTF("%s\n", __func__);

    smp.initial = 1;
    smp.max = 1;
    smp.option = 0;
    smp.attr = SA_THPRI;
    mmce_io_sema = CreateSema(&smp);
}

void DeviceDeinit(void)
{
    DPRINTF("%s\n", __func__);
}

int DeviceReady(void)
{
    return SCECdComplete;
}

void DeviceStop(void)
{
    DPRINTF("%s\n", __func__);
}

// mmcedrv.irx is loaded by the EE core after cdvdman, on every IOP reset. Without it every read fails
// instead of calling garbage pointers.
static int mmceResolveDriver(void)
{
    modinfo_t info;

    if (mmcedrv_read_sector != NULL)
        return 1;

    if (!getModInfo("mmcedrv\0", &info)) {
        DPRINTF("%s: mmcedrv is not loaded\n", __func__);
        return 0;
    }

    mmcedrv_get_size = (void *)info.exports[4];
    mmcedrv_read_sector = (void *)info.exports[5];
    mmcedrv_config_set = (void *)info.exports[6];

    // SIO2 port of the card: 2 = memory-card slot 1, 3 = slot 2. The ACK wait cycles and the alarms keep
    // the driver's defaults.
    if (cdvdman_settings.port == 2 || cdvdman_settings.port == 3)
        mmcedrv_config_set(MMCEDRV_SETTING_PORT, cdvdman_settings.port);

    return 1;
}

// The size is the ISO's once the card is up after the IOP reset, -1 (a 64-bit value) on a timeout or a
// bad reply.
static int mmceProbe(void)
{
    s64 size;

    if (!mmceResolveDriver())
        return 0;

    size = mmcedrv_get_size(cdvdman_settings.iso_fd);
    if (size <= 0)
        return 0;

    DPRINTF("%s: fd %d, %u KiB on port %d\n", __func__, cdvdman_settings.iso_fd, (unsigned int)(size >> 10), cdvdman_settings.port);
    mmce_ready = 1;
    return 1;
}

void DeviceFSInit(void)
{
    int i;

    DPRINTF("Waiting for device...\n");
    for (i = 0; i < MMCE_READY_TRIES; i++) {
        if (mmceProbe())
            break;
        DelayThread(100 * 1000); // 100 ms
    }
    DPRINTF("Waiting for device...%s\n", mmce_ready ? "done!" : "timed out");
}

void DeviceLock(void)
{
    DPRINTF("%s\n", __func__);

    WaitSema(mmce_io_sema);
}

void DeviceUnmount(void)
{
    DPRINTF("%s\n", __func__);
}

// A read that fails on the bus is retried; one that succeeds on a retry is a success.
static int mmceRead(u32 lsn, void *buffer, unsigned int sectors)
{
    int i, res;

    for (i = 0; i < MMCE_READ_TRIES; i++) {
        res = mmcedrv_read_sector(cdvdman_settings.iso_fd, lsn, sectors, buffer);
        if (res == (int)sectors)
            return 1;
        DPRINTF("%s: sector %u, count %u: got %d (try %d)\n", __func__, lsn, sectors, res, i + 1);
    }

    return 0;
}

int DeviceReadSectors(u64 lsn, void *buffer, unsigned int sectors)
{
    int rv = SCECdErNO;

    // DPRINTF("%s(%u, 0x%p, %u)\n", __func__, (unsigned int)lsn, buffer, sectors);

    if (lsn + sectors > MMCE_MAX_SECTORS)
        return SCECdErREAD;

    WaitSema(mmce_io_sema);
    if (!mmce_ready && !mmceProbe()) {
        SignalSema(mmce_io_sema);
        return SCECdErREAD;
    }

    if (((u32)buffer & 3) == 0) {
        if (!mmceRead((u32)lsn, buffer, sectors))
            rv = SCECdErREAD;
    } else {
        // The driver DMAs into the buffer, so unaligned buffers (e.g. from the ZSO reader) go through an
        // aligned bounce buffer, as the BDM back-end does.
        static u8 bounce[2048] __attribute__((aligned(64)));
        u8 *p = buffer;

        while (sectors > 0) {
            if (!mmceRead((u32)lsn, bounce, 1)) {
                rv = SCECdErREAD;
                break;
            }
            memcpy(p, bounce, 2048);
            p += 2048;
            lsn++;
            sectors--;
        }
    }
    SignalSema(mmce_io_sema);

    return rv;
}
