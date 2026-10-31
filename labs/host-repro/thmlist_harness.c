/* Host harness for the theme and language lists of src/themes.c and src/lang.c: the
 * same theme or language on several devices, and a device that goes away. The device
 * code is mirrored here: at start the memory card's OPL folder is listed and the
 * settings applied (as lngInit(), thmInit() and _loadConfig()); a USB drive or an SMB
 * share adds its THM and LNG folders (bdmNeedsUpdate(), ethInitSMB()); a USB drive
 * that goes away and an SMB reconnect call what the tree has for that.
 * Mode: thmlist-t <scenario>. Each step prints the theme list, the language list and
 * what is loaded, as name@device; "!" marks an entry or a loaded file whose device is
 * gone. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG(...)
#define strcasecmp         _stricmp
#define DT_DIR             4
#define DT_REG             8
#define THM_MAX_FILES      64
#define MAX_LANGUAGE_FILES 15
#define MODE_COUNT         4
#define CONFIG_OPL         1
#define LANG_STR_COUNT     1

typedef struct
{
    char *filePath;
    char *name;
} theme_file_t;

typedef struct
{
    char *filePath;
    char *name;
} language_t;

typedef struct
{
    int dummy;
} config_set_t;

typedef struct
{
    int dummy;
} theme_t;

/* --- what the harness simulates --- */
static const char *home = "mc0:";   // the device that holds the settings
static const char *cfgTheme = NULL; // the theme and language in the settings
static const char *cfgLang = NULL;
static const char *loadedThm = NULL; // what thmLoad() and lngLoadFromFile() read last
static const char *loadedLng = NULL;

typedef struct
{
    const char *dir;
    const char *entries[4];
} sim_dir_t;

static sim_dir_t simDirs[8];
static int nSimDirs = 0;
static const char *gone[4]; // devices that went away
static int nGone = 0;

static int isGone(const char *path)
{
    int i;
    for (i = 0; i < nGone; i++)
        if (!strncmp(path, gone[i], strlen(gone[i])))
            return 1;
    return 0;
}

/* --- stubs for what the extracted code calls --- */
static theme_t *gTheme;
static config_set_t configOPLSet;
char *internalEnglish[LANG_STR_COUNT] = {"English"};

config_set_t *configGetByType(int type) { return &configOPLSet; }
char *configGetDir(void) { return (char *)home; }
int configGetStr(config_set_t *set, const char *key, const char **value)
{
    const char *v = NULL;
    if (!strcmp(key, "theme"))
        v = cfgTheme;
    if (!strcmp(key, "language_text"))
        v = cfgLang;
    if (v == NULL)
        return 0;
    *value = v;
    return 1;
}
void moduleUpdateMenu(int mode, int themeChanged, int langChanged) {}
static void thmLoad(const char *themePath) { loadedThm = themePath; }
static void thmSetColors(theme_t *theme) {}
static void bgmMute(void) {}
static void bgmUnMute(void) {}
static int fntLoadDefault(const char *path) { return 0; }
static int lngLoadFromFile(char *path, char *name)
{
    loadedLng = path;
    return 1;
}

int listDir(char *path, const char *separator, int maxElem,
            int (*readEntry)(int index, const char *path, const char *separator, const char *name, unsigned char d_type))
{
    int i, j, index = 0;
    if (isGone(path))
        return 0;
    for (i = 0; i < nSimDirs; i++) {
        if (strcmp(simDirs[i].dir, path))
            continue;
        for (j = 0; j < 4 && simDirs[i].entries[j] && index < maxElem; j++)
            index = readEntry(index, path, separator, simDirs[i].entries[j], strstr(simDirs[i].entries[j], "thm_") ? DT_DIR : DT_REG);
    }
    return index;
}

int thmSetGuiValue(int themeID, int reload);
int thmGetGuiValue(void);
int thmFindGuiID(const char *theme);
char *thmGetFilePath(int themeID);
int lngSetGuiValue(int langID);
int lngGetGuiValue(void);
int lngFindGuiID(const char *lang);
char *lngGetFilePath(int langID);

/* the state of src/themes.c */
static int guiThemeID = 0;
static int nThemes = 0;
static theme_file_t themes[THM_MAX_FILES];
static const char **guiThemesNames = NULL;
static int nGuiThemes = 0;
static int guiThemes[THM_MAX_FILES];
static int guiThemeCopy = -1;

/* the state of src/lang.c */
static int guiLangID = 0;
static char **lang_strs = internalEnglish;
static int nValidEntries = LANG_STR_COUNT;
static int nLanguages = 0;
static language_t languages[MAX_LANGUAGE_FILES];
static char **guiLangNames;
static int nGuiLanguages = 0;
static int guiLanguages[MAX_LANGUAGE_FILES];
static int guiLangCopy = -1;

#include "thmlist_extracted.c"

/* --- the device code, as the tree has it --- */
static int usbThemesLoaded, usbLanguagesLoaded;

static void start(void)
{
    int themeID = -1, langID = -1;

    lngAddLanguages("mc?:OPL", "/", -1);
    thmAddElements("mc?:OPL", "/", 0);
    if (cfgTheme)
        themeID = thmFindGuiID(cfgTheme);
    if (cfgLang)
        langID = lngFindGuiID(cfgLang);
    thmSetGuiValue(themeID, 0);
    lngSetGuiValue(langID);
}

static void usbFound(void)
{
    nGone = 0;
    if (!usbThemesLoaded && thmAddElements("mass0:THM", "/", 1) > 0)
        usbThemesLoaded = 1;
    if (!usbLanguagesLoaded && lngAddLanguages("mass0:LNG", "/", 0) > 0)
        usbLanguagesLoaded = 1;
}

static void usbGone(void)
{
    gone[nGone++] = "mass0:";
#ifdef HAVE_REMOVE
    thmRemoveElements("mass0:");
    lngRemoveLanguages("mass0:");
    usbThemesLoaded = 0;
    usbLanguagesLoaded = 0;
#endif
}

static void smbFound(void)
{
    nGone = 0;
    thmAddElements("smb0:THM", "\\", 1);
    lngAddLanguages("smb0:LNG", "\\", 2);
}

static void smbReconnect(void)
{
    gone[nGone++] = "smb0:";
#ifdef HAVE_REMOVE
    thmRemoveElements("smb0:");
    lngRemoveLanguages("smb0:");
#else
    thmReinit("smb0:");
#endif
}

static void printLoaded(const char *path, const char *prefix, const char *internal)
{
    const char *name, *colon;
    int len;

    if (path == NULL) {
        printf("%s", internal);
        return;
    }
    name = strstr(path, prefix) + strlen(prefix);
    len = (int)strcspn(name, "/\\.");
    colon = strchr(path, ':');
    printf("%.*s@%.*s%s", len, name, (int)(colon - path), path, isGone(path) ? "!" : "");
}

static void state(const char *step)
{
    int i;

    printf("%s thm[", step);
    for (i = 1; guiThemesNames[i] != NULL; i++)
        printf("%s%s%s", i > 1 ? " " : "", guiThemesNames[i], isGone(thmGetFilePath(i)) ? "!" : "");
    printf("]=");
    printLoaded(thmGetGuiValue() ? loadedThm : NULL, "thm_", "<OPL>");
    // the entry the menu marks as in use is not the theme loaded
    if (thmGetGuiValue() && strcmp(thmGetFilePath(thmGetGuiValue()), loadedThm))
        printf("(menu: %s)", guiThemesNames[thmGetGuiValue()]);
    printf(" lng[");
    for (i = 1; guiLangNames[i] != NULL; i++)
        printf("%s%s%s", i > 1 ? " " : "", guiLangNames[i], isGone(lngGetFilePath(i)) ? "!" : "");
    printf("]=");
    printLoaded(lngGetGuiValue() ? loadedLng : NULL, "lang_", "English");
    if (lngGetGuiValue() && strcmp(lngGetFilePath(lngGetGuiValue()), loadedLng))
        printf("(menu: %s)", guiLangNames[lngGetGuiValue()]);
    printf("; ");
}

static void addDir(const char *dir, const char *a, const char *b, const char *c)
{
    sim_dir_t *d = &simDirs[nSimDirs++];
    d->dir = dir;
    d->entries[0] = a;
    d->entries[1] = b;
    d->entries[2] = c;
    d->entries[3] = NULL;
}

int main(int argc, char **argv)
{
    const char *s = argc > 1 ? argv[1] : "";

    if (!strcmp(s, "dup-mc") || !strcmp(s, "dup-usb")) {
        // The same theme and language on the memory card and the USB drive
        home = !strcmp(s, "dup-mc") ? "mc0:" : "mass0:";
        cfgTheme = "A";
        cfgLang = "Russian";
        addDir("mc?:OPL", "thm_A", "lang_Russian.lng", NULL);
        addDir("mass0:THM", "thm_A", "thm_B", NULL);
        addDir("mass0:LNG", "lang_Russian.lng", NULL, NULL);
        start();
        state("start");
        usbFound();
        state("usb+");
    } else if (!strcmp(s, "remove")) {
        // The theme and language in use are only on the USB drive, which goes away and comes back
        cfgTheme = "B";
        cfgLang = "Russian";
        addDir("mc?:OPL", "thm_A", NULL, NULL);
        addDir("mass0:THM", "thm_B", NULL, NULL);
        addDir("mass0:LNG", "lang_Russian.lng", NULL, NULL);
        start();
        usbFound();
        state("usb+");
        usbGone();
        state("usb-");
        usbFound();
        state("usb+");
    } else if (!strcmp(s, "fallback")) {
        // Settings on the USB drive, the same theme and language on the memory card too; the drive goes away
        home = "mass0:";
        cfgTheme = "A";
        cfgLang = "Russian";
        addDir("mc?:OPL", "thm_A", "lang_Russian.lng", NULL);
        addDir("mass0:THM", "thm_A", NULL, NULL);
        addDir("mass0:LNG", "lang_Russian.lng", NULL, NULL);
        start();
        usbFound();
        state("usb+");
        usbGone();
        state("usb-");
    } else if (!strcmp(s, "shift")) {
        // The USB drive's theme and language are listed above the ones in use, from the SMB share; the drive goes away
        cfgTheme = "D";
        cfgLang = "German";
        addDir("mass0:THM", "thm_C", NULL, NULL);
        addDir("mass0:LNG", "lang_Russian.lng", NULL, NULL);
        addDir("smb0:THM", "thm_D", NULL, NULL);
        addDir("smb0:LNG", "lang_German.lng", NULL, NULL);
        start();
        usbFound();
        smbFound();
        state("smb+");
        usbGone();
        state("usb-");
    } else if (!strcmp(s, "smb")) {
        // The theme and language in use are on the memory card; the SMB share reconnects
        cfgTheme = "A";
        cfgLang = "Russian";
        addDir("mc?:OPL", "thm_A", "lang_Russian.lng", NULL);
        addDir("smb0:THM", "thm_C", NULL, NULL);
        addDir("smb0:LNG", "lang_German.lng", NULL, NULL);
        start();
        smbFound();
        state("smb+");
        smbReconnect();
        state("smb-");
        smbFound();
        state("smb+");
    } else {
        printf("unknown scenario\n");
        return 2;
    }
    printf("\n");
    return 0;
}
