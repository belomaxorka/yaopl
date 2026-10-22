/* Host harness for src/sound.c bgmLoad(): the path of the default background
 * music comes from the user's settings. Argument: the default_bgm_path value.
 * snprintf() is intercepted, so a format string taken from that setting is
 * reported instead of being executed. */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG(...)

typedef struct
{
    int unused;
} OggVorbis_File;
typedef struct
{
    int unused;
} ov_callbacks;
static const ov_callbacks OV_CALLBACKS_DEFAULT = {0};
static int ov_open_callbacks(void *f, OggVorbis_File *vf, const char *initial, long ibytes, ov_callbacks cb) { return -1; }
static int thmGetGuiValue(void) { return 0; }
static char *thmGetFilePath(int id) { return "mass:/THM/thm_x/"; }
char gDefaultBGMPath[256];
static OggVorbis_File *vorbisFile;

static int harness_snprintf(char *s, size_t n, const char *fmt, ...)
{
    va_list ap;
    int r;

    if (fmt == gDefaultBGMPath) {
        printf("format string = the setting itself (user data)\n");
        strncpy(s, fmt, n - 1);
        s[n - 1] = '\0';
        return (int)strlen(s);
    }
    printf("format string = \"%s\"\n", fmt);
    va_start(ap, fmt);
    r = vsnprintf(s, n, fmt, ap);
    va_end(ap);
    return r;
}
#define snprintf harness_snprintf

#include "bgm_extracted.c"

int main(int argc, char **argv)
{
    strncpy(gDefaultBGMPath, argc > 1 ? argv[1] : "mass:/bgm.ogg", sizeof(gDefaultBGMPath) - 1);
    printf("bgmLoad = %d\n", bgmLoad());
    return 0;
}
