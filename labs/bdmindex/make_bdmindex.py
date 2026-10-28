# Draws the BdmIndex pictures for two, three and four mass devices: gfx/Index<n>_<p>.png, dot p of n.
# The dots are cut from gfx/Index_0.png (upstream's picture for the first of five devices): the same
# 128x16 canvas, palette and 20-pixel pitch, with the n dots centred. The script checks that it rebuilds
# upstream's five-dot pictures Index_0..Index_4 exactly before it writes anything.
# Run from anywhere: python labs/bdmindex/make_bdmindex.py
import os
import struct
import zlib

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
GFX = os.path.join(ROOT, 'gfx')

PITCH = 20  # width of one dot's cell
BIG = 14    # first column of the big (selected) dot's cell in Index_0
SMALL = 34  # first column of a small dot's cell in Index_0


def chunks(data):
    i = 8
    while i < len(data):
        length, = struct.unpack('>I', data[i:i + 4])
        yield data[i + 4:i + 8], data[i + 8:i + 8 + length]
        i += 12 + length


def read_png(path):
    """Width, height, rows of palette indexes, PLTE and tRNS of an 8-bit palette PNG."""
    data = open(path, 'rb').read()
    idat = b''
    plte = trns = b''
    for kind, body in chunks(data):
        if kind == b'IHDR':
            w, h, depth, ctype, _, _, interlace = struct.unpack('>IIBBBBB', body)
            assert (depth, ctype, interlace) == (8, 3, 0), path
        elif kind == b'PLTE':
            plte = body
        elif kind == b'tRNS':
            trns = body
        elif kind == b'IDAT':
            idat += body
    raw = zlib.decompress(idat)
    rows, prev = [], bytearray(w)
    for y in range(h):
        f = raw[y * (w + 1)]
        line = bytearray(raw[y * (w + 1) + 1:(y + 1) * (w + 1)])
        for x in range(w):
            a = line[x - 1] if x else 0
            b = prev[x]
            c = prev[x - 1] if x else 0
            if f == 1:
                line[x] = (line[x] + a) & 0xFF
            elif f == 2:
                line[x] = (line[x] + b) & 0xFF
            elif f == 3:
                line[x] = (line[x] + (a + b) // 2) & 0xFF
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 0xFF
        rows.append(line)
        prev = line
    return w, h, rows, plte, trns


def rgba(rows, plte, trns):
    """The picture as RGBA tuples, to compare pictures with different palettes."""
    def colour(i):
        return tuple(plte[3 * i:3 * i + 3]) + ((trns[i] if i < len(trns) else 255),)

    return [[colour(i) for i in row] for row in rows]


def write_png(path, w, h, rows, plte, trns):
    def chunk(kind, body):
        return struct.pack('>I', len(body)) + kind + body + struct.pack('>I', zlib.crc32(kind + body))

    raw = b''.join(b'\0' + bytes(row) for row in rows)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n')
        f.write(chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 3, 0, 0, 0)))
        f.write(chunk(b'PLTE', plte))
        f.write(chunk(b'tRNS', trns))
        f.write(chunk(b'IDAT', zlib.compress(raw, 9)))
        f.write(chunk(b'IEND', b''))


def dots(src, w, h, count, selected):
    """Rows of a picture with `count` dots centred, dot `selected` big."""
    clear = src[0][0]
    rows = [bytearray([clear]) * w for _ in range(h)]
    start = w // 2 - count * PITCH // 2
    for k in range(count):
        cell = BIG if k == selected else SMALL
        for y in range(h):
            rows[y][start + k * PITCH:start + (k + 1) * PITCH] = src[y][cell:cell + PITCH]
    return rows


def main():
    w, h, src, plte, trns = read_png(os.path.join(GFX, 'Index_0.png'))

    for p in range(5):
        _, _, rows, p_plte, p_trns = read_png(os.path.join(GFX, 'Index_%d.png' % p))
        assert rgba(dots(src, w, h, 5, p), plte, trns) == rgba(rows, p_plte, p_trns), 'Index_%d' % p

    for count in range(2, 5):
        for p in range(count):
            name = 'Index%d_%d.png' % (count, p)
            write_png(os.path.join(GFX, name), w, h, dots(src, w, h, count, p), plte, trns)
            print(name)


main()
