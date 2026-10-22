"""Build console test files for the parser fixes from one bootable PS2 ISO.

Usage: make_console_kit.py <bootable.iso> <out dir>

The ISO must have SYSTEM.CNF;1 in its root and fit its root directory in one sector, e.g. a
homebrew ELF packed as a disc. Every variant keeps that ISO's game ID. Cheat files only use
codes with address 0, which OPL parses but never sends to the cheat engine.
"""
import os
import shutil
import struct
import sys

SECTOR = 2048


def usba_crc32(name: bytes) -> int:
    """USBA_crc32 with unsigned bytes, as OPL computes it after the fix."""
    tab = [0] * 256
    crc = 0
    for t in range(256):
        crc = (t << 24) & 0xFFFFFFFF
        for _ in range(8):
            crc = (crc << 1) & 0xFFFFFFFF if crc & 0x80000000 else ((crc << 1) ^ 0x04C11DB7) & 0xFFFFFFFF
        tab[255 - t] = crc
    for b in (name + b"\0")[:33]:
        crc = tab[b ^ (crc >> 24)] ^ ((crc << 8) & 0xFFFFFF00)
    return crc


def root_dir(iso: bytearray):
    pvd = 16 * SECTOR
    assert iso[pvd] == 1 and iso[pvd + 1 : pvd + 6] == b"CD001"
    lba, size = struct.unpack_from("<I", iso, pvd + 156 + 2)[0], struct.unpack_from("<I", iso, pvd + 156 + 10)[0]
    assert size == SECTOR, "root directory must be one sector"
    records, pos = [], lba * SECTOR
    while iso[pos] != 0:
        records.append(bytes(iso[pos : pos + iso[pos]]))
        pos += iso[pos]
    return lba, records


def record(name: bytes, lba: int, size: int, xa: int = 0) -> bytes:
    length = 33 + len(name) + (1 - len(name) % 2)  # ISO 9660 9.1.12: pad to an even length
    r = bytearray(length)
    r[0], r[1] = length, xa
    struct.pack_into("<I", r, 2, lba)
    struct.pack_into(">I", r, 6, lba)
    struct.pack_into("<I", r, 10, size)
    struct.pack_into(">I", r, 14, size)
    r[32] = len(name)
    r[33 : 33 + len(name)] = name
    return bytes(r)


def with_record_after_dotdot(base: bytes, extra: bytes) -> bytes:
    """Insert a record right after '.' and '..', before SYSTEM.CNF;1, as OPL scans in order."""
    iso = bytearray(base)
    lba, records = root_dir(iso)
    sector = b"".join(records[:2] + [extra] + records[2:])
    assert len(sector) <= SECTOR
    iso[lba * SECTOR : (lba + 1) * SECTOR] = sector.ljust(SECTOR, b"\0")
    return bytes(iso)


def system_cnf(iso: bytes):
    _, records = root_dir(bytearray(iso))
    for r in records:
        if r[33 : 33 + r[32]] == b"SYSTEM.CNF;1":
            return struct.unpack_from("<I", r, 2)[0], struct.unpack_from("<I", r, 10)[0]
    raise SystemExit("no SYSTEM.CNF;1 in the root")


def game_id(iso: bytes) -> bytes:
    lba, size = system_cnf(iso)
    cnf = iso[lba * SECTOR : lba * SECTOR + size]
    value = cnf.split(b"BOOT2", 1)[1].split(b"=", 1)[1].strip().split(b"\r")[0].split(b"\n")[0]
    return value.split(b":", 1)[1].lstrip(b"\\").split(b";")[0]


def zso(iso: bytes, align: int = 0) -> bytes:
    """ZSO with every block stored uncompressed (index bit 31 set)."""
    blocks = len(iso) // SECTOR
    data = 24 + 4 * (blocks + 1)
    index = [((data + k * SECTOR) >> align) | 0x80000000 for k in range(blocks)] + [(data + blocks * SECTOR) >> align]
    header = b"ZISO" + struct.pack("<IQIBB2x", 24, len(iso), SECTOR, 1, align)
    return header + struct.pack("<%dI" % len(index), *index) + iso


def ul_record(name: bytes, startup: bytes) -> bytes:
    r = bytearray(64)
    r[0 : len(name)] = name
    r[0x20:0x23] = b"ul."
    r[0x23 : 0x23 + len(startup)] = startup
    r[0x2F] = 1  # parts
    r[0x30] = 0x14  # DVD
    r[0x35] = 0x08
    return bytes(r)


def write(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(data)


def main():
    base = open(sys.argv[1], "rb").read()
    out = sys.argv[2]
    gid = game_id(base)
    cnf_lba, cnf_size = system_cnf(base)
    dvd = os.path.join(out, "stick", "DVD")

    write(os.path.join(dvd, "T0 control.iso"), base)
    write(os.path.join(dvd, "T1 zero-length name.iso"), with_record_after_dotdot(base, record(b"", cnf_lba, cnf_size)))
    write(os.path.join(dvd, "T2 ext-attr byte.iso"), with_record_after_dotdot(base, record(b"XA.TXT;1", cnf_lba, cnf_size, xa=0x80)))
    write(os.path.join(dvd, "T3 long file name.iso"), with_record_after_dotdot(base, record(b"A_VERY_LONG_LEVEL2_FILENAME.TXT;1", cnf_lba, cnf_size)))
    no_eq = bytearray(base)
    start = cnf_lba * SECTOR
    cnf = bytes(no_eq[start : start + cnf_size])
    i = cnf.index(b"BOOT2") + 5
    j = cnf.index(b"=", i)
    no_eq[start + j] = ord(" ")
    write(os.path.join(dvd, "T4 BOOT2 without equals.iso"), bytes(no_eq))
    control_zso = zso(base, 0)
    write(os.path.join(dvd, "T5 control.zso"), control_zso)
    write(os.path.join(dvd, "T6 align 12.zso"), control_zso[:21] + bytes([12]) + control_zso[22:])

    # One record plus one stray byte; the chunk is the control image.
    name = b"T7 ragged ul.cfg"
    write(os.path.join(out, "ul-ragged", "ul.cfg"), ul_record(name, gid) + b"\0")
    write(os.path.join(out, "ul-ragged", "ul.%08X.%s.00" % (usba_crc32(name), gid.decode())), base)

    for name in (b"T8 ul ASCII title", "T9 Тест кириллицы".encode("utf-8")):
        assert len(name) <= 32
        write(os.path.join(out, "ul-cyrillic", "ul.%08X.%s.00" % (usba_crc32(name), gid.decode())), base)
    write(
        os.path.join(out, "ul-cyrillic", "ul.cfg"),
        ul_record(b"T8 ul ASCII title", gid) + ul_record("T9 Тест кириллицы".encode("utf-8"), gid),
    )

    cht = gid.decode() + ".cht"
    write(
        os.path.join(out, "cht-indented", cht),
        b'"Indented comment test"\r\nMastercode\r\n00000000 00000001\r\n\r\n'
        b" //An indented comment\r\n00000000 00000002\r\n\r\nCheat B\r\n00000000 00000003\r\n",
    )
    write(os.path.join(out, "cht-251", cht), b"".join(b"Cheat %d\r\n00000000 %08X\r\n" % (n, n + 1) for n in range(251)))
    write(os.path.join(out, "cht-long-last-line", cht), b"Cheat A\r\n00000000 00000001\r\nCheat B\r\n00000000 00000002\r\n" + b"X" * 400)
    cht_dup = (
        b"Mastercode\r\n00000000 00000001\r\n"
        b"Character A\r\nInfinite Health\r\n00000000 00000002\r\n"
        b"Character B\r\nInfinite Health\r\n00000000 00000003\r\n"
    )
    write(os.path.join(out, "cht-duplicate-names", cht), cht_dup)

    # The game's own CFG, ending with a comment line without LF
    write(os.path.join(out, "cfg-comment-end", gid.decode() + ".cfg"), b"$Compatibility=0\r\n#end")
    # A VMC of a size OPL's menu does not offer (contents do not matter for the menu)
    write(os.path.join(out, "vmc-4mb", "keep4mb.bin"), bytes(4 * 1024 * 1024))
    print("game ID", gid.decode(), "- files written to", out)


main()
