/* Host harness for sysCheckVMC() from src/system.c, as the VMC menu calls it:
 *   vmc <dir> <name> <createSize>
 * <dir>/VMC/<name>.bin is the card. Prints the result, the card's size afterwards and whether
 * a new card was requested from genvmc. */
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#define LOG(...)
#define S_IXUSR       0
#define S_IRGRP       0
#define S_IWGRP       0
#define S_IXGRP       0
#define S_IROTH       0
#define S_IWOTH       0
#define S_IXOTH       0
#define open(p, f, m) _open(p, (f) | _O_BINARY, _S_IREAD | _S_IWRITE)
#define lseek         _lseek
#define read          _read
#define close         _close
#define unlink        _unlink

typedef struct
{
    char magic[40];
    u16 page_size;
    u16 pages_per_cluster;
    u16 pages_per_block;
    u16 unused0;
    u32 clusters_per_card;
    u8 rest[300];
    u8 mc_type;
    u8 mc_flag;
} vmc_superblock_t;

typedef struct
{
    char VMC_filename[1024];
    int VMC_size_mb;
    int VMC_blocksize;
    int VMC_thread_priority;
    int VMC_card_slot;
} createVMCparam_t;

static int created;
static int fileXioDevctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    created = ((createVMCparam_t *)arg)->VMC_size_mb;
    return 0;
}

#include "vmc_extracted.c"

int main(int argc, char **argv)
{
    char prefix[512], path[600];
    struct _stat st;
    int r;

    sprintf(prefix, "%s\\", argv[1]);
    r = sysCheckVMC(prefix, "\\", argv[2], atoi(argv[3]), NULL);
    sprintf(path, "%sVMC\\%s.bin", prefix, argv[2]);
    if (_stat(path, &st) == 0)
        printf("returned %d; card kept, %ld bytes", r, (long)st.st_size);
    else
        printf("returned %d; no card file", r);
    printf(created ? "; genvmc asked for a new %d MB card\n" : "; no new card\n", created);
    return 0;
}
