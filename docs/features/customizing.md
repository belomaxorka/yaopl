# Art, themes and languages

## Art

With *Cover Art* on in [Display Settings](../settings.md#display-settings), YAOPL shows images from the `ART`
folder of the game's device. They are PNG files named after the game ID and the kind of image:

| File | Shown as (internal theme) |
| --- | --- |
| `<ID>_COV.png` | Cover, next to the game list |
| `<ID>_ICO.png` | Icon |
| `<ID>_BG.png` | Background of the info screen |
| `<ID>_SCR.png`, `<ID>_SCR2.png` | Screenshots on the info screen |

For example `ART/SLUS_209.46_COV.png`. Other themes can use other names (for example `_LGO` for a logo); see the
theme's description. Images can be up to 1024×1024 pixels; smaller images load faster.

- Art for a game is read from the same device as the game.
- Art for an [app](apps.md) is named after its ELF file: `ART/MYAPP.ELF_COV.png`. It can be on the app's device
  or another enabled one.
- PC tools such as OPL Manager can download art for your games.

## Themes

A theme changes the look of the menu. Put each theme in its own folder named `thm_<Name>` (for example
`thm_Dark`) inside `THM` on a device, or in `mc0:OPL` / `mc1:OPL` on a memory card. Then pick it under *Theme*
in [Display Settings](../settings.md#display-settings) and Save Changes. YAOPL lists up to 64 themes.

A theme folder holds `conf_theme.cfg`, its images and fonts. Themes made for OPL work in YAOPL.

## Languages

The menu is in English by default. Translations are in the `OPNPS2LD-LANGS` package of each release.

1. Copy `lang_<Name>.lng` (and `font_<Name>.ttf` if the package has one for that language) to `LNG` on a device,
   or to `mc0:OPL` / `mc1:OPL`.
2. Pick the language under *Language* in Display Settings and Save Changes.

YAOPL lists up to 15 language files. A language whose script is not in the built-in font needs its font
file next to it. Right-to-left languages need an `EXTRA_FEATURES` build.

With *Notifications* on, YAOPL shows which device a theme or language was loaded from.
