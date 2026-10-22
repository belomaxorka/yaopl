/* Host harness for src/lang.c lngReadEntry(): the directory scan callback that
 * registers a language file. Each argument is a file name found in mc0:/OPL. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG(...)
#define MAX_LANGUAGE_FILES 15
#define DT_DIR             4

typedef struct
{
    char *filePath;
    char *name;
} language_t;

static int nLanguages = 0;
static language_t languages[MAX_LANGUAGE_FILES];

#include "lang_extracted.c"

int main(int argc, char **argv)
{
    int i, index;

    for (i = 1; i < argc; i++) {
        memset(languages, 0, sizeof(languages));
        index = lngReadEntry(0, "mc0:/OPL", "/", argv[i], 0);
        if (index == 1) {
            printf("%s: language \"%s\" from %s\n", argv[i], languages[0].name, languages[0].filePath);
            free(languages[0].name);
            free(languages[0].filePath);
        } else
            printf("%s: skipped\n", argv[i]);
    }
    return 0;
}
