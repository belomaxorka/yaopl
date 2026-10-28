# MMCE

MMCE devices are memory-card emulators with an SD card that can also hold games: SD2PSX, MemCard PRO 2 and
PSXMemCard Gen2. YAOPL lists the games on the card's SD card (the *MMCE Games* page) and streams them in game.
It also tells the card which game is starting, so the card can switch to that game's own memory card. MMCE
support is new in YAOPL.

## Prepare the SD card

Put the games in `CD` and `DVD` on the card's SD card (the file system the card's firmware uses), named as in
[Adding games](../games.md). With an *MMCE Prefix Path* (below) they go in that folder instead of the root.

## Settings

Main menu → **MMCE Settings**:

| Field | Default | What it does |
| --- | --- | --- |
| MMCE Start Mode | Off | Off, Manual or Auto, as for other devices |
| MMCE Slot | Auto | The memory card slot of the card that holds the games: 1, 2, or Auto (the first slot where a card answers) |
| IGR Boot Card Slot(s) | Auto | On [in-game reset](../features/igr.md), the cards in these slots switch back to their boot card: None, 1, 2, Both, or Auto (the slots where a card answered) |
| MMCE Prefix Path | empty | A folder on the SD card that holds `CD`, `DVD` and the other folders |
| Send the game ID to the card | On | When a game starts, from any device, the card switches to the game's own memory card |

Choose **OK** and **Save Changes**. The card's own settings (which memory card it shows, game ID support) are
made in its firmware or menu.

## How it works with games

- With *Send the game ID to the card* on, every game started from YAOPL, also from USB, MX4SIO or the HDD, makes
  the card switch to the memory card for that game. YAOPL waits until the card has finished switching (an
  SD2PSX shows a progress bar meanwhile), up to 7.5 s, and then starts the game, so the game finds its own
  memory card ready.
- An SD2PSX also switches by itself: it notices the game in the memory card's history, which YAOPL writes when
  a game starts, and switches a few seconds later, also with *Send the game ID to the card* off or MMCE Start Mode
  Off. To keep the card on its current memory card, turn off game ID in the SD2PSX's own menu.
- On IGR, the cards in the IGR slots switch back to their boot card, so the console boots it again (for
  example into FreeMcBoot on that card).
- These commands are sent only while *MMCE Start Mode* is Manual or Auto. With it Off, YAOPL sends nothing to the
  memory card slots, and a console without an MMCE card is not affected.

## Limits

- **VMCs are not used** for games on MMCE yet: the game saves to the memory card that the card emulates.
  YAOPL warns about this if the game has a VMC set.
- **Games split into parts** (USBExtreme) cannot be started from MMCE yet.
- *No MMCE card answered.* means no card in the chosen slot responded: check the slot setting and the card's
  firmware.
