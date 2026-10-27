# In-game reset and screenshots

In-game reset (IGR) leaves a running game without turning the console off.

## Button combinations

While a game runs, hold:

| Buttons | Effect |
| --- | --- |
| L1 + L2 + R1 + R2 + START + SELECT | In-game reset |
| L1 + L2 + R1 + R2 + L3 + R3 | Power off |
| L1 + L2 + R1 + R2 + Up | [Screenshot](#in-game-screenshots), then in-game reset (`EXTRA_FEATURES` builds, with GSM on) |

The console's power/reset button also works: press it once to power off, twice quickly for an in-game reset.

[Mode 6](../game-settings.md#compatibility-modes) (Disable IGR) turns all of this off for a game that conflicts
with it.

## Where IGR goes

The reset stops the game and its devices, then:

- If **IGR Path** is set in [Settings](../settings.md#settings), it starts that ELF. Point it at
  `OPNPS2LD.ELF` to come back to YAOPL, or at your launcher, for example `mc0:/BOOT/BOOT.ELF`. Paths on
  `mass:` need `USBD.IRX` and `USBHDFSD.IRX` in `mc0:/SYS-CONF/` or `mc1:/SYS-CONF/` (FreeMcBoot installs
  them).
- If IGR Path is **empty**, it goes to the console's browser (the memory card and settings screen of the stock
  firmware).
- With an [MMCE](../devices/mmce.md) card and *MMCE Start Mode* on, the card switches back to its boot card on
  IGR (see *IGR Boot Card Slot(s)*), so the console boots again from that card, for example into FreeMcBoot.

If the IGR Path ELF cannot be loaded, the console goes to the browser.

## In-game screenshots

`EXTRA_FEATURES` builds (from the `OPNPS2LD-VARIANTS` package) can take a screenshot of a game (IGS):

1. Turn on [GSM](../game-settings.md#gsm) for the game.
2. Have a memory card in **slot 2**.
3. In the game, hold L1 + L2 + R1 + R2 + Up.

The picture is saved on the memory card in slot 2 as `<game ID>_GS(<number>).bmp`, then the console does an
in-game reset.
