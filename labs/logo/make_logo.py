# Draws the YAOPL logo in the style of OPL's logo (by Berion): square outlined capitals with a
# cyan-to-blue gradient and a white edge, and a small subtitle.
# One geometry, three outputs: gfx/logo.png (512x128 RGBA, the greeting screen, embedded in the ELF),
# docs/logo.svg (README) and labs/logo/avatar.png (the GitHub organization's avatar).
# Run from anywhere: python labs/logo/make_logo.py
import os
import struct
import zlib

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
W, H = 512, 128

# Big letters: cell width, height, stroke, gap (OPL's logo: 74, 58, 6, 14)
BW, BH, S, BG = 62, 58, 6, 12
BY = 25
MID = 26  # top of the middle bar, as in OPL's P


def big(ch, x0):
    """Rectangles (x, y, w, h) of one big letter."""
    w, h = BW, BH
    top = (0, 0, w, S)
    bot = (0, h - S, w, S)
    mid = (0, MID, w, S)
    left = (0, 0, S, h)
    right = (w - S, 0, S, h)
    g = {
        'Y': [(0, 0, S, MID), (w - S, 0, S, MID), mid, ((w - S) // 2, MID, S, h - MID)],
        'A': [top, left, right, mid],
        'O': [top, bot, left, right],
        'P': [top, left, (w - S, 0, S, MID), mid],
        'L': [left, bot],
    }[ch]
    return [(x0 + x, BY + y, rw, rh) for x, y, rw, rh in g]


# Subtitle: glyphs of SW x 8 pixels, 1-pixel strokes, bars on rows 0, 4 and 7
SW, SG, SSP = 11, 3, 10
SY = 95


def small(ch, x0):
    r = SW - 1
    c = r // 2

    def hb(y, a=0, b=r):
        return (a, y, b - a + 1, 1)

    def vb(x, a, b):
        return (x, a, 1, b - a + 1)
    g = {
        'Y': [vb(0, 0, 3), vb(r, 0, 3), hb(4), vb(c, 5, 7)],
        'E': [hb(0), hb(4), vb(0, 5, 6), hb(7)],
        'T': [hb(0), vb(c, 1, 7)],
        'A': [hb(0), vb(r, 1, 7), hb(4), vb(0, 5, 7)],
        'N': [hb(0), vb(r, 1, 7), vb(0, 2, 7)],
        'O': [hb(0), vb(r, 1, 6), vb(0, 2, 6), hb(7)],
        'H': [vb(0, 0, 7), vb(r, 0, 7), hb(4, 1, r - 1)],
        'R': [hb(0), vb(r, 1, 3), hb(4, 0, r - 1), vb(r, 5, 7), vb(0, 5, 7)],
        'P': [hb(0), vb(r, 1, 3), hb(4), vb(0, 5, 7)],
        'S': [hb(0), vb(0, 2, 3), hb(4), vb(r, 5, 6), hb(7)],
        '2': [hb(0), vb(r, 1, 3), hb(4), vb(0, 5, 6), hb(7)],
        'L': [vb(0, 0, 7), hb(7)],
        'D': [hb(0), vb(r, 1, 6), vb(1, 1, 6), hb(7)],
    }[ch]
    return [(x0 + x, SY + y, w, h) for x, y, w, h in g]


TITLE, SUB = 'YAOPL', 'YET ANOTHER OPEN PS2 LOADER'
tw = len(TITLE) * BW + (len(TITLE) - 1) * BG
words = SUB.split(' ')
sw = sum(len(wd) * SW + (len(wd) - 1) * SG for wd in words) + (len(words) - 1) * SSP
bx = (W - tw) // 2
sx = (W - sw) // 2
letter_x = [bx + i * (BW + BG) for i in range(len(TITLE))]
big_rects = [r for lx, ch in zip(letter_x, TITLE) for r in big(ch, lx)]
small_rects = []
x = sx
for wd in words:
    for ch in wd:
        small_rects += small(ch, x)
        x += SW + SG
    x += SSP - SG

# Gradients sampled from OPL's logo: title rows 25..82, subtitle rows 95..102
TG = [(0.0, (30, 240, 251)), (0.09, (0, 226, 248)), (1.0, (0, 58, 208))]
SGR = [(0.0, (59, 169, 255)), (1.0, (0, 89, 215))]
# The white edge along the top and the left of each big letter: fully white up to the first
# stop, gone at the second (fractions of the letter's width or height).
HTOP, HLEFT = (0.45, 0.95), (0.35, 0.95)


def grad(stops, t):
    for (t0, c0), (t1, c1) in zip(stops, stops[1:]):
        if t <= t1:
            k = (t - t0) / (t1 - t0)
            return tuple(round(a + (b - a) * k) for a, b in zip(c0, c1))
    return stops[-1][1]


def fadeout(f, stop):
    return 1.0 if f <= stop[0] else max(0.0, 1 - (f - stop[0]) / (stop[1] - stop[0]))


def whiten(p, k):
    return tuple(round(c + (255 - c) * k) for c in p[:3]) + (255,)


px = [[(0, 0, 0, 0)] * W for _ in range(H)]
for rects, stops, y0, y1 in ((big_rects, TG, BY, BY + BH - 1), (small_rects, SGR, SY, SY + 7)):
    for x, y, w, h in rects:
        for yy in range(y, y + h):
            base = grad(stops, (yy - y0) / (y1 - y0)) + (255,)
            px[yy] = list(px[yy])
            for xx in range(x, x + w):
                px[yy][xx] = base
for lx in letter_x:
    for xx in range(lx, lx + BW):
        if px[BY][xx][3]:
            px[BY][xx] = whiten(px[BY][xx], fadeout((xx - lx) / BW, HTOP))
    for yy in range(BY + 1, BY + BH):
        if px[yy][lx][3]:
            px[yy][lx] = whiten(px[yy][lx], fadeout((yy - BY) / BH, HLEFT))


def write_png(path, w, h, rows):
    raw = b''.join(b'\0' + bytes(c for p in row for c in p) for row in rows)

    def chunk(t, c):
        return struct.pack('>I', len(c)) + t + c + struct.pack('>I', zlib.crc32(t + c) & 0xffffffff)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0)) +
                chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


write_png(os.path.join(ROOT, 'gfx', 'logo.png'), W, H, px)


def avatar(text, size, k, name, bg=(28, 28, 28, 255)):
    """A square avatar (GitHub organization): the big letters k times larger, centred on a dark square,
    with the gradient and the white edge computed per output pixel. Not used by the build."""
    xs = [i * (BW + BG) for i in range(len(text))]
    ox = (size - (xs[-1] + BW) * k) // 2
    oy = (size - BH * k) // 2
    img = [[bg] * size for _ in range(size)]
    for lx, ch in zip(xs, text):
        x0 = ox + lx * k
        for x, y, w, h in big(ch, lx):
            for yy in range(oy + (y - BY) * k, oy + (y - BY + h) * k):
                base = grad(TG, (yy - oy) / (BH * k - 1)) + (255,)
                for xx in range(ox + x * k, ox + (x + w) * k):
                    img[yy][xx] = base
        for yy in range(oy, oy + BH * k):
            for xx in range(x0, x0 + BW * k):
                if img[yy][xx] == bg:
                    continue
                if yy < oy + k:
                    img[yy][xx] = whiten(img[yy][xx], fadeout((xx - x0) / (BW * k), HTOP))
                elif xx < x0 + k:
                    img[yy][xx] = whiten(img[yy][xx], fadeout((yy - oy) / (BH * k), HLEFT))
    write_png(os.path.join(ROOT, 'labs', 'logo', name), size, size, img)


avatar('Y', 840, 10, 'avatar.png')


def svg_stops(stops):
    return ''.join('<stop offset="%g" stop-color="#%02x%02x%02x"/>' % ((t,) + c) for t, c in stops)


def svg_fade(name, x1, y1, x2, y2, stop):
    return ('<linearGradient id="%s" gradientUnits="userSpaceOnUse" x1="%d" y1="%d" x2="%d" y2="%d">'
            '<stop offset="%g" stop-color="#fff"/><stop offset="%g" stop-color="#fff" stop-opacity="0"/>'
            '</linearGradient>') % (name, x1, y1, x2, y2, stop[0], stop[1])


def svg_rects(rects):
    return ''.join('<rect x="%d" y="%d" width="%d" height="%d"/>' % r for r in rects)


# The SVG is cropped to the drawing with a small margin and drawn at twice the size.
m = 8
vx, vy = min(bx, sx) - m, BY - m
vw, vh = max(tw, sw) + 2 * m, SY + 8 - BY + 2 * m
defs = ('<linearGradient id="t" gradientUnits="userSpaceOnUse" x1="0" y1="%d" x2="0" y2="%d">%s</linearGradient>'
        '<linearGradient id="s" gradientUnits="userSpaceOnUse" x1="0" y1="%d" x2="0" y2="%d">%s</linearGradient>') % (
    BY, BY + BH, svg_stops(TG), SY, SY + 8, svg_stops(SGR))
edges = ''
for i, lx in enumerate(letter_x):
    defs += svg_fade('h%d' % i, lx, 0, lx + BW, 0, HTOP) + svg_fade('v%d' % i, 0, BY, 0, BY + BH, HLEFT)
    tops = [(x, BY, w, 1) for x, y, w, h in big_rects if y == BY and lx <= x < lx + BW]
    lefts = [(lx, max(y, BY + 1), 1, y + h - max(y, BY + 1)) for x, y, w, h in big_rects if x == lx]
    edges += '<g fill="url(#h%d)">%s</g><g fill="url(#v%d)">%s</g>' % (i, svg_rects(tops), i, svg_rects(lefts))
svg = ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="%d %d %d %d" width="%d" height="%d" '
       'shape-rendering="crispEdges"><title>YAOPL - Yet Another Open PS2 Loader</title><defs>%s</defs>'
       '<g fill="url(#t)">%s</g>%s<g fill="url(#s)">%s</g></svg>\n') % (
    vx, vy, vw, vh, vw * 2, vh * 2, defs, svg_rects(big_rects), edges, svg_rects(small_rects))
with open(os.path.join(ROOT, 'docs', 'logo.svg'), 'w', newline='\n') as f:
    f.write(svg)
