/* Host harness for src/gui.c guiShowAbout(): the About page shows the version
 * and the build options in two labels. diaSetLabel() is intercepted, and
 * diaExecuteDialog() prints what each label would show next to the full
 * string. VER picks the version string the Makefile makes: 1 a YAOPL nightly,
 * 2 the same of the nosio2man variant, 3 an upstream build. The build options
 * follow __RTL, IGS and PADEMU as in OPL. */
#include <stdio.h>
#include <string.h>

#if VER == 1
#define OPL_VERSION "v2.0.0-nightly.171-d24b701-yaopl"
#elif VER == 2
#define OPL_VERSION "v2.0.0-nightly.171-d24b701-yaopl-nosio2man"
#else
#define OPL_VERSION "v1.2.0-Beta-2213-abcdef1"
#endif

enum {
    ABOUT_TITLE = 1,
    ABOUT_BUILD_DETAILS,
};
struct UIItem
{
    int unused;
};
static struct UIItem diaAbout[1];
static const char *shown[3];

static int diaSetLabel(struct UIItem *ui, int id, const char *text)
{
    shown[id] = text;
    return 1;
}
static int diaExecuteDialog(struct UIItem *ui, int uiId, short inMenu, int (*updater)(int modified));

#include "about_extracted.c"

static void report(const char *name, const char *text, const char *full)
{
    printf("%s %s %zu of %zu chars: \"%s\"\n", name, strcmp(text, full) ? "CUT" : "full", strlen(text), strlen(full), text);
}

static int diaExecuteDialog(struct UIItem *ui, int uiId, short inMenu, int (*updater)(int modified))
{
    report("title", shown[ABOUT_TITLE], "Open PS2 Loader " OPL_VERSION);
    report("details", shown[ABOUT_BUILD_DETAILS], "GSM " GSM_VERSION " - UDMA+"
#ifdef __RTL
                                                  " - RTL"
#endif
#ifdef IGS
                                                  " - IGS " IGS_VERSION
#endif
#ifdef PADEMU
                                                  " - PADEMU"
#endif
    );
    return 1;
}

int main(void)
{
    guiShowAbout();
    return 0;
}
