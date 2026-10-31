/* Host harness for where OPL's settings are read and saved: src/opl.c tryAlternateDevice(),
 * trySaveAlternateDevice() and the checkLoadConfig*() / trySaveConfig*() functions they call (with
 * getBootDevice(), tryBootDevice(), trySetBootHome() where the tree has them), src/bdmsupport.c
 * bdmFindPartition() and src/config.c configGetDeviceRank(). The start of _loadConfig() and of
 * _saveConfig() is mirrored here.
 * Mode: cfgloc-t <scenario>. A scenario sets the directory OPL was started from and the devices, then:
 * start (read the settings), save (with every device of the scenario started, as after the user enabled
 * it), start again. Prints where the settings were read ("-": nowhere, defaults), where a first save goes
 * ("home"), which device's copy of a theme on every device is used, where the save went and where the
 * settings are read at the next start. */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG(...)
#define MAX_BDM_DEVICES              5
#define MODE_FLAG_READ_ONLY          0x08
#define CONFIG_OPL                   0x01
#define CONFIG_ALL                   0x1F
#define START_MODE_AUTO              2
#define MMCE_SLOT_1                  0
#define MMCE_SLOT_2                  1
#define USBMASS_IOCTL_GET_DRIVERNAME 0x1
#define CONFIG_OPL_BDM_MODE          "usb_mode"
#define CONFIG_OPL_ENABLE_BDMHDD     "enable_bdm_hdd"
#define CONFIG_OPL_HDD_MODE          "hdd_mode"
#define CONFIG_OPL_MMCE_MODE         "mmce_mode"

typedef unsigned int u32;
typedef struct
{
    int dummy;
} config_set_t;
typedef struct
{
    int dummy;
} DIR;
typedef struct
{
    int flags;
} bdm_list_t;

/* --- the simulated console --- */
enum { DEV_MC,
       DEV_MASS0,
       DEV_MASS1,
       DEV_APA,
       DEV_MMCE0,
       DEV_MMCE1,
       DEV_COUNT };
static const char *devName[DEV_COUNT] = {"mc0", "mass0", "mass1", "pfs0", "mmce0", "mmce1"};

typedef struct
{
    int present;
    const char *driver; // BDM: usb, ata, sdc (MX4SIO), sd (iLink), udp (UDPBD)
    int hasCfg;         // conf_opl.cfg is there
} sim_dev_t;

static sim_dev_t dev[DEV_COUNT];
static const char *cwd = "";
static int savePhase;  // the menu started every device of the scenario
static int hddLoaded;  // atad etc.: the APA HDD and the BDM HDD can be read
static int mmceLoaded; // mmceman
static char home[64];  // where the settings are read and written: configInit(), configSetMove()

static int devOf(const char *path)
{
    int i;
    if (!strncmp(path, "mc", 2))
        return DEV_MC;
    for (i = 1; i < DEV_COUNT; i++)
        if (!strncmp(path, devName[i], strlen(devName[i])))
            return i;
    return -1;
}

static int visible(int d)
{
    if (d < 0 || !dev[d].present)
        return 0;
    if (savePhase || d == DEV_MC)
        return 1;
    if (d == DEV_APA)
        return hddLoaded;
    if (d == DEV_MMCE0 || d == DEV_MMCE1)
        return mmceLoaded;
    // At start only the USB driver is loaded, and the ATA one by the settings search; the drivers of
    // MX4SIO, iLink and UDPBD are loaded later, as the settings ask
    if (!strcmp(dev[d].driver, "usb"))
        return 1;
    if (!strcmp(dev[d].driver, "ata"))
        return hddLoaded;
    return 0;
}

/* --- stubs --- */
static char gBDMPrefix[32] = "";
static char *gHDDPrefix = "pfs0:";
static char *gBaseMCDir = "mc?:OPL";
static int gEnableBdmHDD;
static int showCfgPopup;
static bdm_list_t bdmDeviceList[MAX_BDM_DEVICES];
static config_set_t configOPLSet;

static char *getcwd(char *buf, size_t size)
{
    if (strlen(cwd) + 1 > size) {
        // ps2sdk returns NULL with ERANGE and leaves the buffer as it was: an uninitialised array in
        // OPL. Filled here so every run gives the same result.
        memset(buf, 'X', size);
        return NULL;
    }
    strcpy(buf, cwd);
    return buf;
}

static void configInit(char *prefix) { snprintf(home, sizeof(home), "%s", prefix ? prefix : gBaseMCDir); }
static void configSetMove(char *prefix) { configInit(prefix); }
static void configEnd(void) {}
static void configPrepareNotifications(char *prefix) {}
static config_set_t *configGetByType(int type) { return &configOPLSet; }
static int configSetInt(config_set_t *set, const char *key, const int value) { return 1; }
static char *configGetDir(void)
{
    static char d[16];
    int i = devOf(home);
    snprintf(d, sizeof(d), "%s:", i >= 0 ? devName[i] : "?");
    return d;
}
static int configReadMulti(int types)
{
    int d = devOf(home);
    return (visible(d) && dev[d].hasCfg) ? types : 0;
}
static int configWriteMulti(int types)
{
    int d = devOf(home);
    if (!visible(d))
        return 0;
    dev[d].hasCfg = 1;
    return 1;
}
static int sysCheckMC(void) { return dev[DEV_MC].present ? 0 : -1; }
static int hddLoadModules(void)
{
    hddLoaded = 1;
    return 0;
}
static void hddLoadSupportModules(void) {}
static int hddCheck(void) { return visible(DEV_APA) ? 0 : -1; }
static int hddWaits; // an HDD is there but not through BDM: bdmHDDIsPresent() waits out its timeout
static int bdmHDDIsPresent(u32 timeoutMs)
{
    int i;
    for (i = DEV_MASS0; i <= DEV_MASS1; i++)
        if (dev[i].present && !strcmp(dev[i].driver, "ata"))
            return 1;
    if (dev[DEV_APA].present)
        hddWaits++;
    return 0;
}
static void mmceLoadModules(void) { mmceLoaded = 1; }
static DIR fakeDir;
static DIR *opendir(const char *path) { return visible(devOf(path)) ? &fakeDir : NULL; }
static int closedir(DIR *d) { return 0; }
static int harness_open(const char *path, int flags, ...)
{
    int d = devOf(path);
    if (!visible(d))
        return -1;
    if ((flags & 3) == O_RDONLY)
        return dev[d].hasCfg ? 3 : -1;
    return 3; // a write probe: the file is written later
}
static int harness_close(int fd) { return 0; }
#define open  harness_open
#define close harness_close
static int fileXioDopen(const char *path)
{
    return visible(devOf(path)) ? devOf(path) : -1;
}
static int fileXioIoctl2(int fd, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    strncpy((char *)buf, dev[fd].driver, buflen);
    return 0;
}
static int fileXioDclose(int fd) { return 0; }

#include "cfgloc_extracted.c"

/* --- the start of _loadConfig() and _saveConfig(), as the tree has them --- */
static int loadSettings(void)
{
    int result = 0;

    configInit(NULL);
#ifdef HAVE_BOOT_FIRST
    result = tryBootDevice(CONFIG_ALL);
    if (!(result & CONFIG_OPL))
#endif
        result = configReadMulti(CONFIG_ALL);
    if (!(result & CONFIG_OPL))
        result = tryAlternateDevice(CONFIG_ALL);
    return result;
}

static const char *where(void)
{
    int d = devOf(home);
    return d >= 0 ? devName[d] : "?";
}

static void run(void)
{
    int i, r, best = -1;

    r = loadSettings();
    printf("read=%s home=%s", (r & CONFIG_OPL) ? where() : "-", where());
    if (hddWaits)
        printf(" (%d x 5 s HDD wait)", hddWaits);

#ifdef HAVE_RANK
    // A theme with the same name on every device of the scenario: the copy that is used
    {
        static const char *thm[DEV_COUNT] = {"mc0:OPL/thm_A/", "mass0:THM/thm_A/", "mass1:THM/thm_A/", "pfs0:THM/thm_A/", "mmce0:/THM/thm_A/", "mmce1:/THM/thm_A/"};
        for (i = 0; i < DEV_COUNT; i++)
            if (dev[i].present && (best < 0 || configGetDeviceRank(thm[i]) < configGetDeviceRank(thm[best])))
                best = i;
    }
    printf(" theme=%s", best >= 0 ? devName[best] : "-");
#endif

    // Save from the menu: every device of the scenario is started now
    savePhase = 1;
    for (i = 0; i < MAX_BDM_DEVICES; i++)
        bdmDeviceList[i].flags = 0;
    r = configWriteMulti(CONFIG_ALL);
    if (r == 0)
        r = trySaveAlternateDevice(CONFIG_ALL);
    printf(" | save=%s", r > 0 ? where() : "error");

    // The next start
    savePhase = hddLoaded = mmceLoaded = 0;
    r = loadSettings();
    printf(" | next=%s\n", (r & CONFIG_OPL) ? where() : "-");
}

static void add(int d, const char *driver, int hasCfg)
{
    dev[d].present = 1;
    dev[d].driver = driver;
    dev[d].hasCfg = hasCfg;
}

int main(int argc, char **argv)
{
    const char *s = argc > 1 ? argv[1] : "";

    if (!strcmp(s, "usb")) {
        // Started from mass:/APPS/, settings on the memory card and on the USB drive
        cwd = "mass:/APPS/";
        add(DEV_MC, "mc", 1);
        add(DEV_MASS0, "usb", 1);
    } else if (!strcmp(s, "usb-root")) {
        // Started from mass:/ (short enough for the old 8-byte buffer), same devices
        cwd = "mass:/";
        add(DEV_MC, "mc", 1);
        add(DEV_MASS0, "usb", 1);
    } else if (!strcmp(s, "usb-first")) {
        // Started from mass:/APPS/, no settings anywhere yet
        cwd = "mass:/APPS/";
        add(DEV_MC, "mc", 0);
        add(DEV_MASS0, "usb", 0);
    } else if (!strcmp(s, "usb-apa")) {
        // Started from mass:/APPS/ on a console with an APA HDD, settings on the memory card only
        cwd = "mass:/APPS/";
        add(DEV_MC, "mc", 1);
        add(DEV_MASS0, "usb", 0);
        add(DEV_APA, "apa", 0);
    } else if (!strcmp(s, "mc")) {
        // Started from the memory card, settings on the memory card and on the USB drive
        cwd = "mc0:/APPS/";
        add(DEV_MC, "mc", 1);
        add(DEV_MASS0, "usb", 1);
    } else if (!strcmp(s, "hdd")) {
        // Started from the APA HDD (FMCB / HDD-OSD path), settings on the HDD and on the USB drive
        cwd = "hdd0:__common:pfs:/OPL/";
        add(DEV_MC, "mc", 0);
        add(DEV_MASS0, "usb", 1);
        add(DEV_APA, "apa", 1);
    } else if (!strcmp(s, "hdd-pfs")) {
        // The same, started by PS2BBL (pfs:/...)
        cwd = "pfs:/OPL/";
        add(DEV_MC, "mc", 0);
        add(DEV_MASS0, "usb", 1);
        add(DEV_APA, "apa", 1);
    } else if (!strcmp(s, "mmce")) {
        // Started from an MMCE card in slot 1, settings on its SD card and on its virtual memory card
        cwd = "mmce0:/APPS/";
        add(DEV_MC, "mc", 1);
        add(DEV_MMCE0, "mmce", 1);
    } else if (!strcmp(s, "mmce-first")) {
        // Started from an MMCE card in slot 1, no settings anywhere yet
        cwd = "mmce0:/APPS/";
        add(DEV_MC, "mc", 0);
        add(DEV_MMCE0, "mmce", 0);
    } else if (!strcmp(s, "mx4sio")) {
        // No memory card, no USB drive: games on an MX4SIO card, enabled in the menu
        cwd = "mass:/APPS/";
        add(DEV_MASS0, "sdc", 0);
    } else if (!strcmp(s, "udpbd")) {
        // No memory card, no USB drive: games on a writable UDPBD server, enabled in the menu
        cwd = "mass:/APPS/";
        add(DEV_MASS0, "udp", 0);
    } else {
        printf("unknown scenario\n");
        return 2;
    }
    run();
    return 0;
}
