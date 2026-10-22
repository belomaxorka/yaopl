/* Host harness: ziso_init() from modules/isofs/zso.c and read_raw_data() from
 * modules/isofs/isofs.c, fed a ZSO header whose align byte is argv[2]. */
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

int main(int argc, char **argv)
{
    ZISO_header h = {0x4F53495A, 24, 2048 * 100, 2048, 1, 0, {0, 0}};
    u8 buf[64];
    int r;

    h.align = (u8)atoi(argv[2]);
    MountPoint.fd = _open(argv[1], _O_RDONLY | _O_BINARY);
#ifdef INIT_RETURNS_INT
    if ((r = ziso_init(&h, 0)) != 0) {
        printf("align %u: header rejected (%d)\n", h.align, r);
        return 0;
    }
#else
    ziso_init(&h, 0);
#endif
    printf("align %u: header accepted, reading index...\n", h.align);
    fflush(stdout);
    read_raw_data(buf, 4, 24, ziso_align);
    printf("read ok\n");
    return 0;
}
