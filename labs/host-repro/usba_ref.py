"""Independent USBA_crc32 with unsigned bytes (the behaviour the fix should give)."""
import sys


def usba_crc32(name: bytes) -> int:
    tab = [0] * 256
    crc = 0
    for t in range(256):
        crc = (t << 24) & 0xFFFFFFFF
        for _ in range(8):
            if crc & 0x80000000:
                crc = (crc << 1) & 0xFFFFFFFF
            else:
                crc = ((crc << 1) ^ 0x04C11DB7) & 0xFFFFFFFF
        tab[255 - t] = crc
    data = (name + b"\0")[:33]
    for b in data:
        crc = tab[b ^ (crc >> 24)] ^ ((crc << 8) & 0xFFFFFF00)
    return crc


for a in sys.argv[1:]:
    print("%-24s %08X" % (a or "(empty)", usba_crc32(bytes.fromhex(a))))
