/* Host harness for the remembered cheat picks in src/cheatconfig.c: picks are saved with
 * cheatConfigSaveSelections() and loaded back with cheatConfigLoadSelections(). The names are
 * given on the command line; a leading '+' marks a picked cheat. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;
#define NUL                                0x00
#define SPACE                              0x20
#define MAX_CODES                          250
#define CHEAT_NAME_MAX                     128
#define CONFIG_KEY_NAME_LEN                32
#define CONFIG_KEY_VALUE_LEN               256
#define CONFIG_ITEM_CHEAT_SELECTION_PREFIX "$CheatSel"
#define CHEAT_SELECTION_KEY_MAX            CONFIG_KEY_NAME_LEN

typedef struct
{
    char name[CHEAT_NAME_MAX + 1];
    int enabled;
} cheat_entry_t;
static cheat_entry_t gCheats[MAX_CODES];

/* A tiny config set: enough key/value pairs for the picks. */
typedef struct
{
    char key[64][CONFIG_KEY_NAME_LEN];
    char val[64][CONFIG_KEY_VALUE_LEN];
    int count;
} config_set_t;
static int find(config_set_t *s, const char *key)
{
    int i;
    for (i = 0; i < s->count; i++)
        if (strcmp(s->key[i], key) == 0)
            return i;
    return -1;
}
static int configGetStrCopy(config_set_t *s, const char *key, char *val, int size)
{
    int i = find(s, key);
    if (i < 0)
        return 0;
    strncpy(val, s->val[i], size);
    return 1;
}
static int configSetStr(config_set_t *s, const char *key, const char *val)
{
    int i = find(s, key);
    if (i < 0)
        i = s->count++;
    strcpy(s->key[i], key);
    strcpy(s->val[i], val);
    return 1;
}
static int configRemoveKey(config_set_t *s, const char *key)
{
    int i = find(s, key);
    if (i >= 0)
        s->key[i][0] = '\0';
    return i >= 0;
}

#include "cheatsel_extracted.c"

int main(int argc, char **argv)
{
    static config_set_t set;
    int i;

    for (i = 1; i < argc; i++) {
        strcpy(gCheats[i - 1].name, argv[i][0] == '+' ? argv[i] + 1 : argv[i]);
        gCheats[i - 1].enabled = argv[i][0] == '+';
    }
    cheatConfigSaveSelections(&set);
    for (i = 0; i < argc - 1; i++)
        gCheats[i].enabled = 0;
    cheatConfigLoadSelections(&set);
    printf("picked after reload:");
    for (i = 0; i < argc - 1; i++)
        printf(" [%d %s]=%d", i, gCheats[i].name, gCheats[i].enabled);
    printf("\n");
    return 0;
}
