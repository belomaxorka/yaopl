# USB, MX4SIO and iLink

USB drives, SD cards in an MX4SIO adapter, iLink drives, internal HDDs formatted as exFAT and [UDPBD](udpbd.md)
all go through BDM, the block device manager. They are set up the same way and hold games the same way.

## Set up

1. In **Settings**, set **BDM Start Mode** to Auto or Manual.
2. Open **BDM Devices** (the button next to it) and turn on the devices you use. USB is always on; iLink,
   MX4SIO, *HDD (GPT/MBR)* and UDPBD are off until you turn them on.
3. Choose **OK**, then **Save Changes** in the main menu.

Each connected device gets its own page, titled after its type: *USB Games*, *MX4SIO Games*, *iLink Games*,
*HDD Games*, *UDPBD Games*. Up to five BDM devices can be used at once. A page appears when its device is
plugged in and disappears when it is removed.

## Format the drive

- File system: **FAT32** or **exFAT**.
- Partition table: **MBR**, or **GPT** for drives over 2 TB.
- On FAT32 no file can be 4 GB or larger, so a bigger DVD game has to be split into the
  [USBExtreme format](../games.md#usbextreme-ul). On exFAT it can stay one ISO.

## Copy games so they are not fragmented

YAOPL reads a game straight from the drive's sectors, so it needs to know where every piece of the file is.

- A game may have **at most 64 fragments**. For a USBExtreme game, all its parts together share these 64.
  A game with more is refused with *Error, the game is fragmented.*
- A [VMC](../features/vmc.md) on a BDM device must not be fragmented at all.
- Copy games one at a time, onto a drive with enough free space. The surest way to defragment is to copy
  everything to a PC, format the drive and copy it back. Defragmentation tools are not recommended.
- When formatting as exFAT, keep the default allocation unit size.

## Prefix path

*BDM Prefix Path* in Settings makes YAOPL use a folder on every BDM device instead of its root: with
`OPL`, games go in `OPL/DVD` and settings in `OPL/CFG`. Leave it empty to use the root.

## USB

Any USB mass-storage drive: flash drives, card readers, hard drives and SSDs in USB enclosures (a hard drive
may need its own power). The PS2's USB ports are USB 1.1, so USB is the slowest device: games with long
videos or streaming may stutter; a faster device helps. Some games skip their videos with
[Mode 4](../game-settings.md#compatibility-modes).

## MX4SIO

MX4SIO is an SD card adapter that plugs into a memory card slot. Turn on **MX4SIO** in BDM Devices and format the
card as FAT32 or exFAT like a USB drive.

## iLink

Early fat PS2 models have an iLink (IEEE 1394, FireWire) port, which can connect an SBP-2 drive. Turn on
**iLink** in BDM Devices.

## HDD (GPT/MBR)

An internal HDD formatted as exFAT on MBR or GPT is a BDM device too: turn on **HDD (GPT/MBR)** in BDM Devices.
See [Internal HDD](hdd.md) for how to choose between exFAT and APA.
