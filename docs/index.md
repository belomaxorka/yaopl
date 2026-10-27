# YAOPL documentation

![YAOPL - Yet Another Open PS2 Loader](logo.svg){ width="480" }

YAOPL (Yet Another Open PS2 Loader) is a game and application loader for the PlayStation 2 and
backward-compatible PS3 models. It continues [Open PS2 Loader](https://github.com/ps2homebrew/Open-PS2-Loader)
(OPL): it runs the same games from the same devices, keeps OPL's folders, settings and file name, and adds its
own fixes and new devices. Guides written for OPL mostly apply to YAOPL too; these pages say where YAOPL
differs.

## Where to start

- New to YAOPL or OPL? Read [Install and launch](getting-started.md), then [The interface](interface.md).
- Pick where your games will live: [Choosing a device](devices/index.md) compares USB drives, SD cards on
  MX4SIO, the internal HDD, a PC share over SMB or UDPBD, and MMCE memory-card emulators.
- Put games on it: [Adding games](games.md) covers ISO, ZSO and USBExtreme images, file names and folders.
- A game does not work? Try its [compatibility modes](game-settings.md#compatibility-modes), then see
  [Troubleshooting](troubleshooting.md).
- Every option in the menus is listed in the [Settings reference](settings.md) and
  [Game settings](game-settings.md).

## What YAOPL can load

| From | How | Page |
| --- | --- | --- |
| USB drives, SD cards (MX4SIO), iLink drives | FAT32 or exFAT | [USB, MX4SIO and iLink](devices/usb.md) |
| Internal HDD (expansion bay or network adapter) | exFAT on MBR/GPT, or APA with HDL partitions | [Internal HDD](devices/hdd.md) |
| A shared folder on a PC or NAS | SMBv1 | [SMB share](devices/smb.md) |
| A disk or disk image served by a PC | UDPBD (new in YAOPL) | [UDPBD](devices/udpbd.md) |
| SD2PSX, MemCard PRO 2, PSXMemCard Gen2 | MMCE (new in YAOPL) | [MMCE](devices/mmce.md) |

Games can be ISO, ZSO (compressed ISO) or USBExtreme images. YAOPL also lists homebrew [apps](features/apps.md),
applies [cheats](features/cheats.md), gives games [virtual memory cards](features/vmc.md), forces video modes
with [GSM](game-settings.md#gsm), emulates DualShock 3/4 controllers ([PADEMU](game-settings.md#pademu)) and
returns from games with [in-game reset](features/igr.md).

## Getting help

- Quote the version from START → About (YAOPL versions end in `-yaopl`) when you
  [report a problem](https://github.com/belomaxorka/yaopl/issues).
- What changed in each build is in the [changelog](https://github.com/belomaxorka/yaopl/blob/master/CHANGELOG.md).
- Problems with official OPL builds belong to
  [upstream OPL](https://github.com/ps2homebrew/Open-PS2-Loader/issues).
