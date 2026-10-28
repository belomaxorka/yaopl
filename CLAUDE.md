# CLAUDE.md — YAOPL agent rules

YAOPL (Yet Another Open PS2 Loader) is our fork of Open PS2 Loader that we develop ourselves. Users install the
ELF that CI publishes from `master`. Its PC-side companion is YAOPLM (Yet Another OPL Manager), a separate project.

Read before working: `README.md`, `CONTRIBUTING.md` (upstream's code rules and how this fork works),
`CHANGELOG.md` (what differs from upstream), `docs/` (the user documentation site, `mkdocs.yml`),
`labs/host-repro/README.md`.
If the local knowledge base `kb/` exists, start with `kb/README.md` and `kb/ROADMAP.md`.

## Knowledge base
- `kb/` is local only: it is listed in `.git/info/exclude` and must never be committed or pushed, to our GitHub or
  upstream. Do not `git add -f` it.
- Keep it current: `ROADMAP.md` (what is next), `DECISIONS.md` (new decisions as ADRs), `HISTORY.md` (major steps),
  `TESTING.md`, and `research/` for findings. Core docs are in Russian, `research/` in English.

## Communication
- Talk to the user in **Russian**. They own the project and test on a real PS2.
- Write code, comments, commit messages and public docs in **English**.
- After a set of fixes, give the user a short Russian checklist of what to test on the console.

## Rules
1. **Licence.** YAOPL is AFL-3.0 like OPL. Never copy code from YAOPLM (GPL-3.0) into YAOPL, or the other way.
2. **`master` is the release.** Every push to `master` publishes the public `latest` pre-release. Push only
   finished, checked work. Prepare bigger changes on a short-lived branch and delete it after the merge.
   Only the build of master's tip is published: a commit that does not change the ELF (`labs/`) may say `[skip ci]`,
   but push it only after the running build has published, or that build is not released. A commit that
   changes `docs/` must not say `[skip ci]`: it would skip the docs site workflow too.
3. **Keep the docs current.** Every user-visible change adds a line to `CHANGELOG.md` in the same push (see
   "Changelog" below). Update the pages in `docs/` when a change affects how YAOPL is used, `README.md` if
   what YAOPL is or how to get it changes, and `CONTRIBUTING.md` if how the fork works changes.
4. **One fix per commit.** Subject `<component>: <summary>`. The body states the problem, the fix, its origin
   (credit the RiptOPL commit for ported fixes) and the reproduction with the result before and after.
5. **Check every fix** (details in `CONTRIBUTING.md`, "How fixes are checked"):
   - reproduce on an upstream tree first (`labs/host-repro/run_all.ps1 -Opl <tree>`, or a console test);
   - rerun the same check after the fix, then all checks for regressions;
   - format with clang-format 12 (`make format`, or the `clang-format` 12.0.1 Python wheel on Windows);
   - the GitHub Actions build must pass; then the user tests on the console.
   Host checks do not emulate the PS2. Say so when a fix is only checked on the host or only by reading code.
6. **RiptOPL and other forks are sources of candidates, not truth.** Check each change against upstream code:
   take real bugs and zero-risk hardening, in the smallest form; skip speculative or behaviour-changing ones.
7. **Small, upstream-shaped diffs**, so merging upstream stays easy. Fix GCC warnings.
8. **Upstream contributions** (issues, PRs to ps2homebrew) only when the user asks.
9. **Repository settings** (Actions, Pages, branch rules, releases) and anything posted publicly: ask first.
10. **User data and devices.** Never write to the user's removable drives without a plan and an explicit OK.
    Test data lives in `D:\yaoplm-testdata\` (reference clones in `refs\`, console test files in
    `opl-fork-tests\`); write on D: only there.

## Changelog
- `CHANGELOG.md` lists what changed for users, newest first: YAOPL sections by the day a change reaches
  `master` (`## YAOPL YYYY-MM-DD`, the commit's local date as `git log --date=short` shows it), then
  upstream sections by version. The release notes of `latest` list the YAOPL entries added or changed
  since the previous published build (`.github/changelog.sh since`); the release packages do not carry it.
- Every push to `master` that changes what users get (ELF, packages, behaviour, settings, supported
  devices) adds lines under that day's heading, creating it at the top if needed, in the right `###` area:
  one line per change, user-facing words, the commit hash(es) at the end. On a branch, write the expected
  day and fix it when merging. Wrap at 110 columns like the rest of the file.
- An upstream merge adds a line per user-visible upstream PR (`#N`, upstream `Beta-N`) under the merge's
  day, in an "Upstream" area, and notes which fork changes it touches.
- `CI-compile` fails a push that changes code but not `CHANGELOG.md` (`.github/changelog.sh check`).
  Docs, `labs/`, `.github/` and repository files are exempt; for anything else that changes nothing for
  users, add a line `[no changelog]` to a commit message.

## Commands
- Host checks: `powershell -ExecutionPolicy Bypass -File labs\host-repro\run_all.ps1 [-Opl <tree>] [-Only cheat,cfg,...] [-Images <dir of .iso>]`
- Console test files: `python labs/host-repro/make_console_kit.py <bootable.iso> <out dir>`
- CI: `gh run list -R yaopl/yaopl`, `gh workflow run compilation.yml -R yaopl/yaopl --ref master`
- Upstream sync: see `CONTRIBUTING.md`, "Pulling in upstream changes".
