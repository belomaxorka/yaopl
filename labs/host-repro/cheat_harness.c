/* Host harness for src/cheatman.c parse_buf() and set_cheats_list(), and for
 * ee_core/src/cheat_api.c SetupCheats(). cheat_extracted.c is produced by
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

/* set_cheats_list() fills this list for the EE core (static in cheatman.c) */
static u32 gCheatList[MAX_CHEATLIST];

/* SetupCheats() reads the list through the EE core config and fills the engine's tables.
 * Separate globals, so AddressSanitizer sees a write past any of them. */
struct local_cfg
{
    u32 *gCheatList;
} local_config = {gCheatList};
#define USE_LOCAL_EECORE_CONFIG struct local_cfg *config = &local_config;
u32 numhooks;
u32 hooklist[MAX_HOOKS * 2];
u32 numcodes;
u32 codelist[MAX_CODES * 2];

#include "cheat_extracted.c"

static int run(const char *path, int engine, int sweep);

int main(int argc, char **argv)
{
    char path[1024];
    FILE *list;
    int engine = 0, sweep = 0;

    /* -l <list>: parse every file of the list; -e <file>: enable all cheats and build the engine
     * tables; -s <list>: report the files of the list whose cheats exceed the engine tables */
    if (argc == 3 && (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "-s") == 0)) {
        sweep = strcmp(argv[1], "-s") == 0;
        list = fopen(argv[2], "r");
        while (list != NULL && fgets(path, sizeof(path), list) != NULL) {
            path[strcspn(path, "\r\n")] = '\0';
            if (!sweep)
                printf("## %s\n", strrchr(path, '\\') ? strrchr(path, '\\') + 1 : path);
            run(path, sweep, sweep);
        }
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "-e") == 0)
        engine = 1;
    return run(argv[argc - 1], engine, 0);
}

static int code_count(const cheat_entry_t *cheat)
{
    int j;

    /* A code line may have address 0: only an all-zero pair, which parse_buf() never stores, ends the codes */
    for (j = 0; j < MAX_CHEATLIST && (cheat->codes[j].addr != 0 || cheat->codes[j].val != 0); j++)
        ;
    return j;
}

static int run(const char *path, int engine, int sweep)
{
    FILE *f = fopen(path, "rb");
    long size;
    char *buf;
    int i, j, parsed = 0, listed = 0, wantHooks = 0, wantCodes = 0, nextCodeCanBeHook = 1;

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

    if (!engine) {
        for (i = 0; i < MAX_CODES && gCheats[i].name[0] != NUL; i++)
            printf("cheat %3d name=\"%s\" enabled=%d codes=%d first=%08X\n", i, gCheats[i].name, gCheats[i].enabled, code_count(&gCheats[i]), gCheats[i].codes[0].addr);
        printf("total cheats: %d\n", i);
        return 0;
    }

    /* "enable all", as the automatic cheat mode does */
    for (i = 0; i < MAX_CODES && gCheats[i].name[0] != NUL; i++) {
        gCheats[i].enabled = 1;
        parsed += code_count(&gCheats[i]);
    }
    set_cheats_list();
    for (i = 0; i + 1 < MAX_CHEATLIST && (gCheatList[i] != 0 || gCheatList[i + 1] != 0); i += 2) {
        /* the same classification as SetupCheats(), to count what it would store */
        if ((gCheatList[i] & 0xfe000000) == 0x90000000 && nextCodeCanBeHook)
            wantHooks++;
        else
            wantCodes++;
        nextCodeCanBeHook = !((gCheatList[i] & 0xf0000000) == 0x40000000 || (gCheatList[i] & 0xf0000000) == 0x30000000);
        listed++;
    }
    if (sweep) {
        if (wantHooks > MAX_HOOKS || wantCodes > MAX_CODES)
            printf("%s: %d hooks, %d codes for tables of %d and %d\n", strrchr(path, '\\') ? strrchr(path, '\\') + 1 : path, wantHooks, wantCodes, MAX_HOOKS, MAX_CODES);
        return 0;
    }
    printf("engine list: %d of %d parsed codes", listed, parsed);
    if (listed < 16) {
        printf(":");
        for (j = 0; j < listed; j++)
            printf(" %08X/%08X", gCheatList[j * 2], gCheatList[j * 2 + 1]);
    }
    printf("\n");
    SetupCheats();
    printf("SetupCheats: numhooks=%u numcodes=%u (list had %d hooks and %d codes)\n", numhooks, numcodes, wantHooks, wantCodes);
    return 0;
}
