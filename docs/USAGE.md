# YAOPL user guide

How to install YAOPL, where to put games and how to use its features. YAOPL keeps OPL's folders, settings
files and ELF name, so guides written for Open PS2 Loader mostly apply too; this guide notes where YAOPL
differs. What changed from build to build is in [CHANGELOG.md](../CHANGELOG.md).

- [Getting started](#getting-started)
- [Folders](#folders)
- [Devices](#devices): [USB, MX4SIO, iLink and GPT/MBR HDDs](#usb-mx4sio-ilink-and-gptmbr-hdds),
  [SMB](#smb), [APA HDD](#apa-hdd), [UDPBD](#udpbd), [MMCE](#mmce)
- [Game formats](#game-formats): [ISO](#iso), [ZSO](#zso), [USBExtreme (UL)](#usbextreme-ul)
- [Features](#features): [apps](#apps), [cheats](#cheats), [VMC](#vmc), [in-game reset](#in-game-reset-igr),
  [NBD server](#nbd-server), [PS3](#ps3)
- [Startup keys and frequent issues](#startup-keys-and-frequent-issues)
- [Building from source](#building-from-source)

## Getting started

Download from the [`latest` release](https://github.com/belomaxorka/yaopl/releases/tag/latest):

| File | Contents |
| --- | --- |
| `OPNPS2LD.ELF` | The loader, ready to run. PADEMU is on, IGS and RTL are off. |
| `OPNPS2LD-<version>.7z` | The same ELF with `CHANGELOG.md`, `CREDITS`, `LICENSE` and `README.md`. |
| `OPNPS2LD-VARIANTS-<version>.7z` | Four ELFs for each combination of `PADEMU=0/1` and `EXTRA_FEATURES=0/1`. |
| `OPNPS2LD-LANGS-<version>.7z` | Translations and their fonts, for the `LNG` folder. |
| `SHA256SUMS` | Checksums: `sha256sum -c SHA256SUMS` (Linux) or `Get-FileHash <file>` (Windows). |

`PADEMU` is pad emulation for DualShock 3 and DualShock 4 controllers, with pad macros. `EXTRA_FEATURES` adds
in-game screenshots (IGS) and right-to-left languages (RTL). GSM, VMC, cheats and the parental lock are in
every build.

Copy the ELF to a memory card or a USB drive and start it from FreeMcBoot, OSDMenu, wLaunchELF or any other
launcher. It has the same file name as OPL's ELF, so an existing launcher entry keeps working. To see the
version, press START and choose About; YAOPL versions end in `-yaopl`, for example
`v1.2.0-Beta-2310-1a2b3c4-yaopl`. Quote this version in [issue reports](https://github.com/belomaxorka/yaopl/issues).

Press START for the main menu, which has Settings, Display Settings, Audio Settings, Controller Settings,
OSD/Language Settings, Parental Lock Settings, Network Settings, Start NBD Server, MMCE Settings, About,
Save Changes and more. Press Triangle on a game for its own settings: Compatibility, Cheat, GSM, VMC and (in PADEMU
builds) PADEMU and PADMACRO settings. In the list, SELECT refreshes it, L1 and R1 page, L2 and R2 jump to
the first and the last game.

## Folders

The first time you enable a device, YAOPL creates these folders on it:

| Folder | Contents | Devices |
| --- | --- | --- |
| `CD` | games on CD media (blue-bottom discs) | all but APA HDD |
| `DVD` | DVD5 and DVD9 images | all but APA HDD |
| `VMC` | virtual memory cards | all |
| `CFG` | per-game settings | all |
| `ART` | game art | all |
| `THM` | themes, one folder `thm_<name>` each | all |
| `LNG` | translations, `lang_<Name>.lng` with an optional `font_<Name>.ttf` | all |
| `CHT` | cheat files, `<game ID>.cht` | all |
| `APPS` | homebrew ELFs | all |

Themes and translations can also be kept in `mc0:OPL` or `mc1:OPL` on a memory card. Pick them in Display
Settings.

## Devices

Each device has its own page in the game list. Turn devices on in Settings, set them to Auto to start them
at boot or to Manual to start them from their page.

### USB, MX4SIO, iLink and GPT/MBR HDDs

USB drives, SD cards on an MX4SIO adapter, iLink (SBP2) drives and expansion-port HDDs formatted with MBR or
GPT all go through BDM (Settings → BDM Devices).

- File systems: FAT32 and exFAT. Partition tables: MBR and GPT (GPT for disks over 2 TB).
- On FAT32 a game over 4 GB must be in USBExtreme format (see below); on exFAT it can be one ISO.
- A game may have at most 64 fragments. For a USBExtreme game, all its parts share these 64. A game with
  more is refused with a "fragmented" message. The surest way to defragment is to copy everything to a PC,
  format the drive and copy it back; defragmentation tools are not recommended.
- When formatting an HDD as exFAT, keep the default allocation unit size, and copy games one at a time so
  that they stay contiguous.

### SMB

YAOPL loads games from an SMBv1 share on a PC or NAS, in the same folders as above. Share a folder with full
read and write permissions and enter the PS2's address and the share in Network Settings (stored in
`conf_network.cfg`). The PS2 can use DHCP or a static address.

### APA HDD

An expansion-port HDD with the APA partition scheme (as made by the PS2 browser, HDLoader or wLaunchELF)
holds games as HDL partitions; install them with a PC tool such as HDL-Dump. YAOPL's own folders go in the
partition named by `hdd_partition` in `hdd0:__common/OPL/conf_hdd.cfg`. Without that file YAOPL writes it
with `hdd_partition=+OPL` and creates a 128 MB `+OPL` partition. Do not enlarge partitions that YAOPL created,
as that breaks their LBAs; delete and recreate them with wLaunchELF instead. APA works up to 2 TB; for larger
disks use GPT and exFAT (see above).

### UDPBD

UDPBD loads games over the LAN from a disk or disk image that a PC serves, and runs them with all of
YAOPL's features. It is new in YAOPL and off by default.

1. On the PC, run a UDPBD server (udpbd-server, udpbd-vexfat or PS2-Servers) for a disk or image that has
   the usual `CD` and `DVD` folders. Start it before YAOPL starts UDPBD.
2. In Network Settings, give the PS2 a static IP address on the same LAN; UDPBD does not use DHCP. The server
   is found by broadcast.
3. Turn on Settings → BDM Devices → UDPBD. Its games appear as "UDPBD Games".

Limits:

- SMB and UDPBD cannot share the network adapter: while UDPBD is on, SMB, the NBD server and the other network
  features do not start.
- Per-game settings and VMCs are stored on the UDPBD disk, so they need a server that can write
  (udpbd-vexfat is read-only).
- With the APA HDD page enabled, turning it on while UDPBD runs needs a restart of YAOPL.

### MMCE

MMCE is a memory-card emulator that also stores games: SD2PSX, MemCard PRO 2 and PSXMemCard Gen2. YAOPL lists
the games in `CD` and `DVD` on the card's SD card (`mmce0:` for slot 1, `mmce1:` for slot 2, plus an optional
prefix path) and streams them in game. Configure it in the main menu under MMCE Settings: start mode, slot,
prefix path, the IGR boot card slot and "Send the game ID to the card".

- With "Send the game ID" on (the default), the card switches to the game's own memory card when a game
  starts, from any device. IGR switches the cards back to their boot card.
- These commands are sent only while the MMCE device is on (Auto or Manual), so a console without an MMCE card
  gets none.
- Not supported yet: VMCs on the card (the game saves to the emulated card) and games split into parts.

## Game formats

### ISO

Put `.iso` images in `CD` or `DVD`. The file name should be the game's title, or the game ID followed by the
title (`SLUS_209.46.Kingdom Hearts II.iso`). ISOs are installed on an APA HDD as HDL partitions instead.

### ZSO

ZSO is a compressed ISO (LZ4). Put `.zso` files next to the ISOs. To make them, use `pc/ziso.py` from this
repository, which needs Python 3 and the `lz4` package (`pip install lz4`):

```sh
python ziso.py -c 2 "input.iso" "output.zso"    # compress
python ziso.py -c 0 "input.zso" "output.iso"    # decompress
```

YAOPL reads ZSO files with 2048-byte blocks only and rejects others, which OPL would read as the wrong sectors.
So do not pass `-b`, and do not use maxcso's defaults for images of 2 GiB or more (16 KiB blocks); with
maxcso, pass `--block=2048`.

### USBExtreme (UL)

The old USBExtreme format splits a game into 1 GB parts listed in `ul.cfg`, which lets FAT32 hold games over
4 GB. Make it with a PC tool such as OPL Manager or `pc/iso2opl`. A game whose title has non-ASCII
characters uses the CRC that PC tools compute, which upstream OPL computed differently in some builds; parts
named by such an OPL build may need renaming.

## Features

### Apps

YAOPL lists homebrew ELFs in two ways.

**`title.cfg`.** Put each app in its own folder inside `APPS`, with a `title.cfg` next to the ELF:

```
title=My App Name
boot=MYAPP.ELF
```

An optional `argv1=` line passes an argument. On a memory card, the app folders go in the root of `mc0:` or
`mc1:` and their names must contain `_`.

**`conf_apps.cfg`** (legacy). One line per app, the name and the full path:

```
My App Name=mass:APPS/MYAPP.ELF
```

YAOPL looks for it in `mc?:OPL/`, then in the root of each enabled device. In both methods, names are case
sensitive.

### Cheats

YAOPL reads PS2RD cheat files, `CHT/<game ID>.cht`. In a game's Cheat Settings, turn on the PS2RD Cheat Engine
and pick a mode:

- **Auto Select Cheats** applies every cheat in the file.
- **Select Game Cheats** shows a menu when the game starts. The picks are remembered for the next launch.

A cheat named `Mastercode` (or `Master Code`, `Enable Code (must be on)`) is always on, as the other codes need
it. YAOPL keeps cheat names in any script (Cyrillic, Japanese) and handles multi-line codes and large cheat
files that crashed or confused OPL; see the changelog.

### VMC

A VMC is a memory card image in `VMC`, used by a game instead of a real card. Create or pick one in a game's
VMC Settings; new cards are 8, 16, 32 or 64 MB. YAOPL leaves cards of other sizes (4 MB, 128 MB, PCSX2 `.ps2`)
alone and asks before it modifies one.

### In-game reset (IGR)

While a game runs:

- L1 + L2 + R1 + R2 + START + SELECT returns to YAOPL (or to the IGR path set in Settings).
- L1 + L2 + R1 + R2 + L3 + R3 powers the console off.
- L1 + L2 + R1 + R2 + Up takes a screenshot, in `EXTRA_FEATURES` builds with GSM on.

### NBD server

Start NBD Server in the main menu shares the PS2's APA HDD over the network as a
[Network Block Device](https://github.com/NetworkBlockDevice/nbd/blob/master/doc/proto.md), so PC tools
(`hdl_dump`, `pfs-shell`, a hex editor) can use it as if the drive were attached. The PS2 needs an IP address
(static or DHCP). The export is read-only unless Write Operations is on in Settings. The server is based on
[lwNBD](https://github.com/bignaux/lwNBD).

| Client | Systems | Connect | Disconnect |
| --- | --- | --- | --- |
| `nbd-client` | Linux, WSL with a custom kernel | `nbd-client 192.168.1.45 /dev/nbd1` | `nbd-client -d /dev/nbd1` |
| `nbdfuse` | Linux, WSL2 | `mkdir ps2 && nbdfuse ps2/ nbd://192.168.1.45 &` | `umount ps2` |
| [WNBD](https://cloudbase.it/ceph-for-windows/) | Windows (elevated PowerShell) | `wnbd-client.exe map hdd0 192.168.1.45` | `wnbd-client.exe unmap hdd0` |

`nbd-client -l <ip>` or `nbdinfo --list nbd://<ip>` lists the exports; `nbd-client` needs `modprobe nbd`. For
example, to install a game with `hdl_dump` while connected through `nbdfuse`:
`hdl_dump inject_dvd ps2/nbd "Test Game" ./TEST.ISO`. macOS has no client.

### PS3

YAOPL runs on backward-compatible PS3 models only (boards COK-001 and COK-002/COK-002W), with USB, SMB and HDD.
It needs an entry point for PS2 titles; custom firmware with Cobra is preferred, and only CFW supports HDD.

## Startup keys and frequent issues

- **YAOPL freezes on the logo or a grey screen.** Settings from another OPL build may be the cause. Hold START
  while YAOPL starts to skip loading them, then save your settings again.
- **Nothing on screen after boot.** The video mode may not suit the TV. Hold Triangle + Cross while YAOPL
  starts to reset it to Auto (not together with START).
- **A game freezes on a white screen.** Its main executable could not be read: the image is fragmented or
  damaged.
- **A game does not start or freezes.** Try the compatibility modes in its Compatibility Settings, one at a
  time, and check whether official OPL behaves the same before you
  [report it](https://github.com/belomaxorka/yaopl/issues).

## Building from source

YAOPL builds with the ps2dev toolchain image pinned in `.github/workflows/compilation.yml` (`TOOLCHAIN_IMAGE`):

```sh
git clone https://github.com/belomaxorka/yaopl.git && cd yaopl
docker run --rm -v "$PWD:/src" -w /src <TOOLCHAIN_IMAGE> sh -c 'git config --global --add safe.directory /src && make release'
```

`make` builds the ELF, `make release` also the 7z package, as CI does. Use a full clone: the revision in the
version counts the commits. The options are at the top of the `Makefile` (for example `PADEMU=0`,
`EXTRA_FEATURES=1`, `DTL_T10000=1`). `make format` formats the code with clang-format 12. See
[CONTRIBUTING.md](../CONTRIBUTING.md).
