/* Host harness for the settings of games on a UDPBD server that drops what is written (src/bdmsupport.c
 * bdmCheckWritable() and bdmGetConfig(), src/supportbase.c sbPopulateConfig()):
 *   udpbdro <dir> probe rw|drop|stale|fail
 *   udpbdro <dir> cfg rw|drop|drop-saved|drop-usbhome|usb
 * "massN:" maps to <dir>\massN\ (the game's device), "mc0:" to <dir>\mc0\ (where conf_game.cfg is).
 * The device is a UDPBD server that keeps what is written (rw), drops it but acknowledges it (drop, as
 * udpbd-vexfat does), drops it while its sectors still hold an earlier check's data (stale) or refuses to
 * open (fail); usb is a USB drive. "probe" runs the write check; "cfg" sets the device flag the way
 * bdmUpdateDeviceData() does (the check where the tree has one, else none), loads the game's settings, saves
 * a new key, loads them again and reports where they went and whether the device's CFG file changed. */
#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
#define LOG(...)

#define ISO_GAME_NAME_MAX      160
#define GAME_STARTUP_MAX       12
#define ISO_GAME_EXTENSION_MAX 4
#define GAME_FORMAT_USBLD      0
#define GAME_FORMAT_ISO        1
#define GAME_FORMAT_OLD_ISO    2
#define SCECdPS2CD             0x12
#define CONFIG_GAME            2
#define CONFIG_ITEM_NAME       "#Name"
#define CONFIG_ITEM_SIZE       "#Size"
#define CONFIG_ITEM_FORMAT     "#Format"
#define CONFIG_ITEM_MEDIA      "#Media"
#define CONFIG_ITEM_STARTUP    "$Startup"
#define BDM_TYPE_USB           0
#define BDM_TYPE_UDPBD         4
#define MODE_FLAG_READ_ONLY    0x08

typedef struct
{
    char name[ISO_GAME_NAME_MAX + 1];
    char startup[GAME_STARTUP_MAX + 1];
    char extension[ISO_GAME_EXTENSION_MAX + 1];
    u8 parts;
    u8 media;
    u8 format;
    u32 sizeMB;
} base_game_info_t;

typedef struct
{
    char bdmPrefix[40];
    base_game_info_t *bdmGames;
    int bdmDeviceType;
} bdm_device_data_t;

typedef struct
{
    int flags;
    void *priv;
} item_list_t;

/* A small file-backed config: keys in order, "key=value" lines. */
typedef struct
{
    int type;
    char *filename;
    char key[16][32];
    char val[16][64];
    int count;
    int modified;
} config_set_t;

static char root[512];
static const char *server; /* rw, drop, stale, fail or usb */
static u32 timerSeed;

static void localPath(char *local, const char *path)
{
    const char *colon = strchr(path, ':');
    char *p;
    sprintf(local, "%s\\%.*s\\%s", root, (int)(colon - path), path, colon[1] == '/' ? colon + 2 : colon + 1);
    for (p = local; *p; p++)
        if (*p == '/')
            *p = '\\';
}

static int dropsWrites(const char *path)
{
    return !strncmp(path, "mass0", 5) && (!strcmp(server, "drop") || !strcmp(server, "stale"));
}

/* File calls of the write check. A server that drops writes still creates the file in the directory
 * sectors FatFs keeps in memory, so open() works; the data never reaches the disk. */
static int dropFd = -1;
static int harness_open(const char *path, int flags, ...)
{
    char local[1024];
    int fd;
    localPath(local, path);
    if (!strncmp(path, "mass", 4) && !strcmp(server, "fail") && (flags & O_WRONLY))
        return -1;
    if (dropsWrites(path))
        flags &= ~O_TRUNC; /* the sectors keep what they held */
    fd = _open(local, flags | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (dropsWrites(path) && (flags & O_WRONLY))
        dropFd = fd;
    return fd;
}
static int harness_write(int fd, const void *buf, unsigned int size)
{
    if (fd == dropFd)
        return (int)size;
    return _write(fd, buf, size);
}
static int harness_remove(const char *path)
{
    char local[1024];
    localPath(local, path);
    return remove(local);
}
static int harness_mkdir(const char *path, int mode)
{
    char local[1024];
    localPath(local, path);
    return _mkdir(local);
}
static u64 GetTimerSystemTime(void) { return timerSeed; }

/* What openFile() needs for a memory-card path: the card is mc0 */
typedef struct
{
    int dummy;
} DIR;
static DIR fakeDir;
static char mcID = '0';
static int checkMC(void) { return 0; }
static DIR *opendir(const char *path)
{
    char local[1024];
    struct _stat st;
    localPath(local, path);
    return _stat(local, &st) == 0 ? &fakeDir : NULL;
}
static int closedir(DIR *dir) { return 0; }
static unsigned int USBA_crc32(const char *s) { return 0; }

static config_set_t gameConfig = {CONFIG_GAME, "mc0:OPL/conf_game.cfg"};
static config_set_t *configGetByType(int type) { return &gameConfig; }

static config_set_t *configAlloc(int type, config_set_t *configSet, char *fileName)
{
    config_set_t *c = calloc(1, sizeof(config_set_t));
    c->type = type;
    c->filename = _strdup(fileName);
    return c;
}
static void configMove(config_set_t *configSet, const char *fileName)
{
    free(configSet->filename);
    configSet->filename = _strdup(fileName);
}
static void configSetStr(config_set_t *c, const char *key, const char *value)
{
    int i;
    for (i = 0; i < c->count && strcmp(c->key[i], key); i++)
        ;
    if (i == c->count)
        strcpy(c->key[c->count++], key);
    strcpy(c->val[i], value);
    c->modified = 1;
}
static void configSetInt(config_set_t *c, const char *key, int value)
{
    char s[16];
    sprintf(s, "%d", value);
    configSetStr(c, key, s);
}
static int configRead(config_set_t *c)
{
    char local[1024], line[128];
    FILE *f;
    localPath(local, c->filename);
    if ((f = fopen(local, "r")) == NULL)
        return 0;
    while (fgets(line, sizeof(line), f)) {
        char *eq = strchr(line, '=');
        line[strcspn(line, "\r\n")] = '\0';
        if (eq) {
            *eq = '\0';
            configSetStr(c, line, eq + 1);
        }
    }
    fclose(f);
    c->modified = 0;
    return 1;
}
/* As src/config.c configWrite(): the file is opened through src/util.c openFile(), which makes a missing folder
 * where the tree does so */
int openFile(char *path, int mode);
static int configWrite(config_set_t *c)
{
    char local[1024];
    FILE *f;
    int i, fd;
    if (dropsWrites(c->filename))
        return 1; /* acknowledged, gone */
    if ((fd = openFile(c->filename, O_WRONLY | O_CREAT | O_TRUNC)) < 0)
        return 0;
    _close(fd);
    localPath(local, c->filename);
    if ((f = fopen(local, "w")) == NULL)
        return 0;
    for (i = 0; i < c->count; i++)
        if (c->key[i][0] != '#')
            fprintf(f, "%s=%s\n", c->key[i], c->val[i]);
    fclose(f);
    return 1;
}
static const char *configGet(config_set_t *c, const char *key)
{
    int i;
    for (i = 0; i < c->count; i++)
        if (!strcmp(c->key[i], key))
            return c->val[i];
    return "-";
}

#define open   harness_open
#define write  harness_write
#define read   _read
#define close  _close
#define remove harness_remove
#define mkdir  harness_mkdir

#include "udpbdro_extracted.c"

#undef remove
#undef mkdir

static void readFile(const char *rel, char *out, int size)
{
    char local[1024];
    FILE *f;
    sprintf(local, "%s\\%s", root, rel);
    out[0] = '\0';
    if ((f = fopen(local, "rb")) != NULL) {
        out[fread(out, 1, size - 1, f)] = '\0';
        fclose(f);
    }
}

static void writeFile(const char *rel, const char *text)
{
    char local[1024];
    FILE *f;
    sprintf(local, "%s\\%s", root, rel);
    f = fopen(local, "wb");
    fputs(text, f);
    fclose(f);
}

static int probe(int verbose)
{
#ifdef HAVE_PROBE
    char local[1024];
    struct _stat st;
    int result, i, fd;

    sprintf(local, "%s\\mass0\\yaopl_write_test.tmp", root);
    if (!strcmp(server, "stale")) { /* an earlier check, on a day the server wrote, left its data here */
        u32 old[128];
        for (i = 0; i < 128; i++)
            old[i] = 1111 + i * 0x9E3779B9;
        fd = _open(local, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);
        _write(fd, old, sizeof(old));
        _close(fd);
    }
    timerSeed = 2222;
    result = bdmCheckWritable(0);
    dropFd = -1;
    if (verbose)
        printf("write check %d, test file %s\n", result, _stat(local, &st) == 0 ? "LEFT" : "removed");
    return result;
#else
    if (verbose)
        printf("no write check\n");
    return 1;
#endif
}

int main(int argc, char **argv)
{
    char dir[1024], before[256], after[256];
    base_game_info_t game = {"Some Game", "SLUS_123.45", ".iso", 0, 0x14, GAME_FORMAT_ISO, 100};
    bdm_device_data_t device = {"mass0:", &game, BDM_TYPE_UDPBD};
    item_list_t itemList = {0, &device};
    config_set_t *c;
    char first[64], saved[64], again[64], again2[64];

    strcpy(root, argv[1]);
    server = argv[3];
    sprintf(dir, "%s\\mass0", root);
    _mkdir(dir);
    sprintf(dir, "%s\\mass0\\CFG", root);
    _mkdir(dir);
    sprintf(dir, "%s\\mc0", root);
    _mkdir(dir);
    sprintf(dir, "%s\\mc0\\OPL", root);
    _mkdir(dir);
    sprintf(dir, "%s\\mc0\\OPL\\CFG", root);
    _mkdir(dir);

    if (!strcmp(argv[2], "probe")) {
        probe(1);
        return 0;
    }

    /* cfg */
    writeFile("mass0\\CFG\\SLUS_123.45.cfg", "$Compatibility=2\n");
    if (!strcmp(server, "drop-saved")) {
        server = "drop";
        writeFile("mc0\\OPL\\CFG\\SLUS_123.45.cfg", "$Compatibility=8\n");
    }
    if (!strcmp(server, "drop-usbhome")) {
        /* OPL's settings in the root of a USB drive (mass1:), which has no CFG folder there */
        server = "drop";
        gameConfig.filename = "mass1:/conf_game.cfg";
        sprintf(dir, "%s\\mass1", root);
        _mkdir(dir);
    }
    if (!strcmp(server, "usb"))
        device.bdmDeviceType = BDM_TYPE_USB;
    readFile("mass0\\CFG\\SLUS_123.45.cfg", before, sizeof(before));

    if (device.bdmDeviceType == BDM_TYPE_UDPBD && !probe(0))
        itemList.flags |= MODE_FLAG_READ_ONLY;

    c = bdmGetConfig(&itemList, 0);
    strcpy(first, configGet(c, "$Compatibility"));
    configSetStr(c, "$VMC_0", "slot1");
    printf("%s, Compatibility=%s, save %d", c->filename, first, configWrite(c));
    c = bdmGetConfig(&itemList, 0);
    strcpy(saved, configGet(c, "$VMC_0"));
    strcpy(again, configGet(c, "$Compatibility"));
    readFile("mass0\\CFG\\SLUS_123.45.cfg", after, sizeof(after));
    sprintf(again2, "%s", strcmp(before, after) ? "CHANGED" : "unchanged");
    printf(", reload VMC_0=%s Compatibility=%s, device CFG %s\n", saved, again, again2);
    return 0;
}
