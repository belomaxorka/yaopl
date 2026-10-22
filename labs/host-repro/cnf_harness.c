/* Host harness for ps2cnfGetBootFile() from src/ps2cnf.c. */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define CNF_PATH_LEN_MAX 64
#define LOG(...)

#include "cnf_extracted.c"

int main(int argc, char **argv)
{
    char bootfile[CNF_PATH_LEN_MAX + 1] = "";
    int r = ps2cnfGetBootFile(argv[1], bootfile);

    printf("ps2cnfGetBootFile = %d, bootfile = \"%s\"\n", r, bootfile);
    return 0;
}
