"""Write the malformed inputs used by run_all.ps1 into the given directory."""
import os
import sys


def cheats(out):
    files = {
        "cht_plain.cht": b"Mastercode\r\n90106418 0c0418ae\r\n\r\nCheat\r\n20100000 00000001\r\n",
        # Same layout as SLES_505.41.cht in PS2-Widescreen/OPL-Widescreen-Cheats
        "cht_indented.cht": b'"Game /ID SLES_000.00"\r\nMastercode\r\n90106418 0c0418ae\r\n\r\n'
        b" //Gameplay 16:9\r\n20142e50 3c023faa\r\n201430c4 3c023faa\r\n",
        "cht_251.cht": b"".join(b"Cheat %d\r\n2%07X 00000001\r\n" % (n, 0x100000 + n * 4) for n in range(251)),
        "cht_long_last.cht": b"Mastercode\r\n90106418 0c0418ae\r\n" + b"A" * 400,
        "cht_no_lf.cht": b"Mastercode\r\n90106418 0c0418ae\r\nCheat\r\n20100000 00000001",
    }
    for name, data in files.items():
        with open(os.path.join(out, name), "wb") as f:
            f.write(data)


def ul_record(name, startup):
    r = bytearray(64)
    r[0 : len(name)] = name
    r[0x20:0x23] = b"ul."
    r[0x23 : 0x23 + len(startup)] = startup
    r[0x2F] = 2
    r[0x30] = 0x14
    r[0x35] = 0x08
    return bytes(r)


def ul_cfg(out):
    a = ul_record(b"Game A", b"SLUS_200.62")
    b = ul_record(b"Game B", b"SLES_505.41")
    for d, data in (("ul64", a), ("ul65", a + b"\x00"), ("ul129", a + b + b"\x0a")):
        os.makedirs(os.path.join(out, d), exist_ok=True)
        with open(os.path.join(out, d, "ul.cfg"), "wb") as f:
            f.write(data)
    os.makedirs(os.path.join(out, "cachedir"), exist_ok=True)


def system_cnf(out):
    for name, boot2 in (("cnf_ok.cnf", b"BOOT2 = "), ("cnf_no_eq.cnf", b"BOOT2 ")):
        with open(os.path.join(out, name), "wb") as f:
            f.write(boot2 + b"cdrom0:\\SLUS_200.62;1\r\nVER = 1.00\r\nVMODE = NTSC\r\n")


def config_files(out):
    files = {
        "cfg_comment_end.cfg": b"a=1\r\n#end",  # last line is a comment without LF
        "cfg_plain.cfg": b"a=1\r\n#comment\r\nb=2\r\n",
        "IPCONFIG.DAT": b"192.168.0.10 255.255.255.0 192.168.0.1".ljust(300, b" "),
    }
    for name, data in files.items():
        with open(os.path.join(out, name), "wb") as f:
            f.write(data)


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    cheats(out)
    ul_cfg(out)
    system_cnf(out)
    config_files(out)
    with open(os.path.join(out, "zero.bin"), "wb") as f:
        f.write(bytes(65536))
    print("inputs written to", out)


main()
