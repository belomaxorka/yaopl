# Internal HDD

An ATA or SATA drive in the PS2's expansion bay (through the official network adapter or an expansion-bay HDD
adapter) is the fastest and most compatible place for games. YAOPL supports two ways of formatting it; use one
of them, not both.

| | exFAT (MBR or GPT) | APA |
| --- | --- | --- |
| Format on | A PC | The PS2 (wLaunchELF, the HDD browser) or PC tools |
| Games are | ISO, ZSO and USBExtreme files in `CD` and `DVD` | HDL partitions |
| Add games with | Any file manager on a PC | HDL tools such as HDL-Dump, on a PC or over [NBD](../features/nbd.md) |
| Largest disk | Any (GPT over 2 TB) | 2 TB |
| In Settings | BDM Start Mode, BDM Devices → HDD (GPT/MBR) | HDD (APA) Start Mode |
| Page | *HDD Games* (a BDM page) | *HDD Games* |
| Also works with | — | PS2 HDD browser, HDD-OSD launchers |

Only one HDD can be used at a time: while *HDD (GPT/MBR)* is on, *HDD (APA) Start Mode* cannot be changed, and
while the APA page is on, *HDD (GPT/MBR)* cannot be turned on. Turn one off first.

Both ways have a per-game [DMA mode](../game-settings.md#game-settings) and use *Automatic HDD Spin Down* from
Settings: the drive stops after that many minutes without use (1–20; 0 never stops it).

## exFAT

1. On a PC, create one partition (MBR, or GPT for disks over 2 TB) and format it as exFAT with the default
   allocation unit size.
2. Put the drive in the PS2. In **Settings**, set **BDM Start Mode** to Auto and turn on **HDD (GPT/MBR)** under
   **BDM Devices**. Save Changes.
3. Copy games into `DVD` and `CD` as on a [USB drive](usb.md), one at a time so that they are not fragmented.

On an exFAT drive the DMA mode of a game can only choose between MDMA modes and "the fastest UDMA mode the
drive supports" (up to UDMA 4); a lower UDMA mode is not used.

## APA

APA is the PS2's own partition format. Each game is an HDL partition, installed with a PC tool such as
HDL-Dump (with the drive connected to a PC, or over the network with YAOPL's [NBD server](../features/nbd.md)).

1. Format the drive on the PS2, for example with wLaunchELF (HDD Manager → Format) or the HDD browser.
2. In **Settings**, set **HDD (APA) Start Mode** to Auto. Save Changes.
3. Install games as HDL partitions. They appear on the *HDD Games* page.

YAOPL keeps its own folders (`CFG`, `VMC`, `ART`, `THM`, `LNG`, `CHT`, `APPS`) in one partition, chosen by
`hdd0:__common/OPL/conf_hdd.cfg`:

```
hdd_partition=+OPL
```

- Without that file YAOPL writes it with `+OPL` and creates a **128 MB `+OPL` partition**. Any partition named
  there that does not exist is created the same way.
- If the name starts with `+`, the folders are in the root of the partition; otherwise they are in an `OPL`
  folder inside it (for example `hdd_partition=__common` uses `__common/OPL`).
- Do not enlarge partitions that YAOPL created: that breaks their LBAs. To get more room, delete the partition
  and create a bigger one with wLaunchELF, then copy its contents back.
- *Cache Game List (HDD)* in Settings stores the list of HDL games in `games.bin` in that folder and shows it
  at the next start without scanning the disk. The HDD page does not refresh by itself; press SELECT after
  installing games.

YAOPL loads its APA drivers only when the drive really is APA: on a drive with an MBR or GPT partition table,
the APA page stays empty.

### Starting an HDD game from another launcher

HDD-OSD launchers (for example OPL Launcher) can start an HDL game directly through YAOPL. YAOPL then uses
the game's settings from the OPL partition, the IGR path, spin-down and MMCE settings, without showing its
menu.
