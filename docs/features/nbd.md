# NBD server

**Start NBD Server** in the main menu shares the PS2's internal HDD over the network as a
[Network Block Device](https://github.com/NetworkBlockDevice/nbd/blob/master/doc/proto.md), so PC tools
(`hdl_dump`, `pfs-shell`, a hex editor) can use it as if the drive were attached to the PC. It is the easy way
to install HDL games on an [APA HDD](../devices/hdd.md#apa) without taking it out. The server is based on
[lwNBD](https://github.com/bignaux/lwNBD).

## Start it

1. Set up the PS2's network address (static or DHCP) in [Network Settings](../devices/network.md).
2. To write to the disk (for example to install games), turn on *Write Operations* in
   [Settings](../settings.md#settings). Without it the disk is read-only.
3. Main menu → **Start NBD Server**. YAOPL stops its devices and shows *NBD Server running...* while the
   server runs.
4. Close that message to stop the server; the devices start again.

The export is named `hdd0` (set `nbd_default_export` in `conf_network.cfg` for another name). The NBD server
cannot start while [UDPBD](../devices/udpbd.md) is on: *Failed to start NBD Server.*

## Connect from a PC

| Client | Systems | Connect | Disconnect |
| --- | --- | --- | --- |
| `nbd-client` | Linux, WSL with a custom kernel | `nbd-client 192.168.1.45 /dev/nbd1` | `nbd-client -d /dev/nbd1` |
| `nbdfuse` | Linux, WSL2 | `mkdir ps2 && nbdfuse ps2/ nbd://192.168.1.45 &` | `umount ps2` |
| [WNBD](https://cloudbase.it/ceph-for-windows/) | Windows (elevated PowerShell) | `wnbd-client.exe map hdd0 192.168.1.45` | `wnbd-client.exe unmap hdd0` |

Replace `192.168.1.45` with the PS2's address. `nbd-client -l <ip>` or `nbdinfo --list nbd://<ip>` lists the
exports; `nbd-client` needs `modprobe nbd`. For example, to install a game with `hdl_dump` while connected
through `nbdfuse`:

```sh
hdl_dump inject_dvd ps2/nbd "Test Game" ./TEST.ISO
```

macOS has no client.
