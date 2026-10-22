/* Host harness: the game ID OPL's game list derives from an image file, as in
 * scanForISO(): find SYSTEM.CNF;1 in the root with isofs cdvdman_locatefile(), then
 * GetStartupExecName() (src/supportbase.c) and ps2cnfGetBootFile() (src/ps2cnf.c). */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned int u32;
#define __attribute__(x)
#define DPRINTF(...)
#define LOG(...)
#define CNF_PATH_LEN_MAX 64
#define GAME_STARTUP_MAX 12

static FILE *image;
static unsigned char cdvdman_buf[2048];

static int sceCdReadDvdDualInfo(int *on_dual, u32 *layer1_start)
{
    *on_dual = 0;
    *layer1_start = 0;
    return 1;
}
static int cdEmuRead(u32 lsn, unsigned int count, void *buffer)
{
    memset(buffer, 0, count * 2048);
    fseek(image, (long)lsn * 2048, SEEK_SET);
    fread(buffer, 2048, count, image);
    return 0;
}
int ps2cnfGetBootFile(const char *path, char *bootfile);

#pragma pack(push, 1)
#include "gameid_extracted.c"
#pragma pack(pop)

int main(int argc, char **argv)
{
    static u8 pvd[2048], cnf[2048];
    char startup[GAME_STARTUP_MAX], tmp[L_tmpnam + 16];
    struct dirTocEntry *root, *e;
    char path[] = "SYSTEM.CNF;1";
    FILE *t;
    int r;

    image = fopen(argv[1], "rb");
    if (image == NULL)
        return 2;
    cdEmuRead(16, 1, pvd);
    if (pvd[0] != 1 || memcmp(&pvd[1], "CD001", 5) != 0) {
        printf("no ISO9660 volume: skipped\n");
        return 0;
    }
    root = (struct dirTocEntry *)&pvd[0x9c];
    e = cdvdman_locatefile(path, root->fileLBA, root->fileSize, 0);
    if (e == NULL) {
        printf("no SYSTEM.CNF;1: hidden\n");
        return 0;
    }
    cdEmuRead(e->fileLBA, 1, cnf);
    sprintf(tmp, "%s.cnf", argv[1]);
    t = fopen(tmp, "wb");
    fwrite(cnf, 1, e->fileSize < 2048 ? e->fileSize : 2048, t);
    fclose(t);
    memset(startup, 0, sizeof(startup));
    r = GetStartupExecName(tmp, startup, GAME_STARTUP_MAX - 1);
    remove(tmp);
    if (r != 0)
        printf("hidden (GetStartupExecName = %d)\n", r);
    else
        printf("ID \"%s\"\n", startup);
    return 0;
}
