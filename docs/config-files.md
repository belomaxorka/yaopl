# Configuration files

YAOPL's settings are plain text files with one `key=value` line per setting. You do not need to edit them by
hand: every setting has a place in the menus. This page is for backing them up, copying them between devices,
or fixing a setting that makes YAOPL unusable.

## Where they are

| File | Contents | Where |
| --- | --- | --- |
| `conf_opl.cfg` | YAOPL's settings (Settings, Display, Audio, Controller, Parental Lock, MMCE) | See below |
| `conf_network.cfg` | Network Settings and the SMB server | Next to `conf_opl.cfg` |
| `conf_game.cfg` | The *global* game settings: cheats, GSM, PADEMU, PADMACRO, OSD | Next to `conf_opl.cfg` |
| `conf_last.cfg` | The last game played | Next to `conf_opl.cfg` |
| `conf_apps.cfg` | Legacy app list (optional) | `mc?:OPL` or the root of a device, see [Apps](features/apps.md) |
| `CFG/<game ID>.cfg` | One game's own settings | On the game's device |
| `CD/games.bin`, `DVD/games.bin` | Cache of game IDs | On the device |
| `hdd0:__common/OPL/conf_hdd.cfg` | Which APA partition is YAOPL's | APA HDD, see [Internal HDD](devices/hdd.md#apa) |

`conf_opl.cfg` and its companions are kept together: in `mc0:OPL` or `mc1:OPL` on a memory card, or in the root
of a USB drive or of the APA HDD's OPL partition. The search order is in
[Where settings are saved](getting-started.md#where-settings-saved).

These files are the same as OPL's, so settings can be moved between OPL and YAOPL. The parental lock password
and the SMB password are stored in plain text.

## conf_opl.cfg

The keys of each option are listed in the [Settings reference](settings.md). Values are numbers (`0` off, `1`
on, list positions starting at 0), text, or colors as `#RRGGBB`.

## conf_network.cfg

| Key | Option |
| --- | --- |
| `eth_linkmode` | Ethernet Link Mode (0 Auto, 1 100 Mbit full, 2 100 Mbit half, 3 10 Mbit full, 4 10 Mbit half) |
| `ps2_ip_use_dhcp` | IP Address Type (0 Static, 1 DHCP) |
| `ps2_ip_addr`, `ps2_netmask`, `ps2_gateway`, `ps2_dns` | The PS2's addresses |
| `smb_share_use_nbns` | SMB Address Type (0 IP, 1 NetBIOS) |
| `smb_ip`, `smb_share_nb_addr` | The SMB server's IP address or NetBIOS name |
| `smb_port` | SMB port |
| `smb_share`, `smb_user`, `smb_pass` | Share, user, password |
| `nbd_default_export` | The NBD export name (no menu option; empty means `hdd0`) |

## Game settings files

`CFG/<game ID>.cfg` holds the settings saved in the [game menu](game-settings.md). Keys starting with `$` are
settings; keys starting with `#` describe the game and are written by YAOPL:

| Key | Setting |
| --- | --- |
| `$Compatibility` | Compatibility modes as a sum: Mode 1 = 1, Mode 2 = 2, Mode 3 = 4, Mode 4 = 8, Mode 5 = 16, Mode 6 = 32 |
| `$DMA` | DMA mode (0–2 MDMA 0–2, 3–7 UDMA 0–4) |
| `$DNAS` | Game ID for DNAS |
| `$AltStartup` | Custom ELF |
| `$ParentalLock` | Per-game parental lock |
| `$VMC_0`, `$VMC_1` | VMC names for slot 1 and slot 2 |
| `$EnableCheat`, `$CheatMode`, `$CheatsSource`, `$CheatSel...` | Cheat settings and picks |
| `$EnableGSM`, `$GSMVMode`, `$GSMXOffset`, `$GSMYOffset`, `$GSMFIELDFix`, `$GSMSource` | GSM settings |
| `$EnablePadEmu`, `$PadEmuSettings`, `$PADEMUSource`, `$PadMacroSettings`, `$PadMacroSource` | PADEMU and PADMACRO |
| `$OSDSettingsEnable`, `$CustomLanguageValue`, `$OSDAspectRatio`, `$OSDVideoMode`, `$CustomLanguageSource` | OSD settings |
| `$ConfigSource` | Where the settings came from (1 saved by you, 2 downloaded by Network Update) |

The `*Source` keys are 0 for Global, 1 for Per Game. The info screen shows `Title`, `Genre`, `Release`,
`Developer` and `Description` from this file when PC tools have added them.

`conf_game.cfg` uses the same keys for the global values.
