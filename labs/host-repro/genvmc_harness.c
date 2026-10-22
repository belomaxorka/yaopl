/* Host harness for modules/vmc/genvmc/genvmc.c mc_writecluster(): the write of
 * one cluster under the abort semaphore. Argument: 1 makes write() short. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int cluster_size;
} MCDevInfo;

static MCDevInfo devinfo = {1024};
static int genvmc_abort = 0;
static int genvmc_abort_sema = 1;
static int sema_depth = 0;
static int write_short = 0;

static void WaitSema(int sema) { sema_depth++; }
static void SignalSema(int sema) { sema_depth--; }
static long lseek(int fd, long offset, int whence) { return offset; }
static int write(int fd, const void *buf, int size) { return write_short ? size - 1 : size; }

#include "genvmc_extracted.c"

int main(int argc, char **argv)
{
    char cluster[1024];
    int r;

    write_short = argc > 1 && atoi(argv[1]);
    r = mc_writecluster(3, 0, cluster, 1);
    printf("write %s: mc_writecluster = %d, abort semaphore %s\n", write_short ? "short" : "ok", r, sema_depth == 0 ? "released" : "LEFT TAKEN");
    return 0;
}
