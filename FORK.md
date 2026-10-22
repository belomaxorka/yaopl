# About this fork

This is the YAOPLM project's build of [Open PS2 Loader](https://github.com/ps2homebrew/Open-PS2-Loader).
It follows upstream and adds fixes for crashes, hangs and memory errors that OPL hits on malformed
files: cheat files, ISO and ZSO images, `SYSTEM.CNF`, `ul.cfg`, `games.bin`. It is not an official
OPL build. The licence is the same as upstream's (AFL-3.0, see `LICENSE`).

## Getting a build

Every push to `master` builds OPL in GitHub Actions and replaces the `latest` pre-release of this
repository: the ELF (`OPNPS2LD.ELF`), the upstream-style 7z packages and `SHA256SUMS`. Check a
download with `sha256sum -c SHA256SUMS` (Linux) or compare `Get-FileHash <file>` (Windows).

Fork builds carry the suffix `-yaoplm` in their version string (`LOCALVERSION` in the `Makefile`),
for example `v1.2.0-Beta-2170-d4fb44f-yaoplm`. The revision number counts this fork's commits, so it
does not match upstream's numbering.

## What differs from upstream

| Area | Fix | Origin |
|---|---|---|
| Cheats | A comment line with leading blanks no longer becomes a cheat with an empty name | this fork |
| Cheats | The 251st cheat is no longer written past the cheat table | this fork; overflow found by RiptOPL |
| Cheats | `make_code()` no longer writes one byte past `digits[]` for every code | RiptOPL |
| Cheats | A last line without LF is no longer read past the end of the buffer or copied past `line[]` | RiptOPL |
| ISO | A directory record with an empty name no longer hangs the game list or the game | this fork |
| ISO | The record length is read as one byte; an extended attribute length no longer derails the scan | this fork |
| ISO | In-game lookups no longer overflow `cdvdman_curdir[15]` on names longer than 14 characters | this fork |
| ISO | `cdvdman_findfile()` no longer copies from address 1 or past the 16-byte name | RiptOPL (isofs), this fork (in-game) |
| ZSO | A header with `align` above 11 is rejected instead of dividing by zero | this fork |
| `SYSTEM.CNF` | `BOOT2` without `=` no longer passes NULL to the parser | RiptOPL C++ edition |
| `ul.cfg` | A partial record at the end is ignored instead of written past the game list | RiptOPL C++ edition |
| `games.bin` | Each listed game is looked up in the whole cache, without reading past it | this fork |
| Game list | `isValidIsoName()` no longer reads before short names such as `..` | RiptOPL |
| `ul.cfg` | `USBA_crc32()` hashes names as unsigned bytes (non-ASCII titles), also in `pc/` tools | RiptOPL |

Each fix is a separate commit with the reproduction in its message. RiptOPL is
[NathanNeurotic/Open-PS2-Loader](https://github.com/NathanNeurotic/Open-PS2-Loader) and its C++ edition
[NathanNeurotic/RiptOPL](https://github.com/NathanNeurotic/RiptOPL); the commits credit the RiptOPL commit
they port.

**Compatibility note:** a `ul.cfg` game whose title has non-ASCII bytes now uses the CRC that PC tools
compute. Upstream's CRC for such titles depended on the build, so chunks named by upstream OPL itself
may need renaming.

`labs/host-repro/` holds host programs (Windows, MSVC with AddressSanitizer) that run OPL's own parsing
functions on malformed input, and a generator of console test files. `run_all.ps1 -Opl <tree>` shows each
bug on an upstream tree and its absence here.

## Branches

`master` is the only long-lived branch: the releases are built from it. A fix may be prepared on a
short-lived branch, which is deleted once it is merged into `master`.

The branches `copilot/*`, `gh-pages`, `main`, `master_old` and `v1.1.z` came from upstream when the
fork was created; this fork does not use them.

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
