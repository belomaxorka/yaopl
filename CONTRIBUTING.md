
# Contributing

* Fix or suppress all warnings issued by GCC and cppcheck  
* Format the code using `make format`  

## YAOPL

YAOPL is a fork of [Open PS2 Loader](https://github.com/ps2homebrew/Open-PS2-Loader). The rules above are
upstream's and apply here too; this fork adds the following. [FORK.md](FORK.md) explains what YAOPL changes
and how fixes are checked.

* `master` is the release: every push to it publishes the `latest` pre-release. Open pull requests against
  `master` from a short-lived branch.
* One fix per commit. The subject is `<component>: <summary>` (for example `cheats: ...`, `bdm: ...`); the
  body states the problem, the fix, its origin (credit the commit or PR you port, for example from RiptOPL
  or upstream) and the reproduction with the result before and after the fix.
* Reproduce a bug on an upstream tree first, then show it is gone. `labs/host-repro/` has host checks for
  many parsers (`run_all.ps1`); they do not emulate the PS2, so say whether a fix was tested on a console,
  only on the host or only by reading the code.
* Keep diffs small and close to upstream's code, so that upstream changes can still be merged.
* Add a line to `CHANGELOG.md` for every change users see (see the top of that file), and a row to the
  table in `FORK.md` for every fix or behaviour change. CI fails a push that changes code without a
  `CHANGELOG.md` line; for a change users do not see, add a line `[no changelog]` to the commit message.
* Format with clang-format 12 exactly, as `CI-format-check` does; other versions format differently.
* YAOPL is licensed under AFL-3.0 like OPL. Do not copy code from projects under an incompatible licence,
  such as GPL.
* Bugs that official OPL has too can be reported to upstream as well.
