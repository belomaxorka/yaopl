/* Host harness for cdvdman_findfile() from modules/isofs/isofs.c: opens argv[1] on a fake
 * disc whose root holds SYSTEM.CNF;1 and a file with a 20-character name. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned int u32;
#define __attribute__(x)
#define DPRINTF(...)

typedef struct
{
    u32 lsn;
    u32 size;
    char name[16];
    u8 date[8];
} cd_file_t;
typedef struct
{
    u32 maxLBA;
    u32 rootDirtocLBA;
    u32 rootDirtocLength;
} layer_info_t;
static struct
{
    layer_info_t layer_info[2];
} MountPoint;

static u8 disc[64][2048];
static unsigned char cdvdman_buf[2048];

static int sceCdReadDvdDualInfo(int *on_dual, u32 *layer1_start)
{
    *on_dual = 0;
    *layer1_start = 0;
    return 1;
}
static int cdEmuRead(u32 lsn, unsigned int count, void *buffer)
{
    memcpy(buffer, disc[lsn], count * 2048);
    return 0;
}

#pragma pack(push, 1)
#include "find_extracted.c"
#pragma pack(pop)

static int rec(u8 *sec, int pos, int len_dr, u32 lba, u32 size, int flags, const char *name)
{
    u8 *r = sec + pos;
    int name_len = name[0] < 2 ? 1 : (int)strlen(name); /* names 0x00 and 0x01 are . and .. */
    r[0] = (u8)len_dr;
    memcpy(r + 2, &lba, 4);
    memcpy(r + 10, &size, 4);
    r[25] = (u8)flags;
    r[32] = (u8)name_len;
    memcpy(r + 33, name, name_len);
    return pos + len_dr;
}

int main(int argc, char **argv)
{
    cd_file_t cdfile;
    int pos = 0, r;

    pos = rec(disc[20], pos, 34, 20, 2048, 2, "\0");
    pos = rec(disc[20], pos, 34, 20, 2048, 2, "\1");
    pos = rec(disc[20], pos, 46, 24, 100, 0, "SYSTEM.CNF;1");
    pos = rec(disc[20], pos, 56, 25, 200, 0, "LONGNAME_123456.TXT;1");
    pos = rec(disc[20], pos, 66, 26, 300, 0, "A_VERY_LONG_LEVEL2_FILENAME.TXT;1");
    MountPoint.layer_info[0].rootDirtocLBA = 20;
    MountPoint.layer_info[0].rootDirtocLength = 2048;

    memset(&cdfile, 0, sizeof(cdfile));
    r = cdvdman_findfile(&cdfile, argv[1], 0);
    printf("%s: found=%d lsn=%u size=%u name=\"%.16s\"\n", argv[1], r, cdfile.lsn, cdfile.size, cdfile.name);
    return 0;
}
