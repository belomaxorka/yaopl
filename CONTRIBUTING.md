
# Contributing

* Fix or suppress all warnings issued by GCC and cppcheck  
* Format the code using `make format`  

## YAOPL

YAOPL is a fork of [Open PS2 Loader](https://github.com/ps2homebrew/Open-PS2-Loader). The rules above are
upstream's and apply here too; this fork adds the following. [CHANGELOG.md](CHANGELOG.md) lists what YAOPL
changes.

* `master` is the release: every push to it publishes the `latest` pre-release. Open pull requests against
  `master` from a short-lived branch.
* One fix per commit. The subject is `<component>: <summary>` (for example `cheats: ...`, `bdm: ...`); the
  body states the problem, the fix, its origin (credit the commit or PR you port, for example from RiptOPL
  or upstream) and the reproduction with the result before and after the fix.
* Reproduce a bug on an upstream tree first, then show it is gone (see "How fixes are checked" below). Say
  whether a fix was tested on a console, only on the host or only by reading the code.
* Keep diffs small and close to upstream's code, so that upstream changes can still be merged.
* Add a line to `CHANGELOG.md` for every change users see (see the top of that file). CI fails a push that
  changes code without a `CHANGELOG.md` line; for a change users do not see, add a line `[no changelog]` to
  the commit message. Update `docs/USAGE.md` when a change affects how YAOPL is used.
* Format with clang-format 12 exactly, as `CI-format-check` does; other versions format differently.
* YAOPL is licensed under AFL-3.0 like OPL. Do not copy code from projects under an incompatible licence,
  such as GPL.
* Bugs that official OPL has too can be reported to upstream as well.

## Builds and versions

Every push to `master` builds YAOPL in GitHub Actions and replaces the `latest` pre-release: the ELF
(`OPNPS2LD.ELF`), the 7z packages and `SHA256SUMS`. Only the build of the current tip of `master` is
published. The release notes list the `CHANGELOG.md` entries added since the previous published build. The
7z packages hold the ELFs with `LICENSE` and `CREDITS` only: `CREDITS` carries the MIT notice that the
embedded MMCE modules require to go with every copy. A commit that does not change the ELF may say
`[skip ci]`, but push it only after the running build has published, or that build is not released.

Versions carry the suffix `-yaopl` (`LOCALVERSION` in the `Makefile`), for example
`v1.2.0-Beta-2310-1a2b3c4-yaopl`. The revision number counts this repository's commits, so it does not match
upstream's numbering.

The logo (`gfx/logo.png`, shown on the greeting screen, and `docs/logo.svg`) is drawn by
`labs/logo/make_logo.py`; change the script and rerun it rather than editing the images.

## How fixes are checked

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

## Fixes from other forks

RiptOPL ([NathanNeurotic/Open-PS2-Loader](https://github.com/NathanNeurotic/Open-PS2-Loader) and its C++ edition
[NathanNeurotic/RiptOPL](https://github.com/NathanNeurotic/RiptOPL)) and other forks are sources of candidates,
not of truth. Each change is reviewed against upstream's code: only those that fix a defect present in
upstream, or harden code at no risk, are taken, in the smallest form, and each commit credits the commit it
ports. Speculative or behaviour-changing changes are left out.

## Modules built from other sources

Some modules are not taken from ps2sdk but built from other trees, which `download_*.sh` fetch:

* **USBD** (`modules/usb/usbd_mini`, `download_usbd.sh`): built from the ps2sdk USBD sources from before its
  rewrite (ps2sdk b1f7ff96, upstream Beta-2127). With the rewritten USBD, Crash Bandicoot: The Wrath of Cortex
  takes minutes to load from USB and then black-screens (upstream issue #1751, ps2sdk issue #908). Drop both
  once ps2sdk fixes its USBD.
* **MMCE** (`mmceman`, `mmcedrv`, `mmceigr`, `download_mmce.sh`): built from `ps2-mmce/mmceman` (MIT, see
  `CREDITS`). MMCE games get the SDK `sio2man`, the pairing that the `mmcedrv` built here (mmceman cccc366
  and later) is written for. `make MMCE_NO_SDK_SIO2MAN=1` (`-nosio2man` in the version) gives MMCE games their
  own SIO2MAN instead, for a console A/B with PadEmu off; RiptOPL does that with the older mmcedrv of mmceman
  v2.1.1, so this build is a comparison, not a fix.
* **UDPBD** (`smap`, `ministack`, `udpbd`, `download_udpbd.sh`): built from `rickgaiser/neutrino` (AFL-3.0, see
  `CREDITS`). In game they run with `bdm_udp_cdvdman.irx` (BDM with DEV9), and the game's DEV9 and SMAP modules
  are faked. The driver asks for at most 16 KiB per request (neutrino: 256 KiB), because longer bursts of
  reply packets were lost on a real console and hung the game. The ATA driver resets the DEV9's ATA interface
  when it starts, which cuts UDPBD off if it is already running, so with the APA HDD page enabled it is loaded
  before UDPBD.

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
Before pushing a merge, run `labs/host-repro/run_all.ps1` if upstream touched the files it covers. Upstream's
`README.md` was replaced by YAOPL's and `docs/USAGE.md`: port changes to its user documentation by hand.

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
4. Merge into `master`. Add a line to `CHANGELOG.md` only if the update changes what users see.
