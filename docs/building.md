# Building from source

YAOPL builds with the ps2dev toolchain image pinned in `.github/workflows/compilation.yml` (`TOOLCHAIN_IMAGE`):

```sh
git clone https://github.com/belomaxorka/yaopl.git && cd yaopl
docker run --rm -v "$PWD:/src" -w /src <TOOLCHAIN_IMAGE> sh -c 'git config --global --add safe.directory /src && make release'
```

`make` builds the ELF, `make release` also a ZIP of it with `LICENSE` and `CREDITS`. Use a full clone: the
revision in the version counts the commits.

The options are at the top of the `Makefile`:

| Option | Default | Effect |
| --- | --- | --- |
| `PADEMU` | 1 | DualShock 3/4 pad emulation and pad macros |
| `EXTRA_FEATURES` | 0 | Sets `RTL` (right-to-left languages) and `IGS` (in-game screenshots) |
| `RTL`, `IGS` | `EXTRA_FEATURES` | Each one on its own |
| `DTL_T10000` | 0 | A build for the DTL-T10000 development kit |
| `NOT_PACKED` | 0 | Do not strip and compress the ELF |
| `LOCALVERSION` | `yaopl` | The suffix of the version |

For example `make PADEMU=0 EXTRA_FEATURES=1`. `make format` formats the code with clang-format 12. The debug
builds (`make debug`, `make iopcore_debug`, ...) are listed in the `Makefile`.

## This documentation

The site is built from `docs/` and `mkdocs.yml` with [Zensical](https://zensical.org/):

```sh
pip install zensical==0.0.70
zensical serve
```

How changes to YAOPL are made and checked is in [CONTRIBUTING.md](https://github.com/belomaxorka/yaopl/blob/master/CONTRIBUTING.md).
