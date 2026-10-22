/* Host harness for configWrite() (src/config.c) with the file buffer of src/util.c:
 *   cfgwrite <file> ok|fail|short
 * saves a modified two-key config while write() works, fails or writes half. */
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

static int writeMode; /* 0: works, 1: fails (-1), 2: writes half of the bytes */
static int harness_write(int fd, const void *buf, unsigned int n)
{
    if (writeMode == 1)
        return -1;
    return _write(fd, buf, writeMode == 2 ? n / 2 : n);
}
#define write          harness_write
#define read           _read
#define close          _close
#define lseek          _lseek
#define openFile(p, m) _open(p, (m) | _O_BINARY, _S_IREAD | _S_IWRITE)
#define bgmMute()
#define bgmUnMute()

#include "cfgwrite_extracted.c"

int main(int argc, char **argv)
{
    struct config_value_t b = {"b", "2", NULL}, a = {"a", "1", &b};
    config_set_t set = {0, &a, &b, argv[1], 1, 0};
    struct _stat st;
    int r;

    writeMode = strcmp(argv[2], "fail") == 0 ? 1 : strcmp(argv[2], "short") == 0 ? 2 :
                                                                                   0;
    r = configWrite(&set);
    _stat(argv[1], &st);
    printf("configWrite = %d, modified = %d, file %ld of 10 bytes\n", r, set.modified, (long)st.st_size);
    return 0;
}
