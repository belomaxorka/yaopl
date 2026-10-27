# Virtual memory cards

A virtual memory card (VMC) is a memory card image in the `VMC` folder, `VMC/<name>.bin`, that a game uses
instead of a real card. A game can have a VMC in slot 1, slot 2 or both. VMCs work on USB, MX4SIO, iLink, the
internal HDD, SMB and UDPBD; not yet on [MMCE](../devices/mmce.md#limits).

## Give a game a VMC

1. Triangle on the game → **Configure VMC**.
2. For *VMC Slot 1* (or 2), choose the button with the card's name to open the VMC dialog:
    - **Name**: the card's file name. YAOPL suggests one from the game ID; games of the same series that share
      saves get the same suggested name.
    - **Size**: 8, 16, 32 or 64 MB.
    - **Status** says whether the file exists, has a wrong size, or needs to be created.
    - **Create** makes a new card (a progress bar shows the formatting). For an existing card the button is
      **Modify**; changing the size reformats the card. **Delete** deletes the file.
3. The second button next to the slot is **Use Generic** (use a card named `generic_0` or `generic_1`, shared by
   every game set to it) or **Reset** (give this slot no VMC; the file stays).
4. Save Changes in the game menu.

Modifying or deleting an existing card needs *Write Operations* on in Settings; creating a new one does not.

## Notes

- YAOPL creates cards of 8, 16, 32 and 64 MB. It leaves cards of other sizes (4 MB, 128 MB, PCSX2 `.ps2`) alone
  and asks before it changes one.
- On a BDM device (USB, MX4SIO, iLink, exFAT HDD) the VMC file must be in one piece. If it is fragmented, YAOPL
  asks *VMC ... file is fragmented. Continue with Memory Card in slot N?* and the game uses the real card if
  you continue. Copy the card to a PC and back, or create it on a freshly formatted drive.
- If the card cannot be opened, YAOPL asks *Error accessing VMC ... Continue with the Memory Card in slot N?*.
- PC tools (for example `pc/genvmc` in the YAOPL repository, or memory card managers that read raw `.bin`
  images) can create and edit VMCs.
