# The interface

YAOPL has a game list with one page per device, a main menu for its settings (START), and a menu for each game
(Triangle). The hints at the bottom of the screen show which buttons do what.

## Confirm and back

One of Cross and Circle confirms, the other goes back. Which one is set by *Select Button* in
[Controller Settings](settings.md#controller-settings). Without settings, YAOPL uses Circle to confirm on a
Japanese console and Cross on others. These pages say "confirm" and "back" for these two buttons.

## Game list

Each device has its own page, for example *USB Games*, *HDD Games*, *ETH Games* (SMB), *UDPBD Games*,
*MMCE Games* and *Apps*. A page appears when its device is turned on in [Settings](settings.md#settings).

| Button | In the game list |
| --- | --- |
| Left / Right | Previous / next device page |
| Up / Down | Previous / next game |
| L1 / R1 | One page of games up / down |
| L2 / R2 | First / last game |
| Confirm | Start the game. On a device set to Manual that is not started yet: start the device. |
| Triangle | The [game menu](#game-menu). On the Apps page: Rename and Delete, if Write Operations is on. |
| Square | The info screen of the game, see below. |
| SELECT | Refresh the list |
| START | The [main menu](#main-menu) |

Starting one BDM page (USB, MX4SIO, iLink, an exFAT HDD or UDPBD) starts all of them. With *Automatic
Refresh* on (Display Settings), YAOPL notices new and removed games by itself; USB and other BDM devices are
also checked for being plugged in or out without it.

**Info screen.** Square shows the game's title, genre, release date, developer, size and description, with
its background art if there is one. The text comes from the game's settings file in `CFG`, as written by PC
tools such as OPL Manager. Back returns to the list. A theme without an info screen has no Square button.

**Auto start.** With *Remember Last Played Game* and *Auto Start* set in [Settings](settings.md#settings),
YAOPL selects the last game at boot and starts it after a countdown ("Auto start in N s..."). Press any button
to stop the countdown.

## Main menu

START opens the main menu:

| Item | What it does |
| --- | --- |
| Settings | Devices, start modes, IGR path, write operations and more. [Details](settings.md#settings) |
| Display Settings | Theme, language, colors, the menu's video mode. [Details](settings.md#display-settings) |
| Audio Settings | Sound effects, boot sound, background music. [Details](settings.md#audio-settings) |
| Controller Settings | Confirm button, scrolling, global PADEMU and PADMACRO. [Details](settings.md#controller-settings) |
| OSD Settings | The language, screen size and video output that games see. [Details](settings.md#osd-settings) |
| Parental Lock Settings | A password for the menus and for chosen games. [Details](settings.md#parental-lock-settings) |
| Network Settings | The PS2's address and the SMB server. [Details](devices/network.md) |
| Network Update | Download compatibility settings for your games. [Details](game-settings.md#network-update) |
| Start NBD Server | Share the internal HDD with a PC over the network. [Details](features/nbd.md) |
| MMCE Settings | Memory-card emulators that hold games. [Details](devices/mmce.md#settings) |
| About | Version, build options and credits |
| Save Changes | Write all settings to the [settings files](getting-started.md#where-settings-are-saved) |
| Exit | Leave YAOPL for the console's browser |
| Power Off | Turn the console off |

START or back returns to the game list once at least one device is turned on. If a parental lock password is
set, every item except About, Exit and Power Off asks for it; once it is entered, YAOPL does not ask again
until you choose Save Changes.

## Game menu

Triangle on a game opens its menu. The top shows the game's title and where its settings come from (for
example *Customized Settings* or *Downloaded Defaults*).

| Item | What it does |
| --- | --- |
| Game Settings | Compatibility modes, DMA mode, game ID, custom ELF, parental lock. [Details](game-settings.md#game-settings) |
| Cheat Settings | Turn on the cheat engine and choose how cheats are picked. [Details](features/cheats.md) |
| Configure GSM | Force a video mode for the game. [Details](game-settings.md#gsm) |
| Configure VMC | Give the game virtual memory cards. [Details](features/vmc.md) |
| Configure PADEMU, Configure PADMACRO | DualShock 3/4 emulation and pad macros (PADEMU builds). [Details](game-settings.md#pademu) |
| OSD Settings | Per-game override of the OSD language, screen size and video output. [Details](settings.md#osd-settings) |
| Save Changes | Save this game's settings |
| Test Changes | Start the game with the settings as they are now, without saving them |
| Remove Settings | Remove the game's own settings, the global ones, or all. [Details](game-settings.md#saving-testing-and-removing) |
| Rename, Delete | Rename or delete the game's files (only with *Write Operations* on in Settings, and not on a read-only UDPBD server) |

## Dialogs

In a settings dialog:

- Up, Down, Left and Right move between the fields.
- Confirm on a field starts editing it. An on/off field switches at once. For a number or a list, press Up
  and Down to change the value, then confirm to keep it or back to undo the change.
- A text field opens the on-screen keyboard.
- **OK** at the bottom applies the dialog. Back leaves the dialog without applying it (Audio Settings apply
  every change at once).
- Applied settings are kept only until YAOPL exits, unless you choose **Save Changes**.

## On-screen keyboard

| Button | Effect |
| --- | --- |
| D-pad | Move between the keys and the command column |
| Confirm | Type the key, or run the command (Backspace, Space, Enter, Mode) |
| Square | Backspace |
| Triangle | Space |
| SELECT | Switch the keyboard layout (Mode) |
| START | Enter: accept the text |
| Back | Cancel |

Password fields show `*` for each character.
