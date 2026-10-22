/* Host harness for modules/isofs/zso.c ziso_init() and ziso_read_sector(), with
 * read_raw_data() and longLseek() from modules/isofs/isofs.c and the LZ4 decoder
 * of the tree. Two modes:
 *   zso-t <file> <align>          ziso_init() on a header whose align byte is <align>
 *   zso-t read <zso> <lsn> <count> read <count> sectors into a buffer of exactly
 *                                 count * 2048 bytes and check them against the
 *                                 pattern make_inputs.py wrote (sector n holds
 *                                 (n * 7 + 1) & 0xFF) */
#include <fcntl.h>
#include <io.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
#define lseek                _lseek
#define read                 _read
#define ZISO_IDX_MAX_ENTRIES 257

#include "lz4.c"
#ifndef MIN
#define MIN(x, y) ((x < y) ? x : y)
#endif

typedef struct
{
    u32 magic;
    u32 header_size;
    u64 total_bytes;
    u32 block_size;
    u8 ver;
    u8 align;
    u8 rsv_06[2];
} ZISO_header;

u32 *ziso_idx_cache = NULL;
int ziso_idx_start_block = -1;
u32 ziso_align;
u32 ziso_total_block;
u8 *ziso_tmp_buf = NULL;
static struct
{
    int fd;
} MountPoint;

void *ziso_alloc(u32 size) { return malloc(size); }

#include "zso_extracted.c"

static int init(ZISO_header *h, u32 first_block)
{
#ifdef INIT_RETURNS_INT
    return ziso_init(h, first_block);
#else
    ziso_init(h, first_block);
    return 0;
#endif
}

static int read_test(const char *file, u32 lsn, unsigned int count)
{
    ZISO_header h;
    u32 first_block, i, k;
    u8 *buf;
    int r, bad = -1;

    MountPoint.fd = _open(file, _O_RDONLY | _O_BINARY);
    if (MountPoint.fd < 0 || _read(MountPoint.fd, &h, sizeof(h)) != sizeof(h) || _read(MountPoint.fd, &first_block, 4) != 4) {
        printf("cannot read %s\n", file);
        return 2;
    }
    if ((r = init(&h, first_block)) != 0) {
        printf("header rejected (%d): block size %u, align %u\n", r, h.block_size, h.align);
        return 0;
    }
    printf("header accepted: %u sectors, block size %u, align %u\n", ziso_total_block, h.block_size, h.align);
    fflush(stdout);

    buf = malloc(count * 2048); /* exactly what the caller provides: ASan sees writes outside it */
    memset(buf, 0, count * 2048);
    r = ziso_read_sector(buf, lsn, count);
    for (i = 0; i < (u32)r && bad < 0; i++) {
        u8 want = (u8)(((lsn + i) * 7 + 1) & 0xFF);
        for (k = 0; k < 2048; k++)
            if (buf[i * 2048 + k] != want) {
                bad = lsn + i;
                break;
            }
    }
    if (r != (int)count)
        printf("read %d of %u sectors from %u: read error\n", r, count, lsn);
    else if (bad >= 0)
        printf("read %u sectors from %u: WRONG DATA from sector %d\n", count, lsn, bad);
    else
        printf("read %u sectors from %u: data ok\n", count, lsn);
    free(buf);
    return 0;
}

int main(int argc, char **argv)
{
    ZISO_header h = {0x4F53495A, 24, 2048 * 100, 2048, 1, 0, {0, 0}};
    u8 buf[64];
    int r;

    if (argc == 5 && strcmp(argv[1], "read") == 0)
        return read_test(argv[2], (u32)strtoul(argv[3], NULL, 10), (unsigned int)strtoul(argv[4], NULL, 10));

    h.align = (u8)atoi(argv[2]);
    MountPoint.fd = _open(argv[1], _O_RDONLY | _O_BINARY);
    if ((r = init(&h, 0)) != 0) {
        printf("align %u: header rejected (%d)\n", h.align, r);
        return 0;
    }
    printf("align %u: header accepted, reading index...\n", h.align);
    fflush(stdout);
    read_raw_data(buf, 4, 24, ziso_align);
    printf("read ok\n");
    return 0;
}
