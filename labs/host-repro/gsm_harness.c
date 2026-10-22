/* Host harness for src/gsm.c PrepareGSM(): the per-game $GSMVMode indexes the
 * table of video modes. Argument: the $GSMVMode value. The GS mode constants of
 * gsKit are replaced by distinct numbers; only the table lookup is under test. */
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef short s16;
typedef unsigned int u32;
typedef unsigned long long u64;
#define LOG(...)
#define open     _open
#define read     _read
#define close    _close
#define O_RDONLY _O_RDONLY

#define GS_INTERLACED       1
#define GS_NONINTERLACED    0
#define GS_FIELD            0
#define GS_FRAME            1
#define GS_MODE_NTSC        0x02
#define GS_MODE_PAL         0x03
#define GS_MODE_DTV_480P    0x50
#define GS_MODE_DTV_576P    0x53
#define GS_MODE_DTV_720P    0x52
#define GS_MODE_DTV_1080I   0x51
#define GS_MODE_VGA_640_60  0x1A
#define GS_MODE_VGA_640_72  0x1B
#define GS_MODE_VGA_640_75  0x1C
#define GS_MODE_VGA_640_85  0x1D
#define GS_MODE_VGA_800_56  0x2A
#define GS_MODE_VGA_800_60  0x2B
#define GS_MODE_VGA_800_72  0x2C
#define GS_MODE_VGA_800_75  0x2D
#define GS_MODE_VGA_800_85  0x2E
#define GS_MODE_VGA_1024_60 0x3B
#define GS_MODE_VGA_1024_70 0x3C
#define GS_MODE_VGA_1024_75 0x3D
#define GS_MODE_VGA_1024_85 0x3E
#define GS_MODE_VGA_1280_60 0x4A
#define GS_MODE_VGA_1280_75 0x4B

static int gGSMVMode;
static int gGSMXOffset;
static int gGSMYOffset;
static int gGSMFIELDFix;

#include "gsm_extracted.c"

int main(int argc, char **argv)
{
    char cmdline[256];
    struct GsmConfig_t config;

    gGSMVMode = atoi(argv[1]);
    memset(&config, 0, sizeof(config));
    PrepareGSM(cmdline, &config);
    printf("$GSMVMode=%s: mode used %d, interlace %d, ffmd %d, display %016llX, cmdline \"%s\"\n", argv[1], config.mode, config.interlace, config.ffmd, config.display, cmdline);
    return 0;
}
