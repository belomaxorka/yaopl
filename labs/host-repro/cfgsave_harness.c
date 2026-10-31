/* Host harness for saving OPL's settings (src/config.c configWriteMulti(), configWrite(), configSetMove() with
 * the file buffer of src/util.c), with the start of _saveConfig() mirrored: write the settings where they were
 * read; if that reports a failure, move them to another place and write them there.
 *   cfgsave <home dir> <other dir>
 * conf_opl.cfg is modified; conf_network.cfg and conf_game.cfg were read and not changed. In <home dir> a folder
 * named conf_opl.cfg makes that file fail, as a full or removed device does. Prints what the save reports and
 * which files each place holds afterwards. */
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned int u32;
#define LOG(...)
#define CONFIG_KEY_NAME_LEN  32
#define CONFIG_KEY_VALUE_LEN 256
#define CONFIG_OPL           0x01
#define CONFIG_NETWORK       0x02
#define CONFIG_GAME          0x04
#define CONFIG_LAST          0x08
#define CONFIG_APPS          0x10
#define CONFIG_INDEX_COUNT   5

typedef struct
{
    int fd;
    int mode;
    char *buffer;
    unsigned int size;
    unsigned int available;
    char *lastPtr;
    short allocResult;
    short writeError;
} file_buffer_t;

struct config_value_t
{
    char key[CONFIG_KEY_NAME_LEN];
    char val[CONFIG_KEY_VALUE_LEN];
    struct config_value_t *next;
};
typedef struct
{
    int type;
    struct config_value_t *head;
    struct config_value_t *tail;
    char *filename;
    int modified;
    u32 uid;
} config_set_t;

static config_set_t configFiles[CONFIG_INDEX_COUNT];
static const char *configFilenames[CONFIG_INDEX_COUNT] = {"conf_opl.cfg", "conf_last.cfg", "conf_apps.cfg", "conf_network.cfg", "conf_game.cfg"};
static const int configTypes[CONFIG_INDEX_COUNT] = {CONFIG_OPL, CONFIG_LAST, CONFIG_APPS, CONFIG_NETWORK, CONFIG_GAME};
static char legacyNetConfigPath[256];
static char *gBaseMCDir = "mc?:OPL";
static void configPrepareNotifications(char *prefix) {}
void configMove(config_set_t *configSet, const char *fileName);

#define write          _write
#define read           _read
#define close          _close
#define lseek          _lseek
#define openFile(p, m) _open(p, (m) | _O_BINARY, _S_IREAD | _S_IWRITE)
#define bgmMute()
#define bgmUnMute()

#include "cfgsave_extracted.c"

static void setFile(int i, const char *dir, const char *key, int modified)
{
    static struct config_value_t values[CONFIG_INDEX_COUNT];
    char path[512];

    snprintf(path, sizeof(path), "%s/%s", dir, configFilenames[i]);
    configFiles[i].type = configTypes[i];
    configFiles[i].filename = _strdup(path);
    if (key != NULL) {
        snprintf(values[i].key, sizeof(values[i].key), "%s", key);
        snprintf(values[i].val, sizeof(values[i].val), "1");
        configFiles[i].head = configFiles[i].tail = &values[i];
    }
    configFiles[i].modified = modified;
}

static void listFiles(const char *label, const char *dir)
{
    char path[512];
    struct _stat st;
    int i, n = 0;

    printf(" %s:", label);
    for (i = 0; i < CONFIG_INDEX_COUNT; i++) {
        snprintf(path, sizeof(path), "%s/%s", dir, configFilenames[i]);
        if (_stat(path, &st) == 0 && (st.st_mode & _S_IFREG)) {
            printf(" %s", configFilenames[i]);
            n++;
        }
    }
    if (n == 0)
        printf(" -");
}

int main(int argc, char **argv)
{
    int types = CONFIG_OPL | CONFIG_NETWORK | CONFIG_GAME, r, moved = 0;

    // As after a start that read all three from <home dir>: the settings were changed, the others were not
    setFile(0, argv[1], "scrolling", 1);
    setFile(1, argv[1], NULL, 0);
    setFile(2, argv[1], NULL, 0);
    setFile(3, argv[1], "eth_mode", 0);
    setFile(4, argv[1], "gsm", 0);

    // _saveConfig(): where the settings are, then (trySaveAlternateDevice()) another place
    r = configWriteMulti(types);
    if (r == 0) {
        configSetMove(argv[2]);
        r = configWriteMulti(types);
        moved = 1;
    }
    printf("save reports %s%s |", r > 0 ? "saved" : "error", moved ? " (moved)" : "");
    listFiles("home", argv[1]);
    printf(" |");
    listFiles("other", argv[2]);
    printf("\n");
    return 0;
}
