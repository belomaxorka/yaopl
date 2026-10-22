/* Host harness for USBA_crc32() (src/system.c or pc/iso2opl). Prints the CRC of each
 * argument given as hex bytes ("" for the empty name). MSVC's char is signed, as on the EE. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;
static unsigned int crctab[0x400];

#include "crc_extracted.c"

int main(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        char name[64] = {0};
        size_t n = strlen(argv[i]) / 2, k;

        for (k = 0; k < n; k++)
            name[k] = (char)strtol((char[]) {argv[i][2 * k], argv[i][2 * k + 1], 0}, NULL, 16);
        printf("%-24s %08X\n", argv[i][0] ? argv[i] : "(empty)", USBA_crc32(name));
    }
    return 0;
}
