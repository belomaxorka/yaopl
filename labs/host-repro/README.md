# Host reproductions of parser bugs

Small host programs that run OPL's own functions on malformed input and report
crashes, endless loops and AddressSanitizer findings. Each fix commit of this fork names
the check that shows its bug, with the result before and after.

Nothing here is built into OPL. The harnesses do not carry copies of OPL code:
`extract.py` copies the functions under test out of the OPL tree at build time, so one run
checks whatever is checked out.

## Run

Windows with Visual Studio Build Tools (MSVC with AddressSanitizer) and Python 3:

```
powershell -ExecutionPolicy Bypass -File labs\host-repro\run_all.ps1 [-Opl <OPL checkout>] [-Only <groups>] [-Images <dir>] [-Expected <file>]
```

`-Opl` defaults to this checkout. Inputs and binaries go to `labs/host-repro/build/`.
MSVC's `char` is signed, as on the EE and the IOP. `-Only cheat,zso,...` runs some groups
only. `-Images <dir>` also prints the game ID OPL derives from every `.iso` in the directory.

`-Expected labs\host-repro\expected.txt` compares every result line with that file and
exits with 1 on a difference. `CI-host-checks` (`.github/workflows/host-checks.yml`) runs
the checks that way on `windows-latest` for every push. After adding or changing a check,
regenerate the file from a run on this tree:
`run_all.ps1 > labs\host-repro\expected.txt` (its first line, the tree path, is ignored).

## What it covers

| Group | Function | Bug on upstream |
|---|---|---|
| `cheat` | `src/cheatman.c` `make_code()` | writes `digits[16]`, one byte past the array, for every code |
| `cheat` | `parse_buf()`, `trim_str()` | an indented comment becomes a cheat named `" "` |
| `cheat` | `is_empty_str()`, `is_empty_substr()` | a name without ASCII letters (Cyrillic, Japanese) is dropped and its codes join the cheat before it |
| `cheat` | `parse_buf()` | the 251st cheat is written to `gCheats[250]` |
| `cheat` | `parse_buf()` | a last line over 255 bytes without LF overflows `line[]` |
| `cheat` | `parse_buf()` | a last line without LF reads past the buffer |
| `cheat` | `set_cheats_list()` | a code line with address 0 ends the cheat (upstream #1548) |
| `cheat` | `ee_core/src/cheat_api.c` `SetupCheats()` | more than 5 hooks or 250 codes overflow the engine's tables (`cheat-t -s <list>` sweeps a corpus) |
| `iso` | `cdvdman_locatefile()`, isofs and in-game | a record with name length 0 loops forever; an extended attribute length >= 0x80 steps backwards; in-game, a name over 14 characters overflows `cdvdman_curdir[15]` |
| `find` | `cdvdman_findfile()` | a `/`-only path copies from address 1; long names overflow `name[16]` |
| `zso` | `ziso_init()`, `read_raw_data()` | align > 11 divides by zero; a block size other than 2048 is read as the wrong sectors |
| `zso` | `ziso_read_sector()` with the tree's LZ4 | a corrupt index writes in front of the buffer; a read over 256 sectors runs past the index cache |
| `cnf` | `ps2cnfGetBootFile()` | `BOOT2` without `=` passes NULL to `CNFGetKey()` |
| `sb` | `sbReadList()`, `updateISOGameList()` | a ul.cfg size that is not a multiple of 64 writes past the list; the cache compare reads past the cache |
| `name` | `isValidIsoName()` | reads before the name for `..` |
| `crc` | `USBA_crc32()` | non-ASCII titles index `crctab` negatively; `ref` is an independent unsigned implementation |
| `cfg` | `readFileBuffer()`, `strToColor()`, `splitAssignment()`, `parsePrefix()`, `configReadLegacyIP()`, `CheckPS2Logo()` | a `#` last line without LF crashes; a read error corrupts memory; colours, keys, prefixes and `IPCONFIG.DAT` overflow; the HDD logo read is one sector too long |
| `cheatsel` | `cheatConfigLoadSelections()` | picks of cheats that share a name are lost |
| `cfgwrite` | `configWrite()` | a failed write is reported as saved |
| `cdread` | in-game `cdvdman_read()` | 2340-byte sectors are copied 12 bytes past the buffer |
| `bdmprobe` | `bdmFindPartition()` | the write probe empties `conf_opl.cfg` (upstream #1397) |
| `vmc` | `sysCheckVMC()` | a VMC of another size is replaced without asking |
| `text` | `fntFitString()`, `drawAttributeText()` | the wrap scans past the terminator and writes over it; line breaks go into the config value (upstream #1713) |
| `theme` | `initGameCountText()`, `initImageTexture()`, `validateGUIElems()` | a 1-byte count buffer; a missing image file leaves a garbage texture; a theme without `ItemsList` leaves it NULL |
| `bdmindex` | `drawBDMIndex()` | the dots show the slot of five, not the connected devices: two devices give five dots, slots 0 and 2 give none on slot 0, one device in slot 2 gets dots |
| `padmacro` | `modules/pademu/padmacro.c` `padMacroPerform()` | none: checks the per-game button remap (the bit and its pressure byte move, value 3 does nothing, the slow-down button is the physical one, the IGR combo L1+L2+R1+R2 is not swapped); upstream has no remap |
| `lang` | `lngReadEntry()` | a `.lng` name shorter than `lang_X.lng` gives a negative length |
| `history` | `GetBootFilename()` | a boot file name over 16 characters overflows `filename[17]` |
| `gsm` | `PrepareGSM()` | `$GSMVMode` indexes past the table of 29 modes |
| `pad` | `readPads()` | the button repeat countdown jumps by 29 s at the tick counter wrap |
| `genvmc` | `mc_writecluster()` | a failed write leaves the abort semaphore taken |
| `atad` | in-game `ata_device_sector_io_internal()` | an LBA48 transfer of 65536 sectors or more loops forever, issuing 0-sector commands |
| `smb` | in-game `smb_ReadFile()`, `smb_WriteFile()`, `smb_ReadAndX()` (`modules/iopcore/cdvdman/smb.c`) | a failed read or write leaves `smb_io_sema` taken, so the next disc read and IGR block; a connection closed in the middle of a reply loops forever |
| `smbvmc` | `modules/mcemu/device-smb.c` `DeviceWritePage()`, `DeviceReadPage()` | an SMB error and a failed open of the VMC are reported to mcemu as a page done |
| `bgm` | `bgmLoad()` | the default BGM path is the format string |

## Console test files

`make_console_kit.py <bootable.iso> <out dir>` turns one bootable image (for example a
homebrew ELF packed as a disc) into the same malformed cases for a real PS2: ISO and ZSO
variants for `DVD/`, two `ul.cfg` sets, `.cht`, CFG, VMC, theme, cover and language files.
`iso_harness` can check an image directly: `iso-<tag>.exe file <image>`.
