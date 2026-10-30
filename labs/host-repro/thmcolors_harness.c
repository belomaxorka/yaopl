/* Host harness for the text colour of the internal theme: src/themes.c
 * thmSetColors(), which Display Settings -> Text Color calls without
 * reloading the theme. The main screen has a Background and an ItemsList, the
 * apps screen an ItemsList, the info screens an AttributeText each; all start
 * white (FF), the new text colour is 11. Mode:
 *   thmcolors-t colors */
#include <stdio.h>
#include <string.h>

typedef unsigned long long u64;
#define GS_SETREG_RGBA(r, g, b, a) ((u64)(r) | ((u64)(g) << 8) | ((u64)(b) << 16) | ((u64)(a) << 32))
enum {
    ELEM_TYPE_ATTRIBUTE_TEXT = 0,
    ELEM_TYPE_STATIC_TEXT,
    ELEM_TYPE_ATTRIBUTE_IMAGE,
    ELEM_TYPE_GAME_IMAGE,
    ELEM_TYPE_STATIC_IMAGE,
    ELEM_TYPE_BACKGROUND,
    ELEM_TYPE_MENU_ICON,
    ELEM_TYPE_MENU_TEXT,
    ELEM_TYPE_ITEMS_LIST,
    ELEM_TYPE_ITEM_ICON,
    ELEM_TYPE_ITEM_COVER,
    ELEM_TYPE_ITEM_TEXT,
    ELEM_TYPE_HINT_TEXT,
    ELEM_TYPE_INFO_HINT_TEXT,
    ELEM_TYPE_LOADING_ICON,
    ELEM_TYPE_BDM_INDEX,
    ELEM_TYPE_GAME_COUNT_TEXT
};

typedef struct theme_element
{
    int type;
    u64 color;
    struct theme_element *next;
} theme_element_t;
typedef struct
{
    theme_element_t *first;
    theme_element_t *last;
} theme_elems_t;
typedef struct theme
{
    unsigned char bgColor[3];
    u64 textColor, uiTextColor, selTextColor;
    theme_elems_t mainElems, infoElems, appsMainElems, appsInfoElems;
} theme_t;

static unsigned char gDefaultBgColor[3], gDefaultTextColor[3] = {0x11, 0x11, 0x11}, gDefaultUITextColor[3], gDefaultSelTextColor[3];

#include "thmcolors_extracted.c"

static unsigned int colourOf(theme_element_t *elem)
{
    return (unsigned int)(elem->color & 0xff);
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "colors") == 0) {
        theme_t theme = {0};
        theme_element_t mainBg = {ELEM_TYPE_BACKGROUND}, mainList = {ELEM_TYPE_ITEMS_LIST}, appsList = {ELEM_TYPE_ITEMS_LIST};
        theme_element_t infoText = {ELEM_TYPE_ATTRIBUTE_TEXT}, appsInfoText = {ELEM_TYPE_ATTRIBUTE_TEXT};

        mainBg.color = mainList.color = appsList.color = infoText.color = appsInfoText.color = GS_SETREG_RGBA(0xFF, 0xFF, 0xFF, 0x80);
        mainBg.next = &mainList;
        theme.mainElems.first = &mainBg;
        theme.appsMainElems.first = &appsList;
        theme.infoElems.first = &infoText;
        theme.appsInfoElems.first = &appsInfoText;
        thmSetColors(&theme);
        printf("text colour 11: main list %02X, main background %02X, apps list %02X, info text %02X, apps info text %02X\n",
               colourOf(&mainList), colourOf(&mainBg), colourOf(&appsList), colourOf(&infoText), colourOf(&appsInfoText));
        return 0;
    }
    return 2;
}
