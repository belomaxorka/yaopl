/* Host harness for the folder that src/util.c openFile() (checkFile()) makes for a file it creates:
 *   openfile <dir> <path> [<existing folder> ...]
 * "dev:rest" maps to <dir>\dev\rest ('/' and '\' both separate folders, as on the PS2's file systems and
 * smbman). The listed folders are made first, then the tree's openFile() creates <path>. Prints the
 * folders it made and whether the open worked. */
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define LOG(...)

static char root[512];

static void localPath(char *local, const char *path)
{
    const char *colon = strchr(path, ':');
    char *p;
    sprintf(local, "%s\\%.*s\\%s", root, (int)(colon - path), path, (colon[1] == '/' || colon[1] == '\\') ? colon + 2 : colon + 1);
    for (p = local; *p; p++)
        if (*p == '/')
            *p = '\\';
}

typedef struct
{
    int dummy;
} DIR;
static DIR fakeDir;
static int mcID = '0';
static int checkMC(void) { return mcID; }
static DIR *opendir(const char *path)
{
    char local[1024];
    struct _stat st;
    localPath(local, path);
    return _stat(local, &st) == 0 ? &fakeDir : NULL;
}
static int closedir(DIR *dir) { return 0; }

static char made[512];
static int harness_mkdir(const char *path, int mode)
{
    char local[1024];
    int result;
    localPath(local, path);
    result = _mkdir(local);
    sprintf(made + strlen(made), "%s%s%s", made[0] ? ", " : "", path, result == 0 ? "" : " (failed)");
    return result;
}
static int harness_open(const char *path, int flags, ...)
{
    char local[1024];
    localPath(local, path);
    return _open(local, flags | _O_BINARY, _S_IREAD | _S_IWRITE);
}

#define open  harness_open
#define mkdir harness_mkdir

#include "openfile_extracted.c"

#undef open
#undef mkdir

int main(int argc, char **argv)
{
    char path[256], local[1024];
    int i, fd;

    strcpy(root, argv[1]);
    for (i = 3; i < argc; i++) {
        localPath(local, argv[i]);
        _mkdir(local);
    }

    strcpy(path, argv[2]);
    fd = openFile(path, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd >= 0)
        _close(fd);
    printf("made [%s], open %s\n", made, fd >= 0 ? "ok" : "FAILED");
    return 0;
}
