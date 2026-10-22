/* Host harness for isValidIsoName() from src/supportbase.c. Each argument is copied into a
 * heap block of exactly strlen + 1 bytes, as a short directory entry name would be. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
/* Instrumented here, so AddressSanitizer sees every byte strcasecmp reads (_stricmp is not). */
static int strcasecmp(const char *a, const char *b)
{
    for (; tolower((unsigned char)*a) == tolower((unsigned char)*b); a++, b++)
        if (*a == '\0')
            return 0;
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}
enum GAME_FORMAT {
    GAME_FORMAT_USBLD = 0,
    GAME_FORMAT_OLD_ISO,
    GAME_FORMAT_ISO,
};

#include "name_extracted.c"

int main(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        char *name = _strdup(argv[i]);
        int len = -1;
        int format = isValidIsoName(name, &len);

        printf("%-24s format=%d name_len=%d\n", argv[i], format, len);
        free(name);
    }
    return 0;
}
