# Adding games

Games are disc images of your own PS2 discs. YAOPL reads ISO, ZSO (compressed ISO) and USBExtreme images from
USB, MX4SIO, iLink, exFAT HDDs, SMB, UDPBD and MMCE. On an [APA-formatted HDD](devices/hdd.md#apa) games are HDL
partitions instead.

## Folders

When a device starts, YAOPL creates these folders on it if they are missing:

| Folder | Contents |
| --- | --- |
| `CD` | Games that came on a CD (blue-bottom discs) |
| `DVD` | Games that came on a DVD (DVD5 and DVD9) |
| `CFG` | Per-game settings, `<game ID>.cfg` |
| `VMC` | [Virtual memory cards](features/vmc.md) |
| `ART` | [Cover art](features/customizing.md#art) and other images |
| `THM` | [Themes](features/customizing.md#themes), one folder each |
| `LNG` | [Translations](features/customizing.md#languages) |
| `CHT` | [Cheat files](features/cheats.md), `<game ID>.cht` |
| `APPS` | [Homebrew apps](features/apps.md) |

The APA HDD has no `CD` and `DVD` folders; its other folders are in the [OPL partition](devices/hdd.md#apa).
A *prefix path* (BDM Prefix Path, ETH Prefix Path in Settings; MMCE Prefix Path in MMCE Settings) moves all of
them into a folder on the device.

A CD game put in `DVD` (or the other way round) may not start: the folder tells YAOPL which kind of disc to
emulate.

## ISO

Put `.iso` images in `CD` or `DVD`. Two ways of naming them work:

- **The title only**: `Kingdom Hearts II.iso`. YAOPL reads the game ID from the image's `SYSTEM.CNF`.
- **Game ID, then the title**: `SLUS_209.46.Kingdom Hearts II.iso`. YAOPL takes the game ID from the name and
  does not open the image to list it, which is faster on slow devices. The ID must be in the form `XXXX_NNN.NN`
  followed by a dot.

The title can be up to 160 characters. An image whose `SYSTEM.CNF` cannot be read (a damaged image, or not a
PS2 game) is not listed.

YAOPL remembers the IDs of the games it has read in `games.bin` inside `CD` and `DVD`, so the list appears
faster next time. It is updated when games are added or removed. If you replace an image with a different game
under the same file name, delete `games.bin` in that folder.

## ZSO

ZSO is an ISO compressed with LZ4; put `.zso` files next to the ISOs, named the same way. To make them, use
`pc/ziso.py` from the YAOPL repository, which needs Python 3 and the `lz4` package (`pip install lz4`):

```sh
python ziso.py -c 2 "input.iso" "output.zso"    # compress
python ziso.py -c 0 "input.zso" "output.iso"    # decompress
```

YAOPL reads ZSO files with **2048-byte blocks only** and refuses others, which OPL would read as the wrong
sectors. So do not pass `-b`, and with maxcso pass `--block=2048` (its default for images of 2 GiB or more is
16 KiB blocks).

The *BDM Cache*, *HDD Cache* and *SMB Cache* settings set how many sectors YAOPL reads ahead for ZSO games
(see [Settings](settings.md#settings)).

## USBExtreme (UL)

The old USBExtreme format splits a game into 1 GB parts listed in `ul.cfg`, which lets FAT32 hold games of 4 GB
and more. The parts and `ul.cfg` are in the root of the device (or of the prefix path), not in `CD` or `DVD`.
Make them with a PC tool such as OPL Manager or `pc/iso2opl`. Titles are up to 32 characters.

A game whose title has non-ASCII characters uses the CRC that PC tools compute, which some upstream OPL builds
computed differently; parts named by such a build may need renaming.

USBExtreme games cannot be started from [MMCE](devices/mmce.md) or by another launcher.

## Fragmentation

On USB, MX4SIO, iLink and exFAT HDDs, a game may have at most 64 fragments. See
[Copy games so they are not fragmented](devices/usb.md#copy-games-so-they-are-not-fragmented).

## Game IDs

The game ID (for example `SLUS_209.46`) names a game's files: `CFG/<ID>.cfg`, `CHT/<ID>.cht`, `ART/<ID>_COV.png`,
VMC names. It is the name of the main executable in `SYSTEM.CNF`, or the start of the file name as above. For
an HDL game it is stored in the partition.

## Renaming and deleting

With *Write Operations* on in [Settings](settings.md#settings), the [game menu](interface.md#game-menu) has
Rename and Delete:

- **Rename** renames the image (an ID at the start of the name and the extension are kept) or all parts of a
  USBExtreme game; for an HDL game it changes the title stored in the partition.
- **Delete** deletes the image, all parts of a USBExtreme game, or the HDL partition. The game's settings,
  cheats, art and VMCs stay.

## Starting a game from another launcher

Launchers can start a game in YAOPL directly, without its menu, by passing arguments to `OPNPS2LD.ELF`:

- `<game ID> <start sector> <OPL partition> mini` for an HDL game (used by HDD-OSD launchers such as OPL
  Launcher);
- `<file name> <game ID> CD|DVD bdm` for an ISO or ZSO on a BDM device (the first internal exFAT HDD, else the
  first USB device). USBExtreme games and UDPBD are not supported this way.

The game's settings from `CFG` are used.
