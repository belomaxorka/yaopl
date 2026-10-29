/* Host harness for src/fntsys.c fntFitString(), src/themes.c
 * drawAttributeText() and src/gui.c guiDrawMsgText(): the word wrapping of theme
 * text and of message boxes. Glyphs are 8 pixels per UTF-8 character and the
 * screen is not scaled. Modes:
 *   text-t fit <width> <text>    wrap an exact-size heap copy of <text>
 *   text-t cfgval <width>        wrap a 255-character value inside a config_value_t
 *   text-t attr <width> <text>   draw an AttributeText whose config value is <text>
 *   text-t msgbox <text>         draw <text> as guiMsgBox() does and check that it fits the box
 *   text-t msgboxfile <file>     the same with the (UTF-8) text of <file> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;
typedef unsigned long long u64;
#define LOG(...)
#define SIZING_NONE    -1
#define SIZING_CLIP    0
#define SIZING_WRAP    1
#define DISPLAY_ALWAYS 0
#define DISPLAY_NEVER  2
#define _STR_SIZE      1

#define CONFIG_KEY_NAME_LEN  32
#define CONFIG_KEY_VALUE_LEN 256
struct config_value_t
{
    char key[CONFIG_KEY_NAME_LEN];
    char val[CONFIG_KEY_VALUE_LEN];
    struct config_value_t *next;
};
typedef struct
{
    int uid;
    struct config_value_t value;
    char tail[16]; /* ends a scan that ran past the value and the next pointer */
} config_set_t;

struct menu_list;
struct submenu_list;
typedef struct theme_element
{
    int type;
    int posX;
    int posY;
    short aligned;
    int width;
    int height;
    short scaled;
    u64 color;
    int font;
    void *extended;
} theme_element_t;

typedef struct
{
    char *value;
    int sizingMode;
    char *alias;
    int displayMode;
    u32 currentConfigId;
    char *currentValue;
    char *wrappedValue; /* the private copy a fix may add */
} mutable_text_t;

/* src/gui.c: the message box is drawn between x 50 .. screenWidth - 50 and y 75 .. 410 */
#define MENU_ITEM_HEIGHT 19
#define ALIGN_CENTER     10
typedef struct
{
    int fonts[1];
    int usedHeight;
    u64 textColor;
} theme_t;
static theme_t theme = {{0}, 480, 0};
static theme_t *gTheme = &theme;
static int screenWidth = 640;

static char rendered[1024];
#define MAX_DRAWN 64
static struct
{
    int x, y;
    char text[4096];
} drawn[MAX_DRAWN];
static int drawnCount;

int fntCalcDimensions(int id, const char *str)
{
    int w = 0;
    for (; *str; str++)
        if ((*str & 0xC0) != 0x80) /* not a UTF-8 continuation byte */
            w += 8;
    return w;
}
int rmScaleX(int x) { return x; }
int fntRenderString(int id, int x, int y, short aligned, size_t width, size_t height, const char *string, u64 colour)
{
    strncpy(rendered, string, sizeof(rendered) - 1);
    if (drawnCount < MAX_DRAWN) {
        drawn[drawnCount].x = x;
        drawn[drawnCount].y = y;
        strncpy(drawn[drawnCount].text, string, sizeof(drawn[0].text) - 1);
    }
    drawnCount++;
    return 0;
}
char *_l(unsigned int id) { return id == _STR_SIZE ? "Size" : "Description"; }
void fntFitString(int id, char *string, size_t width);
int configGetStr(config_set_t *configSet, const char *key, const char **value)
{
    if (strcmp(key, configSet->value.key) != 0)
        return 0;
    *value = configSet->value.val;
    return 1;
}

#include "text_extracted.c"

/* print up to n bytes of s, line breaks as '|' */
static void show(const char *label, const char *s, size_t n)
{
    size_t i;

    printf("%s \"", label);
    for (i = 0; i < n && s[i]; i++)
        putchar(s[i] == '\n' ? '|' : s[i]);
    printf("\"\n");
}

/* draw text as the message box does, then report the lines and whether they fit between the rules */
static void msgbox(const char *text)
{
    char joined[4096];
    int i, w, widest = 0, fits = 1;
    size_t len = 0;

    drawnCount = 0;
#ifdef HAVE_MSGTEXT
    guiDrawMsgText(text);
#else
    /* upstream: one centred line, as guiMsgBox() draws it */
    fntRenderString(gTheme->fonts[0], screenWidth >> 1, gTheme->usedHeight >> 1, ALIGN_CENTER, 0, 0, text, gTheme->textColor);
#endif
    joined[0] = '\0';
    for (i = 0; i < drawnCount && i < MAX_DRAWN; i++) {
        w = fntCalcDimensions(0, drawn[i].text);
        if (w > widest)
            widest = w;
        if (drawn[i].x - w / 2 < 50 || drawn[i].x + w / 2 > screenWidth - 50 || drawn[i].y - 10 < 75 || drawn[i].y + 10 > 410)
            fits = 0;
        len += strlen(drawn[i].text) + 1;
        if (len < sizeof(joined)) {
            if (i > 0)
                strcat(joined, " ");
            strcat(joined, drawn[i].text);
        }
    }
    printf("lines %d, y %d..%d, widest %d px of a %d px box: %s\n", drawnCount, drawn[0].y, drawn[(drawnCount < MAX_DRAWN ? drawnCount : MAX_DRAWN) - 1].y,
           widest, screenWidth - 100, fits ? "fits" : "OVERFLOWS");
    if (strcmp(joined, text) == 0)
        printf("text drawn whole, split on spaces only\n");
    else if (strncmp(joined, text, strlen(joined)) == 0)
        printf("text cut after %u of %u bytes, split on spaces only\n", (unsigned)strlen(joined), (unsigned)strlen(text));
    else
        printf("text CHANGED\n");
}

int main(int argc, char **argv)
{
    int width = argc > 2 ? atoi(argv[2]) : 0;

    if (argc == 4 && strcmp(argv[1], "fit") == 0) {
        size_t len = strlen(argv[3]);
        char *copy = malloc(len + 1); /* exactly the text, as a StaticText value */
        memcpy(copy, argv[3], len + 1);
        fntFitString(0, copy, width);
        show("wrapped:", copy, len);
        printf("terminator %s\n", copy[len] == '\0' ? "kept" : "OVERWRITTEN");
        free(copy);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "cfgval") == 0) {
        config_set_t cfg;
        memset(&cfg, 0, sizeof(cfg));
        strcpy(cfg.value.key, "Description");
        memset(cfg.value.val, 'A', CONFIG_KEY_VALUE_LEN - 1); /* the longest value the config reader keeps */
        cfg.value.next = (struct config_value_t *)(size_t)0x4141414141414141ULL;
        strcpy(cfg.tail, "BBBBBBB");
        fntFitString(0, cfg.value.val, width);
        printf("255-character value: terminator %s, next pointer %s, bytes after it %s\n",
               cfg.value.val[CONFIG_KEY_VALUE_LEN - 1] == '\0' ? "kept" : "OVERWRITTEN",
               cfg.value.next == (struct config_value_t *)(size_t)0x4141414141414141ULL ? "intact" : "CORRUPTED",
               strcmp(cfg.tail, "BBBBBBB") == 0 ? "intact" : "CORRUPTED");
        return 0;
    }
    if (argc == 4 && strcmp(argv[1], "attr") == 0) {
        config_set_t cfg;
        theme_element_t elem;
        mutable_text_t text;

        memset(&cfg, 0, sizeof(cfg));
        cfg.uid = 1;
        strcpy(cfg.value.key, "Description");
        strncpy(cfg.value.val, argv[3], CONFIG_KEY_VALUE_LEN - 1);
        memset(&elem, 0, sizeof(elem));
        elem.width = width;
        elem.height = 100;
        elem.extended = &text;
        memset(&text, 0, sizeof(text));
        text.value = _strdup("Description");
        text.alias = _strdup("Description:\n");
        text.sizingMode = SIZING_WRAP;
        text.displayMode = DISPLAY_ALWAYS;
        drawAttributeText(NULL, NULL, &cfg, &elem);
        show("config value after drawing:", cfg.value.val, CONFIG_KEY_VALUE_LEN);
        show("rendered text:", rendered, sizeof(rendered));
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "msgbox") == 0) {
        msgbox(argv[2]);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "msgboxfile") == 0) {
        static char text[4096];
        FILE *f = fopen(argv[2], "rb");
        size_t n;

        if (f == NULL)
            return 3;
        n = fread(text, 1, sizeof(text) - 1, f);
        fclose(f);
        text[n] = '\0';
        msgbox(text);
        return 0;
    }
    return 2;
}
