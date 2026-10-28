# Install and launch

## What you need

- A PlayStation 2 that can run homebrew: a memory card with FreeMcBoot or OpenTuna, PS2BBL, OSDMenu, a
  modchip, or any other way to start an ELF file. YAOPL is started like any other homebrew program.
- A device for your games: see [Choosing a device](devices/index.md).
- A backward-compatible PS3 also works, with limits: see [PS3](#ps3).

## Download

Every change to `master` is built and published as the
[`latest` release](https://github.com/belomaxorka/yaopl/releases/tag/latest):

| File | Contents |
| --- | --- |
| `OPNPS2LD.ELF` | The loader, ready to run. PADEMU is on, IGS and RTL are off. |
| `OPNPS2LD-<version>.7z` | The same ELF with `LICENSE` and `CREDITS`. |
| `OPNPS2LD-VARIANTS-<version>.7z` | Four ELFs, one for each combination of `PADEMU=0/1` and `EXTRA_FEATURES=0/1`, with `LICENSE` and `CREDITS`. |
| `OPNPS2LD-LANGS-<version>.7z` | Translations and their fonts, for the `LNG` folder. |
| `SHA256SUMS` | Checksums of the files above. |

The `.7z` files also come without the version in their name, for links that should always get the newest
build.

Which ELF to take:

- **`OPNPS2LD.ELF`** suits most people.
- **`PADEMU=0`** leaves out DualShock 3/4 [pad emulation](game-settings.md#pademu) and pad macros. Take it if
  you never use a DualShock 3 or 4 on the PS2.
- **`EXTRA_FEATURES=1`** adds [in-game screenshots](features/igr.md#in-game-screenshots) (IGS) and
  right-to-left languages (RTL).

GSM, VMC, cheats, the parental lock and every device are in every build. START → About shows which options
your ELF was built with.

To check a download, run `sha256sum -c SHA256SUMS` on Linux, or `Get-FileHash OPNPS2LD.ELF` in PowerShell and
compare the hash with the line in `SHA256SUMS`.

## Install

1. Copy `OPNPS2LD.ELF` to a memory card (for example with wLaunchELF) or to a USB drive.
2. Start it from your launcher. In wLaunchELF, open the file and press Circle. In FreeMcBoot or OSDMenu, add
   it as a menu entry.

The file name is the same as OPL's, so an existing launcher entry for OPL starts YAOPL once you replace the
file.

!!! tip "Updating"
    To update, replace `OPNPS2LD.ELF` with the new one. Your settings, games, VMCs and art stay where they
    are. Settings made by official OPL work in YAOPL too.

## First launch

The first time, YAOPL has no settings, so every device is off and it opens on the main menu.

1. Choose **Settings**.
2. Turn on the device that holds your games: set its *Start Mode* to **Auto** (start it at boot) or
   **Manual** (start it from its page when you need it). For USB, MX4SIO, iLink, an exFAT HDD and UDPBD this is
   *BDM Start Mode*; then open **BDM Devices** to choose which of them to use. See
   [Settings](settings.md#settings).
3. Optionally set *Default Menu* to the page you want to see at boot.
4. Leave the dialog with **OK**, then choose **Save Changes** in the main menu.

!!! warning "Save Changes"
    The settings dialogs change settings only until YAOPL exits. Choose **Save Changes** in the main menu to
    keep them.

Press START (or the back button) to leave the main menu for the game list. On a page whose device is not
started yet, press the confirm button to start it. YAOPL creates its [folders](games.md#folders) on the
device, and you can copy your games there.

### Where settings are saved

YAOPL looks for its settings, `conf_opl.cfg` and its companion files, in this order:

1. `OPL` on a memory card (`mc0:OPL` or `mc1:OPL`);
2. the device YAOPL was started from (a USB drive or the APA HDD);
3. a USB or other BDM drive, then the internal HDD.

When it saves, it uses the device it was started from, then a memory card, then a USB drive, then the HDD.
With *Notifications* on (Display Settings), YAOPL shows where it loaded the settings from, and Save Changes
says where it saved them. If the settings are found on a USB drive, its *BDM Start Mode* is switched to Auto;
settings found on the APA HDD switch on the HDD page. The full list of files is in
[Configuration files](config-files.md).

### Keys at startup

Hold these while YAOPL starts:

| Keys | Effect |
| --- | --- |
| START | Start without loading any settings. Use it when settings from another OPL build make YAOPL freeze; then save your settings again. |
| Triangle + Cross | Reset the menu's video mode to Auto, for when the screen stays black after a video mode change. |

## Version

START → About shows the version, for example `v1.2.0-Beta-2310-1a2b3c4-yaopl`. Builds of YAOPL end in
`-yaopl`, so they are easy to tell from official OPL builds. Quote it in
[issue reports](https://github.com/belomaxorka/yaopl/issues).

## PS3

YAOPL runs on backward-compatible PS3 models only (boards COK-001 and COK-002/COK-002W), with USB, SMB and the
HDD. It needs an entry point for PS2 titles; custom firmware with Cobra is preferred, and only custom firmware
supports the HDD.
