#!/usr/bin/env python3
"""List the IOP modules (IRX files) embedded in an unpacked OPL ELF: size, SHA-256, module version
from .iopmod (the names are stripped) and the export table(s) if the module has any.
Usage: elfmods.py <ELF> [<ELF2>]  (two files: compare them module by module, by position, then compare
the bytes of the loadable segments that lie outside the modules)"""
import hashlib
import struct
import sys


def irx_modules(data):
    mods = []
    pos = 0
    while True:
        pos = data.find(b"\x7fELF", pos)
        if pos < 0:
            break
        hdr = data[pos:pos + 52]
        if len(hdr) < 52 or hdr[4] != 1 or hdr[5] != 1:
            pos += 4
            continue
        e_type = struct.unpack_from("<H", hdr, 16)[0]
        if e_type != 0xFF80:  # ET_SCE_IOPRELEXEC
            pos += 4
            continue
        (e_phoff, e_shoff, _flags, _ehsize, e_phentsize, e_phnum,
         e_shentsize, e_shnum, _shstrndx) = struct.unpack_from("<IIIHHHHHH", hdr, 28)
        end = 52
        if e_shoff and e_shnum:
            end = max(end, e_shoff + e_shnum * e_shentsize)
            for i in range(e_shnum):
                sh = data[pos + e_shoff + i * e_shentsize:pos + e_shoff + (i + 1) * e_shentsize]
                sh_type, _f, _a, sh_off, sh_size = struct.unpack_from("<IIIII", sh, 4)
                if sh_type != 8:  # SHT_NOBITS
                    end = max(end, sh_off + sh_size)
        ver = "?"
        for i in range(e_phnum):
            ph = data[pos + e_phoff + i * e_phentsize:pos + e_phoff + (i + 1) * e_phentsize]
            p_type, p_off, _v, _p, p_filesz = struct.unpack_from("<IIIII", ph, 0)
            end = max(end, p_off + p_filesz)
            if p_type == 0x70000080 and p_filesz >= 0x1A:  # .iopmod: ... u16 version at 0x18
                v = struct.unpack_from("<H", data, pos + p_off + 0x18)[0]
                ver = "%d.%d" % (v >> 8, v & 0xFF)
        blob = data[pos:pos + end]
        mods.append(dict(off=pos, size=end, sha=hashlib.sha256(blob).hexdigest(), ver=ver,
                         exp=export_versions(blob)))
        pos += end
    return mods


def export_versions(blob):
    # irx_export_table: u32 magic 0x41C00000, u32 next, u16 version, u16 mode, char name[8]
    found = []
    pos = 0
    while True:
        pos = blob.find(b"\x00\x00\xC0\x41", pos)
        if pos < 0:
            break
        if pos + 20 <= len(blob):
            nxt, ver, _mode = struct.unpack_from("<IHH", blob, pos + 4)
            raw = blob[pos + 12:pos + 20].split(b"\0", 1)[0]
            if nxt == 0 and raw and all(32 <= c < 127 for c in raw):
                found.append("%s %d.%d" % (raw.decode("ascii"), ver >> 8, ver & 0xFF))
        pos += 4
    return ", ".join(found)


def load_segments(data):
    """File ranges of the PT_LOAD segments of the (outer) ELF: what the console loads."""
    e_phoff = struct.unpack_from("<I", data, 28)[0]
    e_phentsize, e_phnum = struct.unpack_from("<HH", data, 42)
    ranges = []
    for i in range(e_phnum):
        p_type, p_off, _v, _p, p_filesz = struct.unpack_from("<IIIII", data, e_phoff + i * e_phentsize)
        if p_type == 1 and p_filesz:
            ranges.append((p_off, p_off + p_filesz))
    return ranges


def show(path):
    data = open(path, "rb").read()
    mods = irx_modules(data)
    segs = load_segments(data)
    print("%s: %d bytes, %d bytes in %d loadable segments, %d IRX modules" % (
        path, len(data), sum(b - a for a, b in segs), len(segs), len(mods)))
    for i, m in enumerate(mods):
        print("  #%02d v%-6s %7d bytes  exports: %-24s sha256 %s" % (i, m["ver"], m["size"], m["exp"] or "-", m["sha"][:16]))
    return data, mods, segs


def loaded_outside(data, mods, segs):
    """The bytes of the loadable segments that are not part of an embedded module."""
    out = bytearray()
    for a, b in segs:
        pos = a
        for m in mods:
            if m["off"] >= b or m["off"] + m["size"] <= a:
                continue
            out += data[pos:max(pos, m["off"])]
            pos = max(pos, m["off"] + m["size"])
        out += data[pos:b]
    return bytes(out)


if __name__ == "__main__":
    data, mods, segs = show(sys.argv[1])
    if len(sys.argv) > 2:
        data2, mods2, segs2 = show(sys.argv[2])
        print("")
        print("Comparison by position:")
        differ = 0
        for i in range(max(len(mods), len(mods2))):
            a = mods[i] if i < len(mods) else None
            b = mods2[i] if i < len(mods2) else None
            if a is None or b is None:
                print("  #%02d only in the %s file" % (i, "first" if a else "second"))
                differ += 1
            elif a["sha"] != b["sha"]:
                print("  #%02d DIFFERS: %d -> %d bytes, v%s -> v%s, exports %s -> %s" % (
                    i, a["size"], b["size"], a["ver"], b["ver"], a["exp"] or "-", b["exp"] or "-"))
                differ += 1
        print("  modules: %d vs %d, differing: %d" % (len(mods), len(mods2), differ))
        r1, r2 = loaded_outside(data, mods, segs), loaded_outside(data2, mods2, segs2)
        print("  loadable bytes outside the modules: %d vs %d, %s" % (len(r1), len(r2), "identical" if r1 == r2 else "differ"))
        if r1 != r2 and len(r1) == len(r2):
            diffs = [i for i in range(len(r1)) if r1[i] != r2[i]]
            # Group the differing bytes into runs (a gap of more than 16 bytes starts a new run).
            runs = []
            for i in diffs:
                if runs and i - runs[-1][1] <= 16:
                    runs[-1][1] = i
                else:
                    runs.append([i, i])
            print("  differing bytes: %d in %d runs" % (len(diffs), len(runs)))
            for lo, hi in runs[:6]:
                a, b = max(0, lo - 12), min(len(r1), hi + 13)
                print("   at %d (%d bytes): %r -> %r" % (lo, hi - lo + 1, r1[a:b], r2[a:b]))
            if len(runs) > 6:
                print("   ... %d more runs" % (len(runs) - 6))
