/* Host harness for bdmFindPartition() (src/bdmsupport.c), the probe the settings save uses to
 * find a writable USB device:
 *   bdmprobe <dir>
 * "massN:" maps to <dir>\massN\. Probes for conf_opl.cfg with write = 1 and prints its size
 * before and after. */
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_BDM_DEVICES 5
static char gBDMPrefix[32];
static char root[512];

static int harness_open(const char *path, int flags, ...)
{
    char local[1024];
    int unit = path[4] - '0'; /* "massN:..." */
    sprintf(local, "%s\\mass%d\\%s", root, unit, strchr(path, ':') + 1);
    return _open(local, flags | _O_BINARY, _S_IREAD | _S_IWRITE);
}
#define open  harness_open
#define close _close

#include "bdmprobe_extracted.c"

int main(int argc, char **argv)
{
    char target[256], cfg[600];
    struct _stat st;
    long before;

    strcpy(root, argv[1]);
    sprintf(cfg, "%s\\mass0\\conf_opl.cfg", root);
    before = _stat(cfg, &st) == 0 ? (long)st.st_size : -1;
    int r = bdmFindPartition(target, "conf_opl.cfg", 1);
    _stat(cfg, &st);
    printf("probe = %d (%s); conf_opl.cfg %ld bytes before, %ld after\n", r, target, before, (long)st.st_size);
    return 0;
}
