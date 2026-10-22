/* Host harness for the in-game cdvdman_read() (modules/iopcore/cdvdman/cdvdman.c), as built for
 * USB, MX4SIO and SMB (cdvdman_buf holds one sector):
 *   cdread <sector size> <sectors>
 * reads into a heap block of exactly <sectors> * <sector size> bytes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#define PHYSADDR(x)         (x)
#define CDVDMAN_BUF_SECTORS 1
#define SCECdStatRead       0x06
#define SCECdStatPause      0x0A
#define WaitSema(s)         ((void)0)
#define SignalSema(s)       ((void)0)

static struct
{
    int status;
} cdvdman_stat;
static int cdvdman_searchfilesema;
static u32 ReadPos;
/* the static buffer, with the memory that follows it in the module */
static struct
{
    u8 buf[CDVDMAN_BUF_SECTORS * 2048];
    u8 next[512];
} mem;
#define cdvdman_buf mem.buf

typedef struct
{
    u8 minute, second, sector, track;
} sceCdlLOCCD;
static void sceCdIntToPos(u32 i, sceCdlLOCCD *p) { p->minute = p->second = p->sector = p->track = 0; }
static int cdvdman_read_sectors(u32 lsn, unsigned int sectors, void *buf)
{
    memset(buf, 0x5A, sectors * 2048);
    return 0;
}

#include "cdread_extracted.c"

int main(int argc, char **argv)
{
    u16 size = (u16)atoi(argv[1]);
    u32 sectors = (u32)atoi(argv[2]);
    u8 *buf = malloc(size * sectors);

    cdvdman_read(16, sectors, size, buf);
    printf("read %u sectors of %u bytes into a %u-byte buffer\n", sectors, size, size * sectors);
    free(buf);
    return 0;
}
