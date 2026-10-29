# Apps

The *Apps* page lists homebrew ELF programs, so you can start them from YAOPL. Turn it on with
*Applications Start Mode* in [Settings](../settings.md#settings).

## `title.cfg` (recommended)

Put each app in its own folder inside `APPS` on any enabled device, with a `title.cfg` next to the ELF:

```
title=My App Name
boot=MYAPP.ELF
```

- `title` is the name shown in the list (up to 128 characters), `boot` the ELF's file name in that folder. Both
  are required.
- An optional `argv1=` line passes one argument to the ELF.
- On a memory card there is no `APPS` folder: the app folders are in the root of `mc0:` or `mc1:`, and their
  names must contain `_` (for example `mc0:/APP_MYAPP/`).

## `conf_apps.cfg` (legacy)

One line per app, the name and the full path:

```
My App Name=mass:APPS/MYAPP.ELF
```

YAOPL reads `mc?:OPL/conf_apps.cfg` and `conf_apps.cfg` in the root (or prefix path) of each enabled device. A
`mass:` path is looked up on the first four BDM devices.

Names in both files are case sensitive.

## Managing apps

With *Write Operations* on in Settings, Triangle on an app offers:

- **Rename**: changes the `title=` line (or the name in `conf_apps.cfg`);
- **Delete**: deletes the app's whole folder (or, for a `conf_apps.cfg` entry, the ELF and its line).

An app on a read-only [UDPBD](../devices/udpbd.md#servers-that-write-and-read-only-servers) server has no
such menu: the server would drop the change.

Cover art for an app is named after its ELF file, for example `ART/MYAPP.ELF_COV.png`; see
[Art](customizing.md#art).
