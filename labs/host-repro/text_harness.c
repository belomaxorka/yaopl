/* Host harness for src/fntsys.c fntFitString() and src/themes.c
 * drawAttributeText(): the word wrapping of theme text. Glyphs are 8 pixels
 * per byte and the screen is not scaled. Modes:
 *   text-t fit <width> <text>    wrap an exact-size heap copy of <text>
 *   text-t cfgval <width>        wrap a 255-character value inside a config_value_t
 *   text-t attr <width> <text>   draw an AttributeText whose config value is <text> */
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

static char rendered[1024];

int fntCalcDimensions(int id, const char *str) { return 8 * (int)strlen(str); }
int rmScaleX(int x) { return x; }
int fntRenderString(int id, int x, int y, short aligned, size_t width, size_t height, const char *string, u64 colour)
{
    strncpy(rendered, string, sizeof(rendered) - 1);
    return 0;
}
char *_l(unsigned int id) { return id == _STR_SIZE ? "Size" : "Description"; }
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
    return 2;
}
