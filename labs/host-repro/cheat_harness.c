/* Host harness for src/cheatman.c parse_buf(). cheat_extracted.c is produced by
 * extract.py from the OPL checkout under test, so the same harness runs against
 * master and against a fix branch. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;

#define MAX_HOOKS      5
#define MAX_CODES      250
#define MAX_CHEATLIST  (MAX_HOOKS * 2 + MAX_CODES * 2)
#define CHEAT_NAME_MAX 128
#define NUL            0x00
#define LF             0x0A
#define CR             0x0D
#define SPACE          0x20
#define CODE_DIGITS    16
#define LOG(...)

typedef struct
{
    u32 addr;
    u32 val;
} code_t;

typedef struct
{
    char name[CHEAT_NAME_MAX + 1];
    code_t codes[MAX_CHEATLIST];
    int enabled;
} cheat_entry_t;

cheat_entry_t gCheats[MAX_CODES];
static int gCheatMode = 1; /* select mode: only master codes start enabled */

static int cheatConfigIsMasterCode(const char *name)
{
    return _stricmp(name, "mastercode") == 0 || _stricmp(name, "master code") == 0;
}

#include "cheat_extracted.c"

static int run(const char *path);

int main(int argc, char **argv)
{
    char path[1024];
    FILE *list;

    if (argc == 3 && strcmp(argv[1], "-l") == 0) { /* -l <file with one path per line> */
        list = fopen(argv[2], "r");
        while (list != NULL && fgets(path, sizeof(path), list) != NULL) {
            path[strcspn(path, "\r\n")] = '\0';
            printf("## %s\n", strrchr(path, '\\') ? strrchr(path, '\\') + 1 : path);
            run(path);
        }
        return 0;
    }
    return run(argv[1]);
}

static int run(const char *path)
{
    FILE *f = fopen(path, "rb");
    long size;
    char *buf;
    int i, j;

    if (f == NULL)
        return 2;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);
    buf = malloc(size + 1); /* same layout as read_text_file(): exactly size + 1 */
    fread(buf, 1, size, f);
    fclose(f);
    buf[size] = '\0';

    memset(gCheats, 0, sizeof(gCheats));
    parse_buf(buf);
    free(buf);

    for (i = 0; i < MAX_CODES && gCheats[i].name[0] != NUL; i++) {
        for (j = 0; j < MAX_CHEATLIST && gCheats[i].codes[j].addr != 0; j++)
            ;
        printf("cheat %3d name=\"%s\" enabled=%d codes=%d first=%08X\n", i, gCheats[i].name, gCheats[i].enabled, j, gCheats[i].codes[0].addr);
    }
    printf("total cheats: %d\n", i);
    return 0;
}
