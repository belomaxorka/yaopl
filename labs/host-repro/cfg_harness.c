/* Host harness for the text-file and config helpers of src/util.c and src/config.c:
 *   cfg lines <file> [readerror]  readFileBuffer() lines of a file, as configRead() reads them
 *   cfg color <string>            strToColor() into color[3], with a canary after it
 *   cfg split <line>              splitAssignment() into key[32] and val[256]
 *   cfg prefix <line>             parsePrefix() into prefix[32], with a canary after it
 *   cfg legacyip <IPCONFIG.DAT>   configReadLegacyIP()
 *   cfg logo                      CheckPS2Logo() for an HDD game, counting sectors read */
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned int u32;
#define ALIGNED(x)
#define LOG(...)

typedef struct
{
    int fd;
    int mode;
    char *buffer;
    unsigned int size;
    unsigned int available;
    char *lastPtr;
    short allocResult;
    short writeError; /* as in include/util.h; unused by older trees */
} file_buffer_t;

static int failReads; /* make read() fail, as on a USB unplug or SMB drop */
static int harness_read(int fd, void *buf, unsigned int n)
{
    return failReads ? -1 : _read(fd, buf, n);
}
#define read            harness_read
#define write           _write
#define close           _close
#define lseek           _lseek
#define openFile(p, m)  _open(p, (m) | _O_BINARY)
#define getFileSize(fd) _filelength(fd)

/* CheckPS2Logo() */
#define ZSO_MAGIC                 0x4F53495A
#define CONSOLE_REGION_EUROPE     2
#define guiWarning(text, timeout) ((void)0)
typedef struct
{
    u32 magic;
} ZISO_header;
static int ConsoleRegion, gEnableDebug, probed_fd;
static u32 probed_lba;
static int ziso_init(ZISO_header *h, u32 first) { return -1; }
static int ziso_read_sector(u8 *buf, u32 sector, unsigned int count) { return 0; }
static int sectorsRead;
static int hddReadSectors(u32 lba, u32 nsectors, void *buf)
{
    sectorsRead += nsectors;
    memset(buf, 0x11, nsectors * 512);
    return 0;
}

/* configReadLegacyIP() */
#define CONFIG_INDEX_NETWORK 3
#define CONFIG_NET_PS2_IP    "ps2_ip_addr"
#define CONFIG_NET_PS2_NETM  "ps2_netmask"
#define CONFIG_NET_PS2_GATEW "ps2_gateway"
#define CONFIG_NET_PS2_DNS   "ps2_dns"
typedef struct
{
    int unused;
} config_set_t;
static config_set_t configFiles[8];
static char legacyNetConfigPath[256];
static int ps2_ip[4], ps2_netmask[4], ps2_gateway[4];
static int configSetStr(config_set_t *set, const char *key, const char *value)
{
    printf("%s=%s ", key, value);
    return 1;
}

#undef min
#undef max
#include "cfg_extracted.c"

int main(int argc, char **argv)
{
    const char *mode = argv[1];

    if (strcmp(mode, "lines") == 0) {
        file_buffer_t *fb = openFileBuffer(argv[2], O_RDONLY, 0, 4096);
        char *line;
        int n = 0;

        failReads = argc > 3;
        while (fb != NULL && readFileBuffer(fb, &line) && n < 20)
            printf("line %d: \"%s\"\n", ++n, line);
        printf("%d lines\n", n);
    } else if (strcmp(mode, "color") == 0) {
        struct
        {
            unsigned char color[3];
            unsigned char canary[13];
        } c;
        memset(&c, 0xCC, sizeof(c));
        strToColor(argv[2], c.color);
        printf("color %02X %02X %02X, canary %02X %02X %02X %02X\n", c.color[0], c.color[1], c.color[2], c.canary[0], c.canary[1], c.canary[2], c.canary[3]);
    } else if (strcmp(mode, "split") == 0) {
        struct
        {
            char key[32], val[256];
        } kv;
        char line[1024];
        memset(&kv, 0, sizeof(kv));
        strcpy(line, argv[2]);
        splitAssignment(line, kv.key, sizeof(kv.key), kv.val, sizeof(kv.val));
        printf("key length %u (buffer 32): \"%s\"\n", (unsigned)strlen(kv.key), kv.key);
    } else if (strcmp(mode, "prefix") == 0) {
        struct
        {
            char prefix[32];
            char canary[96];
        } p;
        char line[1024];
        memset(&p, 0, sizeof(p));
        strcpy(line, argv[2]);
#ifdef PREFIX_MAX
        parsePrefix(line, p.prefix, sizeof(p.prefix));
#else
        parsePrefix(line, p.prefix);
#endif
        printf("prefix length %u, canary %s\n", (unsigned)strlen(p.prefix), p.canary[0] ? "OVERWRITTEN" : "intact");
    } else if (strcmp(mode, "legacyip") == 0) {
        strcpy(legacyNetConfigPath, argv[2]);
        printf("-> %d\n", configReadLegacyIP());
    } else if (strcmp(mode, "logo") == 0) {
        CheckPS2Logo(0, 1000);
        printf("HDD logo check read %d sectors of 512 bytes into a %d-byte buffer\n", sectorsRead, 12 * 2048);
    }
    return 0;
}
