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

!!! warning
    Do not change the files on the served disk from the PC while the PS2 uses it: both sides writing to one
    file system can corrupt it.

## On the PS2

1. Main menu → **Network Settings**: set *IP Address Type* to **Static** and give the PS2 an address on the
   same LAN as the PC (see [Network setup](network.md)). UDPBD does not use DHCP.
2. **Settings**: set **BDM Start Mode** to Auto, open **BDM Devices** and turn on **UDPBD**.
3. Save Changes in the main menu. The games appear on the *UDPBD Games* page.

## Limits

- **SMB and UDPBD cannot share the network adapter.** While UDPBD is on, SMB, the NBD server and Network Update
  do not start (*The network adapter is used by UDPBD...*). The other way round, if SMB started first, UDPBD
  waits for a restart of YAOPL (*The network adapter is used by SMB. Restart OPL to use UDPBD.*). Turn the one
  you do not use off and restart YAOPL.
- Per-game settings and VMCs are stored on the served disk, so saving them needs a server that can write;
  udpbd-vexfat is read-only.
- UDPBD is not used when a launcher starts a BDM game directly in YAOPL.
