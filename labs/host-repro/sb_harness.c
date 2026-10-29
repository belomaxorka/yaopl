/* Host harness for src/supportbase.c:
 *   sb ul <dir> <iso games>          sbReadList() over <dir>\ul.cfg plus fake ISO games
 *   sb cache <dir> <cached> <listed> [swap]
 *                                    updateISOGameList() with a games.bin cache of <cached>
 *                                    games and a scanned list of <listed> games (same names,
 *                                    optionally in another order); prints whether it rewrote
 *   sb cachero <dir> <cached> <listed>
 *                                    the same for a read-only device (sbListCacheReadOnly, where
 *                                    the tree has it) */
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned int u32;
#define LOG(...)
#define memalign(a, s)  malloc(s)
#define openFile(p, f)  _open(p, (f) | _O_BINARY)
#define getFileSize(fd) _filelength(fd)
#define read            _read
#define close           _close
#define SCECdPS2CD      0x12
#define SCECdPS2DVD     0x14

#define UL_GAME_NAME_MAX       32
#define ISO_GAME_NAME_MAX      160
#define ISO_GAME_EXTENSION_MAX 4
#define GAME_STARTUP_MAX       12
enum GAME_FORMAT {
    GAME_FORMAT_USBLD = 0,
    GAME_FORMAT_OLD_ISO,
    GAME_FORMAT_ISO,
};
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
    char name[UL_GAME_NAME_MAX];
    char magic[3];
    char startup[GAME_STARTUP_MAX];
    u8 parts;
    u8 media;
    u8 unknown[4];
    u8 Byte08;
    u8 unknown2[10];
} USBExtreme_game_entry_t;

struct game_list_t;
static int scanForISO(char *path, char type, struct game_list_t **glist);
#ifdef HAVE_SB_RO
static int sbListCacheReadOnly;
#endif

#include "sb_extracted.c"

static int fake_iso_games;

static struct game_list_t *make_list(int n, int swap)
{
    struct game_list_t *head = NULL;
    int i;

    for (i = n - 1; i >= 0; i--) {
        struct game_list_t *g = calloc(1, sizeof(*g));
        int k = (swap && i < 2) ? 1 - i : i;
        sprintf(g->gameinfo.name, "Game %d", k);
        strcpy(g->gameinfo.extension, ".iso");
        g->next = head;
        head = g;
    }
    return head;
}

static int scanForISO(char *path, char type, struct game_list_t **glist)
{
    int n = (type == SCECdPS2CD) ? fake_iso_games : 0;
    struct game_list_t *l = make_list(n, 0), *t = l;

    if (t == NULL)
        return 0;
    while (t->next != NULL)
        t = t->next;
    t->next = *glist;
    *glist = l;
    return n;
}

int main(int argc, char **argv)
{
    char prefix[512];

    sprintf(prefix, "%s\\", argv[2]);
#ifdef HAVE_SBREADLIST
    if (strcmp(argv[1], "ul") == 0) {
        base_game_info_t *list = NULL;
        int fsize, count, i;

        fake_iso_games = atoi(argv[3]);
        count = sbReadList(&list, prefix, &fsize, &count);
        printf("ul.cfg size %d, %d games:", fsize, count);
        for (i = 0; i < count; i++)
            printf(" [%s]", list[i].name);
        printf("\n");
        return 0;
    }
#endif
#ifdef HAVE_UPDATE
    if (strcmp(argv[1], "cache") == 0 || strcmp(argv[1], "cachero") == 0) {
        struct game_cache_list cache;
        char gb[600];
        int listed = atoi(argv[4]);
        struct game_list_t *l = make_list(listed, argc > 5), *g;
        int i;

        cache.count = atoi(argv[3]);
        cache.games = calloc(cache.count, sizeof(base_game_info_t)); /* exactly count entries */
        for (i = 0, g = make_list(cache.count, 0); i < (int)cache.count; i++, g = g->next)
            cache.games[i] = g->gameinfo;
        sprintf(gb, "%s\\games.bin", argv[2]);
        remove(gb);
#ifdef HAVE_SB_RO
        sbListCacheReadOnly = argv[1][5] == 'r';
#endif
        updateISOGameList(argv[2], &cache, l, listed);
        printf("cache %u, listed %d%s%s: games.bin %s\n", cache.count, listed, argc > 5 ? " (reordered)" : "", argv[1][5] == 'r' ? " (read-only)" : "",
               _access(gb, 0) == 0 ? "rewritten" : "kept");
        return 0;
    }
#endif
    return 1;
}
