# Choosing a device

YAOPL can load games from several kinds of devices, and you can use more than one at a time: each has its own
page in the game list.

| Device | Needs | Good for | Limits |
| --- | --- | --- | --- |
| [USB drive](usb.md) | Nothing extra | Getting started; any PS2 | The PS2's USB 1.1 is slow: some games stutter in videos or loading |
| [MX4SIO](usb.md#mx4sio) | An SD card adapter in a memory card slot | Faster than USB, small | Takes a memory card slot |
| [iLink](usb.md#ilink) | An iLink (FireWire) drive and an early PS2 with an iLink port | Early fat models | Only models with an iLink port |
| [Internal HDD](hdd.md) | An HDD in the expansion bay (fat PS2 with a network adapter, or an HDD adapter) | The fastest, most compatible way | Fat PS2 or expansion-bay adapter |
| [SMB share](smb.md) | A network adapter and a PC or NAS on the LAN | Keeping games on a PC | SMBv1 only; slower than an HDD |
| [UDPBD](udpbd.md) | A network adapter, a PC running a UDPBD server | Faster than SMB, all YAOPL features | New in YAOPL; SMB is off while it runs |
| [MMCE](mmce.md) | SD2PSX, MemCard PRO 2 or PSXMemCard Gen2 | Games and memory cards on one card | New in YAOPL; no VMC, no split games |

Every device holds games the same way, in the folders `CD` and `DVD` (see [Adding games](../games.md)), except
the APA-formatted internal HDD, which holds them as HDL partitions.

## Turning a device on

In [Settings](../settings.md#settings) each kind of device has a *Start Mode*:

- **Off**: the device is not used and its page is hidden.
- **Manual**: the page is there, and the device starts when you press confirm on it.
- **Auto**: the device starts when YAOPL starts.

USB, MX4SIO, iLink, an exFAT HDD and UDPBD share *BDM Start Mode* ("block device manager"); choose which of them
to use under **BDM Devices**. The SMB share is *ETH Start Mode*, the APA HDD *HDD (APA) Start Mode*, apps
*Applications Start Mode*, and MMCE has its own [MMCE Settings](mmce.md#settings).
Choose **Save Changes** in the main menu to keep the choice.

## Folders on a device

When a device starts, YAOPL creates the folders it uses if they are missing: `CD`, `DVD`, `CFG`, `VMC`, `ART`,
`THM`, `LNG`, `CHT` and `APPS` (no `CD` and `DVD` on the APA HDD). A *prefix path* in Settings or MMCE Settings
moves them all into a folder of your choice on that device. See [Folders](../games.md#folders).
