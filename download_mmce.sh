#!/bin/bash

## Fetch the MMCE IOP modules (mmceman, mmcedrv, mmceigr) of ps2-mmce/mmceman at a pinned commit: OPL builds
## and embeds them for the MMCE device (SD2PSX, MemCard PRO 2 and other memory-card emulators). The modules
## are MIT licensed, see CREDITS; FORK.md says why this fork carries them.
REPO_URL="https://github.com/ps2-mmce/mmceman.git"
REPO_FOLDER="modules/mmce/mmceman"
COMMIT="de28936fdf45fe445554cfa0b8f058b05fa8212f"
if test ! -d "$REPO_FOLDER/.git"; then
  # Only the three module directories of that one commit: a sparse, shallow, blobless fetch.
  git init -q "$REPO_FOLDER" || { exit 1; }
  (cd "$REPO_FOLDER" && git remote add origin "$REPO_URL" && git sparse-checkout set mmceman mmcedrv mmceigr) || { exit 1; }
fi
if test "$(cd "$REPO_FOLDER" && git rev-parse HEAD 2>/dev/null)" != "$COMMIT" || test ! -f "$REPO_FOLDER/mmceman/src/mmceman.c"; then
  (cd "$REPO_FOLDER" && git fetch -q --depth 1 --filter=blob:none origin "$COMMIT" && git checkout -q "$COMMIT") || { exit 1; }
fi
