# Host reproductions of parser bugs

Small host programs that run OPL's own parsing functions on malformed input and report
crashes, endless loops and AddressSanitizer findings. They back the `fix/*` branches of this
fork; each branch's commit message names the test that shows its bug.

Nothing here is built into OPL. The harnesses do not carry copies of OPL code:
`extract.py` copies the functions under test out of the OPL tree at build time, so one run
checks whatever is checked out.

## Run

Windows with Visual Studio Build Tools (MSVC with AddressSanitizer) and Python 3:

```
powershell -ExecutionPolicy Bypass -File labs\host-repro\run_all.ps1 [-Opl <OPL checkout>]
```

`-Opl` defaults to this checkout. Inputs and binaries go to `labs/host-repro/build/`.
MSVC's `char` is signed, as on the EE and the IOP.

## What it covers

| Test | Function | Bug on master |
|---|---|---|
| `cheat *` | `src/cheatman.c` `make_code()` | writes `digits[16]`, one byte past the array, for every code |
| `cheat cht_indented` | `parse_buf()`, `trim_str()` | an indented comment becomes a cheat named `" "` |
| `cheat cht_251` | `parse_buf()` | the 251st cheat is written to `gCheats[250]` |
| `cheat cht_long_last` | `parse_buf()` | a last line over 255 bytes without LF overflows `line[]` |
| `cheat cht_no_lf` | `parse_buf()` | a last line without LF reads past the buffer |
| `isofs/cdvdman locatefile zero` | `cdvdman_locatefile()` | a record with name length 0 loops forever |
| `isofs/cdvdman locatefile xa` | `cdvdman_locatefile()` | extended attribute length >= 0x80 steps backwards |
| `cdvdman locatefile long` | in-game `cdvdman_locatefile()` | a name over 14 characters overflows `cdvdman_curdir[15]` |
| `isofs findfile` | `cdvdman_findfile()` | `/`-only path copies from address 1; long names overflow `name[16]` |
| `zso align` | `ziso_init()`, `read_raw_data()` | align > 11 divides by zero |
| `ps2cnf cnf_no_eq` | `ps2cnfGetBootFile()` | `BOOT2` without `=` passes NULL to `CNFGetKey()` |
| `sbReadList` | `src/supportbase.c` | a ul.cfg size that is not a multiple of 64 writes past the list |
| `updateISOGameList` | `src/supportbase.c` | compares `games[i]` in the `j` loop, reads past the cache |
| `isValidIsoName` | `src/supportbase.c` | reads before the name for `..` |
| `USBA_crc32` | `src/system.c` | non-ASCII titles index `crctab` negatively; `ref` is an independent unsigned implementation |

## Console test files

`make_console_kit.py <bootable.iso> <out dir>` turns one bootable image (for example a
homebrew ELF packed as a disc) into the same malformed cases for a real PS2: ISO and ZSO
variants for `DVD/`, two `ul.cfg` sets and three `.cht` files. `iso_harness` can check an
image directly: `iso-<tag>.exe file <image>`.
