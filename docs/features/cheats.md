# Cheats

YAOPL has the PS2RD cheat engine. It reads cheat files in the PS2RD format, `CHT/<game ID>.cht` on the game's
device (for example `CHT/SLUS_209.46.cht`).

## Turn cheats on

1. Put the cheat file in `CHT`.
2. Triangle on the game → **Cheat Settings**: set *Settings Mode* to Per Game (or use the global values), turn on
   **PS2RD Cheat Engine** and pick a mode:
    - **Auto Select Cheats** applies every cheat in the file.
    - **Select Game Cheats** shows a menu of the cheats when the game starts. Your picks are remembered for the
      next launch.
3. Save Changes in the game menu.

## File format

```
Mastercode
902D51F8 0C0B95B6

Infinite Health
2034C2A0 00000064
2034C2A4 00000064
```

- A line with exactly 16 hexadecimal digits (spaces allowed) is a code: an address and a value.
- Any other line starts a new cheat and is its name, in any script (Latin, Cyrillic, Japanese).
- Lines starting with `//` or `#` are comments.

A cheat named `Mastercode`, `Master Code` or `Enable Code (must be on)` (in any case) is always on, as the other
codes need it.

## Limits

- At most 250 cheats per file.
- The cheats turned on together may have at most about 250 code lines.
- A line may be up to 255 characters long, a name up to 128.

If the file is missing or cannot be read, YAOPL shows *Error: failed to load cheat file.* when the game starts.
