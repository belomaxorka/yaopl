#!/bin/bash

## Fetch the ps2sdk USBD sources from before the "USBD feature update" (ps2sdk b1f7ff96, merged 2024-09-24).
## OPL embeds a usbd_mini.irx built from them (modules/usb/usbd_mini): the rewritten USBD of today's
## ps2sdk hangs some games on USB (upstream issue #1751, ps2sdk issue #908). See FORK.md.
REPO_URL="https://github.com/ps2dev/ps2sdk.git"
REPO_FOLDER="modules/usb/usbd_mini/ps2sdk"
COMMIT="2dc6b32f4ce43593609d895207c720fc6e02e0e4"
if test ! -d "$REPO_FOLDER/.git"; then
  # Only iop/usb/usbd of that one commit: a sparse, shallow, blobless fetch of about 0.5 MB.
  git init -q "$REPO_FOLDER" || { exit 1; }
  (cd "$REPO_FOLDER" && git remote add origin "$REPO_URL" && git sparse-checkout set iop/usb/usbd) || { exit 1; }
fi
if test "$(cd "$REPO_FOLDER" && git rev-parse HEAD 2>/dev/null)" != "$COMMIT" || test ! -f "$REPO_FOLDER/iop/usb/usbd/src/usbd.c"; then
  (cd "$REPO_FOLDER" && git fetch -q --depth 1 --filter=blob:none origin "$COMMIT" && git checkout -q "$COMMIT") || { exit 1; }
fi
