# Game settings

Press Triangle on a game for its [game menu](interface.md#game-menu). Its settings are saved in `CFG/<game ID>.cfg`
on the game's device (on the APA HDD, in the OPL partition).

## Global and per-game settings

Cheat, GSM, PADEMU, PADMACRO and OSD settings have a *Settings Mode*:

- **Global**: the game uses the global values, shared by every game that is set to Global. They are stored in
  `conf_game.cfg` with YAOPL's other settings.
- **Per Game**: the game has its own values, in its `CFG` file.

Changing a value while the mode is Global changes it for all games that use the global values. The global
PADEMU and PADMACRO settings can also be made in [Controller Settings](settings.md#controller-settings), and the
global OSD settings from the main menu.

## Saving, testing and removing

- **Save Changes** in the game menu saves the game's settings (and the global ones). *Game settings saved.*
- **Test Changes** starts the game with the settings as they are, without saving them, to try a mode first.
- **Remove Settings** asks which settings to remove: the confirm button removes the game's own settings, Square
  the global settings (for every game), Triangle both. Back cancels.

The top of the game menu shows where the settings came from: *Customized Settings* (saved by you) or
*Downloaded Defaults* ([Network Update](#network-update)).

## Game Settings

### Compatibility modes

If a game does not start, freezes or has glitches, try these modes, one at a time, with **Test Changes**:

| Mode | Name | What it does |
| --- | --- | --- |
| Mode 1 | Accurate Reads | Reads data at the speed of the real DVD drive. For games that break when data arrives faster than from a disc. |
| Mode 2 | Synchronous Mode | Reads data in another, synchronous way. For games that freeze while loading. |
| Mode 3 | Unhook Syscalls | Removes YAOPL's system call hooks once the game is running. For games that do not like them. |
| Mode 4 | Skip Videos | Skips the game's videos (PSS and Bink files appear empty). For games that freeze in a video. |
| Mode 5 | Emulate DVD-DL | Presents the image as a dual-layer DVD. For DVD9 games whose image lacks the layer information. |
| Mode 6 | Disable IGR | Turns off [in-game reset](features/igr.md) and its button combinations for this game. For games that conflict with them. |

If no mode helps, check whether the game works in official OPL, and whether the image is good, before
[reporting it](https://github.com/yaopl/yaopl/issues).

### Other fields

| Field | What it does |
| --- | --- |
| Download Defaults | Download this game's compatibility settings from the [Network Update](#network-update) server. |
| DMA Mode | Only for games on the internal HDD: the ATA transfer mode, MDMA 0–2 or UDMA 0–4 (default UDMA 4). A slower mode can help with HDDs or adapters that give read errors. On an exFAT HDD, any UDMA choice uses the fastest mode the drive supports. |
| Game ID | For online games that use DNAS: the ID of the original disc. **Load From Disc** reads it from a disc in the drive. |
| Custom ELF | Start another executable from the image instead of the one in `SYSTEM.CNF`. |
| Parental Lock | Ask for the [parental lock](settings.md#parental-lock-settings) password every time this game starts. Needs a password to be set. |

## Cheat Settings

| Field | What it does |
| --- | --- |
| Settings Mode | Global or Per Game |
| PS2RD Cheat Engine | Turn the cheat engine on for this game |
| PS2RD Cheat Engine Mode | **Auto Select Cheats** applies every cheat in the file; **Select Game Cheats** shows a menu when the game starts |

The cheat file is `CHT/<game ID>.cht`. See [Cheats](features/cheats.md) for the file format.

## GSM

GSM (Graphics Synthesizer Mode Selector) forces a video mode on a game, for example progressive 480p on a TV
that does not take interlaced video, or a VGA mode for a monitor.

| Field | What it does |
| --- | --- |
| Settings Mode | Global or Per Game |
| GSM Selector | Turn GSM on for this game |
| Video Mode | NTSC, PAL and their non-interlaced versions, PAL @60Hz, PS1 NTSC/PAL as HDTV 480p/576p, HDTV 480p, 576p, 720p, 1080i, VGA modes from 640×480 to 1280×1024 at various refresh rates |
| H-POS, V-POS | Move the picture, −100…100 |
| Emulate FIELD Flipping | For games that glitch in progressive modes |

The video mode, position and field flipping can be changed only while GSM Selector is on. Not every game
works in every mode; if the picture is broken, try another mode or turn GSM off. GSM is also needed for
[in-game screenshots](features/igr.md#in-game-screenshots).

## VMC

Configure VMC gives the game a virtual memory card in slot 1, slot 2 or both. See
[Virtual memory cards](features/vmc.md).

## PADEMU

In builds with PADEMU (the default `OPNPS2LD.ELF`), Configure PADEMU lets a DualShock 3 or DualShock 4 act as a
PS2 controller in the game, over USB or through a USB Bluetooth adapter.

| Field | What it does |
| --- | --- |
| Settings Mode | Global or Per Game |
| Pad Emulator | Turn pad emulation on for this game |
| Pad Emulator Mode | **DualShock3/4 USB** (controller on a USB cable) or **DualShock3/4 BT** (through a Bluetooth adapter) |
| Multitap Emulation | Emulate a multitap, for more than two players |
| Multitap Emulator On Port | The port with the emulated multitap, 1 or 2 |
| Settings For Port | The port (1A … 2D) that the next two fields apply to |
| Emulation | Emulate a controller on this port |
| Vibration | Vibration for this port |
| Pair DS Controller | Pair a controller with the Bluetooth adapter: connect the controller with a USB cable and press **Pair**. The adapter's and the controller's MAC addresses are shown above. |
| Fake DS3 Workaround | For some non-Sony DualShock 3 controllers that need it |
| Bluetooth Adapter Information | The adapter's IDs and features, and whether it should work with DS3/DS4 |

## PADMACRO

Configure PADMACRO (PADEMU builds) changes how an emulated controller behaves:

| Field | What it does |
| --- | --- |
| Settings Mode | Global or Per Game |
| Analog Slow-Down (Left Stick, Right Stick) | A button that reduces the stick's range, *While Button Pressed* or *Toggled By Button* |
| Analog Axis Invert (LX, LY, RX, RY) | Invert a stick axis |
| Turbo Button Speed | How fast a button repeats when turbo is active, 1–4 |
| Button Remap | Per game only: swap buttons for this game, **Off** (the default), **L2/R2 <-> Square/Cross** or **L2/R2 <-> L1/R1** |

PADMACRO works only on a DualShock 3 or 4 used through PADEMU, with PADEMU on for the game; a DualShock 2 in
the controller port is not changed.

**Button Remap** gives a game the modern trigger layout. Most PS2 racing games (Gran Turismo, Need for Speed,
Burnout) accelerate with Cross and brake with Square; with **L2/R2 <-> Square/Cross** the game gets Cross
from R2 and Square from L2, with the trigger's pressure, so gas and brake are analog. **L2/R2 <-> L1/R1** is
for games whose main pair of buttons is L1/R1. Notes:

- It is a swap: the buttons trade places, and nothing is lost. Cross on the controller now sends R2, so in
  the game's menus you confirm with R2.
- It is saved only in the game's own settings, whatever the Settings Mode, and cannot be set in Controller
  Settings: menus of other games would stop working with it.
- DualShock 3: all its buttons are pressure-sensitive, so both directions are analog. DualShock 4: only L2
  and R2 are analog; a trigger moved onto Cross or Square is analog, Cross or Square moved onto a trigger
  is on or off.
- Games with their own controller setup (Gran Turismo 4 has one) do not need it; use one or the other.
- Do not use it with PS3 guitars: it swaps their frets.
- The other macros (slow-down button, turbo, PS-button combos) use the buttons you press, before the swap.
- While L1, L2, R1 and R2 are all held, nothing is swapped, so the IGR and GSM combos (L1+L2+R1+R2 with
  Start+Select, L3+R3 or Up) work as without the remap.

## OSD Settings

Per-game override of the OSD language, screen size and video output; see
[OSD Settings](settings.md#osd-settings).

## Network Update

Main menu → **Network Update** downloads compatibility settings (modes and other game settings) for all your
games from the OPL compatibility list server (`sx.sytes.net`, run by the OPL community, not by YAOPL).
*Download Defaults* in Game Settings does it for one game.

- It needs a working network connection (see [Network setup](devices/network.md)) and does not run while
  [UDPBD](devices/udpbd.md) is on.
- Games whose settings you saved yourself are skipped, unless *Overwrite Existing Records* is on.
- It works for games on the HDD, SMB and the first BDM page. On the other BDM pages and on MMCE it ends with
  *Update failed.*; apps are not updated.
