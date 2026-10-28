# Settings reference

Every option of the main menu (START), with its default and the key it is saved as. The keys are in
`conf_opl.cfg` unless noted; see [Configuration files](config-files.md). Changes are kept only after
**Save Changes** in the main menu. Per-game settings are on the [Game settings](game-settings.md) page.

## Settings

| Option | Default | What it does | Key |
| --- | --- | --- | --- |
| Debug Colors | Off | Flashes colors on the screen while a game starts, to show how far it got. For reporting problems. | `disable_debug` (1 = on) |
| PS2 Logo | Off | Shows the PS2 logo when a game starts, for a disc whose logo matches the console's region. | `ps2logo` |
| Cache Game List (HDD) | Off | APA HDD: keeps the list of HDL games in `games.bin` and shows it at start without scanning the disk. | `hdd_game_list_cache` |
| IGR Path | empty | An ELF to start after an [in-game reset](features/igr.md), for example `mc0:/BOOT/BOOT.ELF`. Empty: the console's browser. | `exit_path` |
| Write Operations | Off | Allows Rename and Delete of games and apps, changing or deleting VMCs, and writing through the [NBD server](features/nbd.md). | `enable_delete_rename` |
| Remember Last Played Game | Off | Selects the last game you played when YAOPL starts. | `remember_last` |
| Auto Start | 9 | With *Remember Last Played Game* on: start that game after this many seconds (0–9, 0 = do not start). | `autostart_last` |
| BDM Prefix Path | empty | A folder on BDM devices that holds YAOPL's folders. | `usb_prefix` |
| ETH Prefix Path | empty | A folder inside the SMB share that holds YAOPL's folders. | `eth_prefix` |
| Automatic HDD Spin Down | 20 | Minutes without use before the internal HDD stops (0–20, 0 = never). | `hdd_spindown` |
| BDM Start Mode | Off | Off, Manual or Auto for USB, MX4SIO, iLink, exFAT HDD and UDPBD. | `usb_mode` |
| BDM Devices | — | Which BDM devices to use, see below. Shown when BDM Start Mode is not Off. | |
| HDD (APA) Start Mode | Off | Off, Manual or Auto for the APA HDD. Locked while *HDD (GPT/MBR)* is on. | `hdd_mode` |
| ETH Start Mode | Off | Off, Manual or Auto for the [SMB share](devices/smb.md). | `eth_mode` |
| Applications Start Mode | Off | Off, Manual or Auto for the [Apps](features/apps.md) page. | `app_mode` |
| Default Menu | Apps | The page shown when YAOPL starts: BDM Games, ETH Games, HDD Games, Apps or MMCE (if it is not Off). | `default_device` |
| BDM Cache | 16 | Sectors (2 KB each) read ahead for ZSO games on BDM devices, 0–32; 0 turns it off. | `bdm_cache` |
| HDD Cache | 8 | The same for ZSO games on the APA HDD. | `hdd_cache` |
| SMB Cache | 16 | The same for ZSO games on SMB. | `smb_cache` |

Start modes: **Off** hides the device; **Manual** shows its page and starts it when you press confirm there;
**Auto** starts it with YAOPL.

### BDM Devices

| Option | Default | Key |
| --- | --- | --- |
| USB | always on | |
| iLink | Off | `enable_ilink` |
| MX4SIO | Off | `enable_mx4sio` |
| HDD (GPT/MBR) | Off; locked while HDD (APA) Start Mode is not Off | `enable_bdm_hdd` |
| UDPBD | Off | `enable_udpbd` |

See [USB, MX4SIO and iLink](devices/usb.md), [Internal HDD](devices/hdd.md) and [UDPBD](devices/udpbd.md).

## Display Settings

| Option | Default | What it does | Key |
| --- | --- | --- | --- |
| Theme | internal | The [theme](features/customizing.md#themes) | `theme` |
| Language | English | The menu [language](features/customizing.md#languages) | `language_text` |
| Automatic Sorting | On | Sort games by title | `autosort` |
| Automatic Refresh | Off | Check the devices for new or removed games by itself | `autorefresh` |
| Cover Art | Off | Show [art](features/customizing.md#art) from `ART` | `enable_coverart` |
| Notifications | Off | Show where the settings, theme and language were loaded from, and mounted partitions | `enable_notifications` |
| Text, Selected, Dialog and Background Color | theme colors | Colors of the internal theme (other themes bring their own); **Reset Colors** restores them | `text_color`, `sel_text_color`, `ui_text_color`, `bg_color` |
| Video Mode | Auto | The menu's own video mode: Auto, PAL, NTSC, EDTV 480p/576p, VGA 640×480, HIRES modes, HDTV 720p and 1080i, 240p/256p | `vmode` |
| H-POS, V-POS | 0 | Move the menu on the screen, −300…300 | `xoff`, `yoff` |
| Overscan | 0 | Shrink the menu to fit TVs that cut the edges, 0–100 | `overscan` |
| Widescreen | Off | Lay the menu out for a 16:9 screen | `wide_screen` |

After a *Video Mode* change YAOPL asks *Confirm video mode change?* and goes back to the old mode if you do not
confirm. If the screen stays black, hold Triangle + Cross while YAOPL starts to reset it to Auto. The menu's
video mode does not change a game's; use [GSM](game-settings.md#gsm) for that.

## Audio Settings

Changes apply at once.

| Option | Default | Key |
| --- | --- | --- |
| Sound Effects | Off | `enable_sfx` |
| Boot Sound | Off | `enable_boot_snd` |
| Background Music | Off | `enable_bgm` |
| Sound Effects Volume | 80 | `sfx_volume` |
| Boot Sound Volume | 80 | `boot_snd_volume` |
| Background Music Volume | 70 | `bgm_volume` |
| Default Theme Music | empty: a path to a music file to play with the internal theme | `default_bgm_path` |

## Controller Settings

| Option | Default | What it does | Key |
| --- | --- | --- | --- |
| Scrolling Speed | Medium | Slow, Medium or Fast scrolling in lists | `scrolling` |
| Select Button | Circle on Japanese consoles, else Cross | The confirm button; the other one goes back | `swap_select_btn` (0 = Circle, 1 = Cross) |
| Analog X-Axis / Y-Axis Sensitivity | Medium | Low, Medium or High, for moving with the left stick | `x_sensitivity`, `y_sensitivity` |
| Configure PADEMU, Configure PADMACRO | — | The global [PADEMU](game-settings.md#pademu) and [PADMACRO](game-settings.md#padmacro) settings (PADEMU builds) | in `conf_game.cfg` |

## OSD Settings

The PS2 stores the language, screen type and video output chosen in its system configuration, and games read
them. OSD Settings make a game see other values. In the main menu they are the global values; in a game's menu
you can give one game its own (*Settings Mode* Global or Per Game).

| Option | What it does | Key |
| --- | --- | --- |
| Settings Mode | Per game only: Global or Per Game | `$CustomLanguageSource` |
| Enable | Use the values below instead of the console's | `$OSDSettingsEnable` |
| OSD Language | Japanese, English, French, Spanish, German, Italian, Dutch, Portuguese or System Default | `$CustomLanguageValue` |
| Screen Size | 4:3, Full Screen, 16:9 or System Default | `$OSDAspectRatio` |
| Video Mode | RGB, Y Cb/Pb Cr/Pr (component) or System Default | `$OSDVideoMode` |

Global values are saved in `conf_game.cfg`, per-game values in the game's `CFG` file.

## Parental Lock Settings

| Option | What it does | Key |
| --- | --- | --- |
| Parental Lock Password | When set, the settings menus, game menus and Rename/Delete ask for it. Leave empty to turn the lock off. | `parental_lock_password` |

- Once you enter the password, YAOPL does not ask again until you choose Save Changes.
- A game with *Parental Lock* on in its [Game Settings](game-settings.md#other-fields) asks for the password every
  time it starts, even when the menus are unlocked.
- The password is stored in plain text in `conf_opl.cfg`. If you forget it, delete the line
  `parental_lock_password=` from that file on a PC. (Starting YAOPL with START held and saving also removes
  it, but replaces all your other settings with the defaults too.)

## Network Settings

The PS2's address and the SMB server: see [Network setup](devices/network.md) and
[SMB share](devices/smb.md#on-the-ps2). Saved in `conf_network.cfg`.

## MMCE Settings

See [MMCE](devices/mmce.md#settings).

| Option | Default | Key |
| --- | --- | --- |
| MMCE Start Mode | Off | `mmce_mode` |
| MMCE Slot | Auto | `mmce_slot` (0 = slot 1, 1 = slot 2, 2 = Auto) |
| IGR Boot Card Slot(s) | Auto | `mmceigr_slot` (0 Off, 1 slot 1, 2 slot 2, 3 both, 4 Auto) |
| MMCE Prefix Path | empty | `mmce_prefix` |
| Send the game ID to the card | On | `mmce_gameid` |

## Other main menu items

- **Network Update**: see [Network Update](game-settings.md#network-update).
- **Start NBD Server**: see [NBD server](features/nbd.md).
- **About**: the version (ending in `-yaopl`), the build options (GSM, UDMA+, RTL, IGS, PADEMU) and credits.
- **Save Changes**: saves `conf_opl.cfg`, `conf_network.cfg` and `conf_game.cfg`.
- **Exit**: leaves YAOPL for the console's browser. **Power Off**: turns the console off.
