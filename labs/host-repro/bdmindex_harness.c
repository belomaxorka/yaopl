/* Host harness for the BdmIndex theme element of src/themes.c: drawBDMIndex()
 * on a menu of device pages. Prints, for every visible page, the picture the
 * element draws ("-" for none). The pages are the five mass-device slots
 * (modes 0-4) and SMB, HDD, APPS and MMCE (modes 5-8), in menu order.
 * Modes: bdmindex-t <scenario> [fixed], where fixed is a theme that has its own
 * five-dot pictures only. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ETH_MODE        5
#define MAX_BDM_DEVICES 5

typedef struct
{
    int uid;
} config_set_t;
typedef struct
{
    void *Mem;
} GSTEXTURE;
typedef struct item_list
{
    int mode;
} item_list_t;
struct menu_item
{
    int visible;
    void *userdata;
};
struct menu_list
{
    struct menu_item *item;
    struct menu_list *prev;
    struct menu_list *next;
};
struct submenu_list;
struct theme_element
{
    int posX, posY;
    short aligned;
    int width, height;
    short scaled;
};
typedef struct theme
{
    int bdmIndexFixed;
} theme_t;

static theme_t theme;
static theme_t *gTheme = &theme;
static int gDefaultCol;
static char drawn[64];
static char texName[64];
static GSTEXTURE texture = {&texture};

int texLookupInternalTexId(const char *name)
{
    strcpy(texName, name);
    return 0;
}
GSTEXTURE *thmGetTexture(unsigned int id) { return &texture; }
void rmDrawPixmap(GSTEXTURE *txt, int x, int y, short aligned, int width, int height, short scaled, int col) { strcpy(drawn, texName); }

#include "bdmindex_extracted.c"

static const struct
{
    const char *name;
    int modes[9];
    int visible[9];
} scenarios[] = {
    {"one", {0, 1, 2, 3, 4, 5, 6, 7, 8}, {1, 0, 0, 0, 0, 1, 1, 1, 1}},
    {"two", {0, 1, 2, 3, 4, 5, 6, 7, 8}, {1, 1, 0, 0, 0, 0, 1, 1, 1}},
    {"gap", {0, 1, 2, 3, 4, 5, 6, 7, 8}, {1, 0, 1, 0, 0, 1, 1, 1, 1}},
    {"slot2", {0, 1, 2, 3, 4, 5, 6, 7, 8}, {0, 0, 1, 0, 0, 1, 1, 1, 1}},
    {"three", {0, 1, 2, 3, 4, 5, 6, 7, 8}, {1, 1, 0, 1, 0, 1, 1, 1, 1}},
    {"four", {0, 1, 2, 3, 4, 5, 6, 7, 8}, {1, 1, 1, 1, 0, 1, 1, 1, 1}},
    {"five", {0, 1, 2, 3, 4, 5, 6, 7, 8}, {1, 1, 1, 1, 1, 1, 1, 1, 1}},
    /* BDM switched on after the other pages were added: its pages come last */
    {"late", {5, 6, 7, 8, 0, 1, 2, 3, 4}, {1, 1, 1, 1, 1, 1, 0, 0, 0}},
};

int main(int argc, char **argv)
{
    item_list_t lists[9];
    struct menu_item items[9];
    struct menu_list menu[9];

    for (size_t s = 0; s < sizeof(scenarios) / sizeof(scenarios[0]); s++) {
        if (argc < 2 || strcmp(argv[1], scenarios[s].name) != 0)
            continue;

        theme.bdmIndexFixed = argc > 2 && strcmp(argv[2], "fixed") == 0;
        for (int i = 0; i < 9; i++) {
            lists[i].mode = scenarios[s].modes[i];
            items[i].visible = scenarios[s].visible[i];
            items[i].userdata = &lists[i];
            menu[i].item = &items[i];
            menu[i].prev = i > 0 ? &menu[i - 1] : NULL;
            menu[i].next = i < 8 ? &menu[i + 1] : NULL;
        }

        for (int i = 0; i < 9; i++) {
            if (!items[i].visible)
                continue;
            struct theme_element elem = {0};
            drawn[0] = '\0';
            drawBDMIndex(&menu[i], NULL, NULL, &elem);
            printf("%d:%s ", lists[i].mode, drawn[0] ? drawn : "-");
        }
        printf("\n");
        return 0;
    }

    printf("unknown scenario\n");
    return 1;
}
