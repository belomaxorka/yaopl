# UDPBD

UDPBD (UDP Block Device) lets the PS2 use a disk or disk image that a PC serves over the LAN as if it were a
USB drive. It is faster than SMB and runs games with all of YAOPL's features. It is new in YAOPL and off by
default.

## On the PC

1. Prepare a disk or a disk image with an exFAT (or FAT32) file system that has the usual `CD` and `DVD`
   folders, as on a [USB drive](usb.md).
2. Run a UDPBD server for it, for example
   [udpbd-server](https://github.com/israpps/udpbd-server),
   [udpbd-vexfat](https://github.com/awaken1ng/udpbd-vexfat) (serves a folder as a read-only virtual exFAT
   disk) or [PS2-Servers](https://github.com/NathanNeurotic/PS2-Servers) (with a GUI). See the server's own
   instructions.
3. Start the server before YAOPL starts UDPBD. The PS2 finds it by broadcast on the LAN.

Choose a server that writes (udpbd-server or PS2-Servers with a disk image) if you want per-game settings on
the served disk and VMCs; with a read-only one (udpbd-vexfat, PS2-Servers serving a folder) games run, but some features are not available,
see [Servers that write and read-only servers](#servers-that-write-and-read-only-servers).

!!! warning
    Do not change the files on the served disk from the PC while the PS2 uses it: both sides writing to one
    file system can corrupt it.

## On the PS2

1. Main menu → **Network Settings**: set *IP Address Type* to **Static** and give the PS2 an address on the
   same LAN as the PC (see [Network setup](network.md)). UDPBD does not use DHCP.
2. **Settings**: set **BDM Start Mode** to Auto, open **BDM Devices** and turn on **UDPBD**.
3. Save Changes in the main menu. The games appear on the *UDPBD Games* page.

## Servers that write and read-only servers

The UDPBD protocol does not tell whether the server writes, and read-only servers answer every write as if
it had worked. So when the UDPBD device connects, YAOPL writes a small test file, `yaopl_write_test.tmp`, at
the root of the served disk, reads it back and deletes it. If the data does not come back, the server is
treated as read-only until the device connects again.

| | Server that writes | Read-only server |
| --- | --- | --- |
| Examples | udpbd-server, PS2-Servers with a disk image | udpbd-vexfat; PS2-Servers with a folder or `--read-only`; udpbd-server with an image it cannot write |
| [Game settings](../game-settings.md) | `CFG/<game ID>.cfg` on the served disk, as on a USB drive | `CFG/<game ID>.cfg` next to YAOPL's own settings: `mc0:OPL/CFG/`, or `CFG` on the device YAOPL was started from |
| [VMC](../features/vmc.md) | In `VMC/` on the served disk | Not available: *Configure VMC* shows a note; a VMC set in the game's settings is not used and the game saves to the real memory card |
| Rename, Delete | In the game menu, with *Write Operations* on | Not offered, for games and for apps in the served `APPS` folder |
| Folders (`CD`, `DVD`, `CFG`, ...) | Created if missing | Not created |
| Game list cache (`games.bin`) | Kept up to date | Read if there is one, not written |
| YAOPL's own settings | Never saved there: they are read before UDPBD starts | Never saved there |

With a read-only server, a game that has no settings saved next to YAOPL's own yet uses the
`CFG/<game ID>.cfg` on the served disk, if there is one (for example made by a PC manager). After the first
save on the console, the file next to YAOPL's settings is used and the one on the served disk is no longer
read.

If YAOPL keeps its own settings on a USB drive (for example because it was started from it), the folder next
to them is the `CFG` folder in the root of that drive; YAOPL makes it if it is missing. A game on a read-only
server and the same game (same ID) on the USB drive then share one settings file when the drive has no BDM
prefix. That is intended: a VMC set for the USB copy stays in the file and is simply not used when the game
runs from the server.

A device is read-only for YAOPL only when the write check above fails; the same rules would apply to any
other device found read-only.

!!! note
    Writing over UDPBD (settings, VMCs) is new and not yet tested on a console. Keep a copy of the disk image.

## Limits

- **SMB and UDPBD cannot share the network adapter.** While UDPBD is on, SMB, the NBD server and Network Update
  do not start (*The network adapter is used by UDPBD...*). The other way round, if SMB started first, UDPBD
  waits for a restart of YAOPL (*The network adapter is used by SMB. Restart OPL to use UDPBD.*). Turn the one
  you do not use off and restart YAOPL.
- With a read-only server (udpbd-vexfat) there are no VMCs and no Rename or Delete, and game settings are
  saved next to YAOPL's own settings, see [above](#servers-that-write-and-read-only-servers).
- UDPBD is not used when a launcher starts a BDM game directly in YAOPL.
