# YAOPL and upstream OPL

YAOPL (Yet Another Open PS2 Loader) is a fork of [Open PS2 Loader](https://github.com/ps2homebrew/Open-PS2-Loader)
that continues its development. It follows upstream and adds fixes for crashes, hangs, memory errors and data
loss that OPL hits on malformed or unusual files: cheat files, ISO and ZSO images, `SYSTEM.CNF`, `ul.cfg`,
`games.bin`, configuration files and VMCs. It is not an official OPL build. The licence is the same as
upstream's (AFL-3.0, see `LICENSE`).

## Getting a build

Every push to `master` builds OPL in GitHub Actions and replaces the `latest` pre-release of this
repository: the ELF (`OPNPS2LD.ELF`), the upstream-style 7z packages and `SHA256SUMS`. Check a
download with `sha256sum -c SHA256SUMS` (Linux) or compare `Get-FileHash <file>` (Windows).
Only the build of the current tip of `master` is published.

YAOPL builds carry the suffix `-yaopl` in their version string (`LOCALVERSION` in the `Makefile`),
for example `v1.2.0-Beta-2310-1a2b3c4-yaopl`. The revision number counts this fork's commits, so it
does not match upstream's numbering.

The build uses a pinned toolchain image (ps2sdk, gsKit, ps2sdk-ports), so the same commit always gets the
same SDK modules; the release notes name the ps2sdk commit. See "Updating ps2sdk" below.

## What differs from upstream

| Area | Fix | Origin |
|---|---|---|
| Cheats | A comment line with leading blanks no longer becomes a cheat with an empty name | this fork |
| Cheats | Cheat names in Cyrillic, Japanese and other non-ASCII scripts are kept: a name without ASCII letters was dropped and its codes joined the cheat before it, often the always-on Mastercode | this fork |
| Cheats | The 251st cheat is no longer written past the cheat table | this fork; overflow found by RiptOPL |
| Cheats | `make_code()` no longer writes one byte past `digits[]` for every code | RiptOPL |
| Cheats | A last line without LF is no longer read past the end of the buffer or copied past `line[]` | RiptOPL |
| Cheats | Picks of cheats that share a name are no longer lost on a reload: as many of them are picked again (the file stores names only, not which of them) | RiptOPL |
| ISO | A directory record with an empty name no longer hangs the game list or the game | this fork |
| ISO | The record length is read as one byte; an extended attribute length no longer derails the scan | this fork |
| ISO | In-game lookups no longer overflow `cdvdman_curdir[15]` on names longer than 14 characters | this fork |
| ISO | `cdvdman_findfile()` no longer copies from address 1 or past the 16-byte name | RiptOPL (isofs), this fork (in-game) |
| ZSO | A header with `align` above 11 is rejected instead of dividing by zero | this fork |
| In-game | 2340-byte sector reads no longer write past the game's buffer | RiptOPL |
| `SYSTEM.CNF` | `BOOT2` without `=` no longer passes NULL to the parser | RiptOPL C++ edition |
| `ul.cfg` | A partial record at the end is ignored instead of written past the game list | RiptOPL C++ edition |
| `ul.cfg` | `USBA_crc32()` hashes names as unsigned bytes (non-ASCII titles), also in `pc/` tools | RiptOPL |
| `games.bin` | Each listed game is looked up in the whole cache, without reading past it | this fork |
| Game list | `isValidIsoName()` no longer reads before short names such as `..` | RiptOPL |
| Config files | A CFG, theme or language file whose last line is a `#` comment without LF no longer crashes OPL | this fork |
| Config files | A read error (device removed) ends the file instead of corrupting memory | RiptOPL |
| Config files | Keys of 32+ and values of 256+ characters are cut and terminated | RiptOPL |
| Config files | A long line without `=` no longer overflows the prefix buffer | RiptOPL |
| Settings | A failed save (e.g. full memory card) is reported instead of "Settings saved" | RiptOPL |
| Settings | Looking for a USB device to save to no longer empties its `conf_opl.cfg` (the fallback to USB that upstream issue #1397 reports runs this probe) | this fork; RiptOPL has the same change |
| Themes | A colour with more than three bytes (`#RRGGBBAA`) no longer writes past the colour | RiptOPL |
| Network | An `IPCONFIG.DAT` of 256 bytes or more no longer overflows the stack | RiptOPL |
| VMC | Leaving the VMC menu no longer replaces a card of another size (4 MB, 128 MB, PCSX2 `.ps2`); Modify asks first | this fork, after RiptOPL |
| VMC | VMC slots are found again after an aborted launch, so a game cannot use the previous game's VMC | RiptOPL |
| BDM | All parts of a UL game must fit the fragment table together | RiptOPL |
| BDM | One VMC error is no longer reported again for the other slot | RiptOPL |
| BDM | Returning from the NBD server no longer crashes | RiptOPL |
| HDD | The PS2 logo check reads 48 sectors, not 49 | RiptOPL |
| Misc | Format string, terminators and the IOP patch mode compare (hardening, no known trigger) | RiptOPL |
| Cheats | A code line with address 0 (the second half of a multi-line code) no longer ends the cheat (upstream issue #1548) | this fork |
| Cheats | More than 5 hooks or 250 codes are dropped instead of written past the engine's tables (`SLUS_210.09.cht` has 9 hooks) | this fork |
| Themes | `fntFitString()` stops at the end of the string instead of scanning and writing past it (upstream issue #1713) | this fork |
| Themes | Wrapped text is a private copy: line breaks no longer go into the game's CFG (upstream issue #1713) | RiptOPL (same fix) |
| Themes | `GameCountText` gets the 60 bytes it formats into, not 1 | this fork; RiptOPL has the same fix |
| Themes | An image file that is missing disables the element instead of leaving a garbage texture that is drawn and freed | this fork; RiptOPL has the same fix |
| Themes | A theme without an `ItemsList` keeps the default list, instead of a NULL that crashes the first scroll | this fork; RiptOPL has the same fix |
| Languages | A `.lng` file name shorter than `lang_X.lng` (for example `ru.lng`) no longer crashes OPL at boot | this fork; RiptOPL checks the prefix |
| Launch | A boot file name over 16 characters (`$AltStartup`) no longer overflows the OSD history buffer | this fork |
| Settings | The default BGM path is no longer used as a format string | this fork |
| GSM | A `$GSMVMode` outside the table of modes falls back to mode 0 instead of reading past the table | RiptOPL |
| Pad | A held button keeps repeating across the 29 s wrap of the tick counter | RiptOPL (upstream PR #1757) |
| VMC | A failed cluster write while creating a VMC no longer leaves genvmc's abort semaphore taken (the next creation hung) | RiptOPL, smaller form |
| GUI | The scroll speed is restored after dialogs that return through their updater (VMC menu) | upstream PR #1757 |
| Art | Grey and grey+alpha PNGs are loaded | wOPL |
| ZSO | Images with a block size other than 2048 (maxcso default for 2 GiB+ images: 16 KiB) are rejected instead of read as the wrong sectors | this fork |
| ZSO | The block index is checked before the compressed data is read: a corrupt index gives a read error, not a write before the buffer | this fork |
| ZSO | Reads of more than 256 sectors go in slices instead of running past the index cache | this fork |
| In-game | IGR is sampled from the VBLANK_END interrupt handler again, not by a polling thread that busy games starve (upstream issue #1787); Fatal Fury: Battle Archives Vol. 2 needs Mode 6 as before | revert of upstream PR #1762, as RiptOPL 38481e9c |
| USB | `usbd_mini` is built from the ps2sdk USBD sources from before the "USBD feature update" (ps2sdk b1f7ff96, OPL Beta-2127): with the rewritten USBD, Crash Bandicoot: The Wrath of Cortex takes minutes to load from USB and then black-screens (upstream issue #1751, ps2sdk issue #908). `download_usbd.sh` fetches the sources, `modules/usb/usbd_mini` builds them; drop both once ps2sdk fixes its USBD | upstream PR #1752 (prebuilt binaries), ps2sdk PR #911 |
| MMCE | A new device, MMCE (Settings → MMCE Settings): games in `CD` and `DVD` on an MMCE memory-card emulator (SD2PSX, MemCard PRO 2, PSXMemCard Gen2) are listed and started, the ISO streamed in game by `mmcedrv` over SIO2. For a game from any device the card gets the game ID and switches to the game's own card ("Send the game ID", on by default), and IGR switches it back to its boot card (IGR slot "Auto": the cards that answer a ping); both only while the MMCE device is on (Auto or Manual), so a console without an MMCE card gets no MMCE commands. Autolaunch uses the same settings. Not yet: VMCs on the card, games split into parts. The modules `mmceman`, `mmcedrv` and `mmceigr` are built from `ps2-mmce/mmceman` (MIT, see `CREDITS`) by `download_mmce.sh`. MMCE games get the SDK `sio2man`, the pairing the `mmcedrv` built here (mmceman cccc366 and later) is written for. `make MMCE_NO_SDK_SIO2MAN=1` (`-nosio2man` in the version) gives MMCE games their own SIO2MAN instead, for a console A/B with PadEmu off; RiptOPL does that with the older mmcedrv of mmceman v2.1.1, and its own audit expects the pairing with the newer mmcedrv to fail, so this build is a comparison, not a fix | ps2-mmce/Open-PS2-Loader `mmce_support` (qnox32, BBsan), hardening after RiptOPL 09618a06, b5df7836 |
| UDPBD | A new BDM device, UDPBD (Settings → BDM Devices → UDPBD, off by default): a disk or disk image that a PC serves over the LAN (udpbd-server, udpbd-vexfat, PS2-Servers) is listed as "UDPBD Games" and its games run in the OPL core with all its features. The PS2 uses the static IP of the network settings (no DHCP); the server is found by broadcast, so it must be on the same LAN and running before OPL loads the driver. SMB and UDPBD cannot share the network adapter: while UDPBD is on, SMB and the other network features do not start. The driver asks for at most 16 KiB per request (upstream neutrino: 256 KiB), because longer bursts of reply packets were lost on a real console and hung the game. Per-game settings and VMCs are stored on the UDPBD disk, so they need a server that can write (udpbd-vexfat is read-only). The ATA driver resets the DEV9's ATA interface when it starts, which cuts UDPBD off if it is already running, so with the HDD (APA) page enabled it is loaded before UDPBD. Known limits: turning the HDD page on while UDPBD runs needs a restart of OPL; a game that starts its own network stack can hang (Gran Turismo 4 with its LAN modules). The modules `smap`, `ministack` and `udpbd` are built from `rickgaiser/neutrino` (AFL-3.0, see `CREDITS`) by `download_udpbd.sh`; in game they run with the new `bdm_udp_cdvdman.irx` (BDM with DEV9), and the game's DEV9 and SMAP modules are faked | this fork; approach of tihmstar's OPL `udpbd` branch and uOPL, modules by Maximus32 |
| In-game | The SDK `sio2man` goes into a game only for the modules that use it (a VMC, PadEmu, MX4SIO, MMCE); other games keep their own SIO2MAN and pad and memory-card drivers on it, as before upstream Beta-2248 (PR #1785, which loaded it for every game). Suspected cause of games that freeze after some minutes while the sound goes on (upstream issues #1795, #1790); not yet confirmed on a console | RiptOPL 192716a5 (which also takes MMCE off the SDK `sio2man`; here MMCE keeps it) |

RiptOPL is [NathanNeurotic/Open-PS2-Loader](https://github.com/NathanNeurotic/Open-PS2-Loader) and its C++ edition
[NathanNeurotic/RiptOPL](https://github.com/NathanNeurotic/RiptOPL). Its fixes were reviewed one by one against
upstream: only those that fix a defect present in upstream were taken, some in a smaller or corrected form. Each
commit credits the RiptOPL commit it ports.

**Compatibility note:** a `ul.cfg` game whose title has non-ASCII bytes now uses the CRC that PC tools
compute. Upstream's CRC for such titles depended on the build, so chunks named by upstream OPL itself
may need renaming.

## How fixes are checked

Each fix is a separate commit whose message states the problem, the fix, its origin and the reproduction.

1. Reproduce the bug on an upstream tree, with a host check (`labs/host-repro/run_all.ps1 -Opl <tree>`) or,
   for code that only runs on the console, a console test.
2. Fix it, then rerun the same check on this tree: the bug must be gone.
3. Rerun all host checks, and the real data where it applies (cheat corpus, ISO fixtures), for regressions.
4. Format with clang-format 12, as `CI-format-check` does.
5. The GitHub Actions build (ps2dev toolchain) must pass; then test on a console.

`labs/host-repro/` holds the host checks. They compile OPL's own functions, taken from the tree at build
time, on Windows with MSVC and AddressSanitizer, with stubs in place of PS2 I/O. They show logic errors and
memory errors on the given inputs; they do not emulate the PS2, so a console test remains the final check.
`make_console_kit.py` turns a bootable ISO into the malformed test files for a console.

## Branches

`master` is the only long-lived branch: the releases are built from it. A fix may be prepared on a
short-lived branch, which is deleted once it is merged into `master`.

The branches `copilot/*`, `main`, `master_old` and `v1.1.z` came from upstream when the fork was
created; this fork does not use them. Upstream's website branch `gh-pages` was deleted and GitHub Pages
is off, so no copy of the official OPL site is published here.

## Pulling in upstream changes

```
git remote add upstream https://github.com/ps2homebrew/Open-PS2-Loader.git   # once
git fetch upstream
git switch master
git merge upstream/master        # resolve conflicts, keeping this fork's fixes
git push origin master           # CI builds and publishes the new pre-release
```

Do not use GitHub's "Sync fork" button with "Discard commits": it would drop this fork's fixes.
Before pushing a merge, run `labs/host-repro/run_all.ps1` if upstream touched the files it covers.

## Updating ps2sdk

OPL embeds about 30 ps2sdk modules, several of which run inside games (`udnl`, `sio2man`, the USB,
iLink and MX4SIO block device drivers), and links ps2sdk's libraries. Upstream builds with the newest
toolchain image, so a ps2sdk change reaches its builds unannounced; here the image is pinned by digest
in `.github/workflows/compilation.yml` (`TOOLCHAIN_IMAGE`, the `container:` lines and the ps2sdk commit
in `TOOLCHAIN_PS2SDK`).

`CI-sdk-canary` (weekly, or by hand) builds with the newest image and warns when it differs from the
pinned one, listing the embedded modules that changed. To move to it:

1. Read the ps2sdk commits since `TOOLCHAIN_PS2SDK` for the modules OPL embeds, in-game ones first.
2. On a short-lived branch, change only the digest and the ps2sdk commit, in one commit
   `ci: build with ps2sdk <commit>` whose body lists the relevant ps2sdk changes.
3. After CI, test on a console: boot, pads, memory card save, a USB game, SMB list, HDD, sounds and
   BGM, IGR, an MX4SIO game if there is one, and whatever the ps2sdk changes touch.
4. Merge into `master`. Add a row to the table above only if the update changes what users see.
