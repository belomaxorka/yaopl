# Network setup

The [SMB share](smb.md), [UDPBD](udpbd.md), the [NBD server](../features/nbd.md) and
[Network Update](../game-settings.md#network-update) use the PS2's Ethernet port: built into slim models, or the
network adapter in the expansion bay of a fat model. Connect it to your router or switch with a cable.

## Network Settings

Main menu → **Network Settings**. The top part is the PS2 itself; the *SMB Server* part is for the
[SMB share](smb.md).

| Field | Default | What to enter |
| --- | --- | --- |
| Advanced Options | Off | Turn on to change *Ethernet Link Mode* and the SMB *Port* |
| Ethernet Link Mode | Auto | Auto, or force 100 Mbit / 10 Mbit, full or half duplex. Leave Auto unless the link fails. |
| IP Address Type | DHCP | DHCP gets an address from the router. Static uses the fields below. |
| IP Address | 192.168.0.10 | The PS2's address (static only) |
| Mask | 255.255.255.0 | The subnet mask of your LAN |
| Gateway | 192.168.0.1 | Your router |
| DNS Server | 192.168.0.1 | Your router or another DNS server; only Network Update needs it |

Choose **OK** to apply, then **Save Changes** in the main menu to keep the settings (they are stored in
`conf_network.cfg`).

!!! note "Static or DHCP"
    SMB, NBD and Network Update work with DHCP or a static address. **UDPBD needs a static address**: it
    ignores DHCP. Pick an address on your LAN that no other device uses, outside the range your router hands
    out by DHCP.

Your LAN is often `192.168.1.x` instead of `192.168.0.x`: check the address of your PC (`ipconfig` on Windows,
`ip addr` on Linux) and use the same first three numbers.

## Network errors

| Message | Meaning |
| --- | --- |
| `200: No network adaptor detected.` | No network adapter found: on a fat PS2, check that the adapter is seated. |
| `310: No network connection.` | No link after 30 seconds: check the cable and the router port, or try a fixed *Ethernet Link Mode*. |
| `311: DHCP server unavailable.` | No answer from DHCP: check the router, or use a static address. |
| `202: The network adapter is used by UDPBD...` | UDPBD is on, so SMB, NBD and Network Update cannot start. See [UDPBD](udpbd.md#limits). |
| `<n>: Network startup error.` | Another startup error; note the number when reporting it. |

The SMB errors (300–305) are on the [SMB page](smb.md#errors).
