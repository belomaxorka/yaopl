/* Host harness for the in-game ATA driver, modules/iopcore/cdvdman/atad.c
 * ata_device_sector_io_internal(): the loop that splits a transfer into ATA DMA commands.
 * Arguments: 48 or 28 (LBA mode) and the number of 512-byte sectors to transfer.
 * The ATA command is a stub that records its sector count; the loop is stopped after
 * 1000 commands, which no real request needs. */
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define ATA_C_READ_DMA      0xc8
#define ATA_C_WRITE_DMA     0xca
#define ATA_C_READ_DMA_EXT  0x25
#define ATA_C_WRITE_DMA_EXT 0x35
#define ATA_RES_ERR_ICRC    -510
#define SPD_R_IF_CTRL       0x64
#define SPD_IF_DMA_ENABLE   0x04
#define USE_SPD_REGS
#define SPD_REG16(r) spd_regs[(r)]
#define WAITIOSEMA(x)
#define SIGNALIOSEMA(x)

static u16 spd_regs[0x100];
static char lba_48bit;
static int ata_gamestar_workaround;
static int ata_io_sema;

#define MAX_COMMANDS 1000
static int commands;
static u64 sectors_done;
static u32 first_count, last_count;

static void ata_set_dir(int dir) { (void)dir; }

static int sceAtaExecCmd(void *buf, u32 blkcount, u16 feature, u16 nsector, u16 sector, u16 lcyl, u16 hcyl, u16 select, u16 command)
{
    if (commands == MAX_COMMANDS) {
        printf("ENDLESS LOOP: %d commands, %llu sectors transferred, last blkcount %u\n", commands, sectors_done, last_count);
        exit(0);
    }
    if (commands++ == 0)
        first_count = blkcount;
    last_count = blkcount;
    sectors_done += blkcount;
    return 0;
}

static int sceAtaWaitResult(void) { return 0; }

#include "atad_extracted.c"

int main(int argc, char **argv)
{
    u32 nsectors;
    int res;

    lba_48bit = atoi(argv[1]) == 48;
    nsectors = (u32)strtoul(argv[2], NULL, 0);
    res = ata_device_sector_io_internal(0, NULL, 0, nsectors, 0);
    printf("%u sectors: %d commands (first %u, last %u sectors), %llu transferred, res %d\n", nsectors, commands, first_count, last_count, sectors_done, res);
    return 0;
}
