/* Host harness for cdvdman_locatefile(): modules/isofs/isofs.c (EE game list) or, with
 * /DINGAME, modules/iopcore/cdvdman/searchfile.c (in-game CDVDMAN). The root directory
 * of a fake disc is built from the test name given in argv[1]. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#define __attribute__(x)
#define DPRINTF(...)

static u8 disc[64][2048];
static unsigned char cdvdman_buf[2048];
static long reads; /* sector reads, to tell a hang from a slow scan */

static int sceCdReadDvdDualInfo(int *on_dual, unsigned int *layer1_start)
{
    *on_dual = 0;
    *layer1_start = 0;
    return 1;
}

#ifdef INGAME
#define IOPCORE_COMPAT_EMU_DVDDL 0x10
static struct
{
    struct
    {
        u32 flags;
    } common;
} cdvdman_settings;
static int sceCdRead(u32 lsn, u32 count, void *buf, void *mode)
{
    if (++reads > 1000000 || lsn + count > 64)
        exit(3);
    memcpy(buf, disc[lsn], count * 2048);
    return 1;
}
static int sceCdSync(int mode) { return 0; }
#else
static int cdEmuRead(u32 lsn, unsigned int count, void *buffer)
{
    if (++reads > 1000000 || lsn + count > 64)
        exit(3);
    memcpy(buffer, disc[lsn], count * 2048);
    return 0;
}
#endif

#pragma pack(push, 1)
#include "iso_extracted.c"
#pragma pack(pop)

/* ISO 9660 9.1: LEN_DR, extended attribute length, LBA (both-endian), size, ..., flags, name. */
static int rec(u8 *sec, int pos, int len_dr, int xa, u32 lba, u32 size, int flags, const char *name, int name_len)
{
    u8 *r = sec + pos;
    r[0] = (u8)len_dr;
    r[1] = (u8)xa;
    memcpy(r + 2, &lba, 4);
    memcpy(r + 10, &size, 4);
    r[25] = (u8)flags;
    r[32] = (u8)name_len;
    memcpy(r + 33, name, name_len);
    return pos + len_dr;
}

int main(int argc, char **argv)
{
    const char *test = argv[1];
    u8 *root = disc[20];
    int pos = 0;
    struct dirTocEntry *e;
#ifdef INGAME
    char path[] = "\\SYSTEM.CNF;1";
#else
    char path[] = "SYSTEM.CNF;1";
#endif

    if (strcmp(test, "file") == 0) { /* an image file whose root directory is at LBA 20 */
        FILE *f = fopen(argv[2], "rb");
        if (f == NULL)
            return 2;
        fread(disc, 1, sizeof(disc), f);
        fclose(f);
        goto locate;
    }
    pos = rec(root, pos, 34, 0, 20, 2048, 2, "\0", 1);
    pos = rec(root, pos, 34, 0, 20, 2048, 2, "\1", 1);
    if (strcmp(test, "zero") == 0) /* name length 0, record length 34 */
        pos = rec(root, pos, 34, 0, 30, 10, 0, "", 0);
    else if (strcmp(test, "xa") == 0) /* extended attribute length 0x80 in byte 1 */
        pos = rec(root, pos, 46, 0x80, 30, 10, 0, "A.TXT;1", 7);
    else if (strcmp(test, "long") == 0) /* ISO 9660 level 2 name of 33 characters */
        pos = rec(root, pos, 66, 0, 30, 10, 0, "A_VERY_LONG_LEVEL2_FILENAME.TXT;1", 33);
    pos = rec(root, pos, 46, 0, 24, 100, 0, "SYSTEM.CNF;1", 12);

locate:
    e = cdvdman_locatefile(path, 20, 2048, 0);
    if (e != NULL)
        printf("found lba=%u size=%u after %ld sector reads\n", e->fileLBA, e->fileSize, reads);
    else
        printf("not found after %ld sector reads\n", reads);
    return 0;
}
