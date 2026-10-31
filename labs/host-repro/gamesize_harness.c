/* Host harness for the size that src/supportbase.c sbPopulateConfig() reads for the game info page:
 *   gamesize iso|old|ul <prefix> <separator>
 * Prints the paths it asks stat() for (every one "exists" with 700 MiB) and the size it reports. The game's
 * files are where sbCreatePath() puts them, the paths every launcher opens. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned char u8;
typedef unsigned int u32;

#define ISO_GAME_NAME_MAX      160
#define GAME_STARTUP_MAX       12
#define ISO_GAME_EXTENSION_MAX 4
#define GAME_FORMAT_USBLD      0
#define GAME_FORMAT_ISO        1
#define GAME_FORMAT_OLD_ISO    2
#define SCECdPS2CD             0x12
#define SCECdPS2DVD            0x14
#define CONFIG_ITEM_NAME       "#Name"
#define CONFIG_ITEM_SIZE       "#Size"
#define CONFIG_ITEM_FORMAT     "#Format"
#define CONFIG_ITEM_MEDIA      "#Media"
#define CONFIG_ITEM_STARTUP    "$Startup"

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
    int size;
} config_set_t;

static config_set_t theConfig;
static config_set_t *configAlloc(int type, config_set_t *configSet, char *fileName) { return &theConfig; }
static int configRead(config_set_t *c) { return 0; }
static void configMove(config_set_t *c, const char *fileName) {}
static void configSetStr(config_set_t *c, const char *key, const char *value) {}
static void configSetInt(config_set_t *c, const char *key, int value)
{
    if (!strcmp(key, CONFIG_ITEM_SIZE))
        c->size = value;
}
static unsigned int USBA_crc32(const char *s) { return 0x12345678; }

static int harness_stat(const char *path, struct stat *st)
{
    printf("%s ", path);
    st->st_size = 700 << 20;
    return 0;
}
#define stat(path, st) harness_stat(path, st)

void sbCreatePath(const base_game_info_t *game, char *path, const char *prefix, const char *sep, int part);

#include "gamesize_extracted.c"

int main(int argc, char **argv)
{
    base_game_info_t game = {"Some Game", "SLUS_123.45", ".iso", 1, SCECdPS2DVD, GAME_FORMAT_ISO, 0};

    if (!strcmp(argv[1], "old"))
        game.format = GAME_FORMAT_OLD_ISO;
    if (!strcmp(argv[1], "ul")) {
        game.format = GAME_FORMAT_USBLD;
        game.extension[0] = '\0';
        game.parts = 2;
    }
#ifdef HAVE_CFGDIR
    sbPopulateConfig(&game, argv[2], argv[3], NULL);
#else
    sbPopulateConfig(&game, argv[2], argv[3]);
#endif
    printf("-> %d MB\n", theConfig.size);
    return 0;
}
