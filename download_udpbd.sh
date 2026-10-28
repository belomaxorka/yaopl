#!/bin/bash

## Fetch the UDPBD IOP modules (smap, ministack, udpbd) of rickgaiser/neutrino at a pinned commit: OPL builds
## and embeds them for UDPBD, a block device served over UDP by a PC on the LAN. The modules are AFL-3.0
## licensed like OPL, see CREDITS; CONTRIBUTING.md says why this fork carries them.
REPO_URL="https://github.com/rickgaiser/neutrino.git"
REPO_FOLDER="modules/network/neutrino"
COMMIT="7be8de2798c99af433c91993e021746a54d6800d"
if test ! -d "$REPO_FOLDER/.git"; then
  # Only the three module directories, their shared header and iop/Rules.make: a sparse, shallow, blobless fetch.
  git init -q "$REPO_FOLDER" || { exit 1; }
  (cd "$REPO_FOLDER" && git remote add origin "$REPO_URL" && git sparse-checkout set iop/common iop/smap iop/ministack iop/udpbd) || { exit 1; }
fi
if test "$(cd "$REPO_FOLDER" && git rev-parse HEAD 2>/dev/null)" != "$COMMIT" || test ! -f "$REPO_FOLDER/iop/udpbd/src/udpbd.c"; then
  (cd "$REPO_FOLDER" && git fetch -q --depth 1 --filter=blob:none origin "$COMMIT" && git checkout -q -f "$COMMIT") || { exit 1; }
fi
# Ask for at most 32 sectors (16 KiB, 12 reply packets) per UDPBD read instead of 512. The server sends the reply
# packets back to back; on a console test a 64 KiB reply (47 packets from a 1 Gbit/s PC) lost packets every time and
# the game hung, while 12 and 24 packet bursts never did. 16 KiB is the size of the SMAP receive FIFO.
sed -i 's|^#define UDPBD_MAX_SECTOR_READ .*|#define UDPBD_MAX_SECTOR_READ  32 // OPL: see download_udpbd.sh|' "$REPO_FOLDER/iop/udpbd/src/udpbd.h" || { exit 1; }
grep -q "UDPBD_MAX_SECTOR_READ  32 " "$REPO_FOLDER/iop/udpbd/src/udpbd.h" || { echo "download_udpbd.sh: UDPBD_MAX_SECTOR_READ not found"; exit 1; }
# Repeat the INFO broadcast until the server answers: the driver sent it once, and when it went out before the link was
# up the game waited for the device forever after an IOP reset (console test, 1 in 6 launches).
PATCH="$PWD/modules/network/udpbd-info-retry.diff"
if ! (cd "$REPO_FOLDER" && git apply --reverse --check "$PATCH" 2>/dev/null); then
  (cd "$REPO_FOLDER" && git apply "$PATCH") || { echo "download_udpbd.sh: $PATCH does not apply"; exit 1; }
fi
