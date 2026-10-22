/* Host harness for src/OSDHistory.c GetBootFilename() and
 * AddHistoryRecordUsingFullPath(): the launch path is cut to the file name that
 * goes into the OSD history. Each argument is a boot path. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int AddHistoryRecord(const char *name)
{
    printf("record \"%s\" (%u characters)\n", name, (unsigned int)strlen(name));
    return 0;
}

#include "history_extracted.c"

int main(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        printf("%s: ", argv[i]);
        fflush(stdout);
        AddHistoryRecordUsingFullPath(argv[i]);
    }
    return 0;
}
