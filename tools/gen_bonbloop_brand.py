#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""BONBLOOP's look outside the FM-1, in the FM-1's own font (Terminus Bold 8x16, assets/fonts) and colours.

  tools/gen_bonbloop_brand.py logo              assets/logo/bonbloop-logo.svg (1.0: the penguin and BON / BLOOP),
                                                assets/logo/bonbloop-wordmark.svg and web/icon-*.png (the penguin)
  tools/gen_bonbloop_brand.py promo SCREENS OUT  the promo images (1600x900 and 1080x1080 PNG) into OUT, from the
                                                device screens SCREENS/synth-*.ppm (tests/synth_mode_test.c writes them)

The promo images and the site are in bonbap.com.br's look (Inter Tight, #0a0a0a, the giant word, thin lines): the
images are HTML pages shot by Edge headless (Windows: EDGE; run it from PowerShell or cmd: from Git Bash Edge
exits at once and draws nothing). Needs Pillow. The screens are rendered by the firmware's
own code; no third-party logo or photo."""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BDF = ROOT / "assets" / "fonts" / "ter-u16b.bdf"
WHITE, BLUE, GREEN, YELLOW, ORANGE = "#ffffff", "#287cff", "#1ecc70", "#ffc618", "#ff621a"
TRACK = [BLUE, GREEN, YELLOW, ORANGE]          # the four track colours (gfx.c TE_COL_DEF)
GRAY, DIM, BG, TILE, LINE = "#c4c4cc", "#8a8a92", "#000000", "#16161a", "#2a2a30"
URL = "bonbap.com.br/bonbloop"


def load_bdf(path=BDF):
    """glyphs {char: [row bits]}, 8x16 cells (Terminus 16 bold: every glyph in the 8x16 box)"""
    glyphs, cur, rows, inbm = {}, None, [], False
    for ln in path.read_text(encoding="latin-1").splitlines():
        if ln.startswith("ENCODING "):
            cur = int(ln.split()[1])
        elif ln == "BITMAP":
            inbm, rows = True, []
        elif ln == "ENDCHAR":
            if cur is not None and cur < 0x2500:
                glyphs[chr(cur)] = rows
            inbm, cur = False, None
        elif inbm:
            rows.append(int(ln, 16))
    return glyphs


GLYPHS = None


def glyph(ch):
    global GLYPHS
    if GLYPHS is None:
        GLYPHS = load_bdf()
    return GLYPHS.get(ch) or GLYPHS.get("?")


def text_pixels(text):
    """[(x, y)] of the lit pixels of text in 8x16 cells"""
    out = []
    for i, ch in enumerate(text):
        for y, bits in enumerate(glyph(ch)):
            for x in range(8):
                if bits & (0x80 >> x):
                    out.append((i * 8 + x, y))
    return out


def pixelify_word(text):
    """(BONBLOOP 1.9.2) [(x, y)] of the lit pixels of text in the FM-1's own screen font (Pixelify Sans on its 11 px
    grid, tools/gen_font.py), its capitals' rows only, and its width"""
    import sys
    sys.path.insert(0, str(ROOT / "tools"))
    from gen_font import render_pixelify, FONTS, PAD
    h, glyphs = render_pixelify(FONTS / "PixelifySans-Variable.ttf", 11, 1, 32, 127, "S")
    out, pen = [], 0
    for ch in text:
        adv, bw, px = glyphs[ord(ch) - 32]
        for y in range(5, 12):                         # (the capitals: rows 5..11 of the 16-pixel cell)
            for x in range(bw):
                if px[y * bw + x]:
                    out.append((pen + x - PAD, y - 5))
        pen += adv
    return out, pen - 1


PENGUIN = ROOT / "assets" / "logo" / "bonbloop-penguin.png"   # (BONBLOOP 1.0) the mark: a Magellanic penguin's head,
                                                            # pixel art, 1 px a pixel, transparent around it


def penguin_rects(ox=0, oy=0):
    """the penguin as SVG rects (a row's runs of one colour as one rect), its width and height"""
    from PIL import Image
    im = Image.open(PENGUIN).convert("RGBA")
    px = im.load()
    out = []
    for y in range(im.height):
        x = 0
        while x < im.width:
            r, g, b, a = px[x, y]
            if a < 128:
                x += 1
                continue
            x0 = x
            while x < im.width and px[x, y] == (r, g, b, a):
                x += 1
            out.append(f'<rect x="{ox + x0}" y="{oy + y}" width="{x - x0}" height="1" fill="#{r:02X}{g:02X}{b:02X}"/>')
    return out, im.width, im.height


def wordmark_rects(ox=0, oy=0, k=1):
    """BON over BLOOP, as the boot screen draws it (splash.c, BONBLOOP 1.7): the FM-1's font, BON in light lilac
    #D9DAF4, BLOOP in Very Peri #6667AB, k screen pixels a pixel; its rects, width and height"""
    bon, _ = pixelify_word("BON")
    bloop, _ = pixelify_word("BLOOP")
    wb, wl = max(x for x, _ in bon) + 1, max(x for x, _ in bloop) + 1   # (1.0: the lit pixels' width: pixelify_word's
    gap = 3                                                              # is one less, and BLOOP's P lost its right side)
    out = []
    for pts, x0, y0, col in ((bon, (wl - wb) // 2, 0, "#D9DAF4"), (bloop, 0, 7 + gap, "#6667AB")):
        for x, y in sorted(pts, key=lambda q: (q[1], q[0])):
            out.append(f'<rect x="{ox + (x0 + x) * k}" y="{oy + (y0 + y) * k}" width="{k}" height="{k}" fill="{col}"/>')
    return out, wl * k, (7 * 2 + gap) * k


def svg(w, h, rects):
    """the first rect is the background (the installer's CSS makes it transparent)"""
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" shape-rendering="crispEdges" '
            f'role="img" aria-label="BONBLOOP"><rect width="{w}" height="{h}" fill="#000"/>' + "".join(rects) + "</svg>\n")


LOGO_W, LOGO_H = 111, 36                                   # (the logo's size in its pixels: logo_svg)


def logo_png(path):
    """the logo as a PNG, 1 px a pixel, transparent around it (the promo pages scale it, pixelated)"""
    import re
    from PIL import Image, ImageDraw
    s = logo_svg()
    w, h = (int(v) for v in re.search(r'viewBox="0 0 (\d+) (\d+)"', s).groups())
    assert (w, h) == (LOGO_W, LOGO_H), (w, h)
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for i, m in enumerate(re.finditer(r'<rect ([^/]*)/>', s)):
        if i == 0:
            continue                                     # (the ground)
        a = dict(re.findall(r'(\w+)="([^"]+)"', m.group(1)))
        x, y = int(a.get("x", 0)), int(a.get("y", 0))
        d.rectangle([x, y, x + int(a["width"]) - 1, y + int(a["height"]) - 1], fill=a["fill"])
    im.save(path)
    return path


def editor_logo():
    """the editor's logo (web/editor.html, between <!--LOGO--> and <!--/LOGO-->): the logo SVG inline"""
    import re
    p = ROOT / "web" / "editor.html"
    s = p.read_text(encoding="utf-8")
    new, k = re.subn(r"<!--LOGO-->.*?<!--/LOGO-->", lambda m: "<!--LOGO-->" + logo_svg().strip() + "<!--/LOGO-->", s,
                     flags=re.S)
    if k != 1:
        sys.exit("web/editor.html: no <!--LOGO-->...<!--/LOGO-->")
    p.write_text(new, encoding="utf-8")
    return p


def logo_svg():
    """(BONBLOOP 1.0) the logo: the penguin's head, then BON / BLOOP at twice its pixel (as thick as the penguin's
    outline), in the middle of its height"""
    pen, pw, ph = penguin_rects()
    _, ww, wh = wordmark_rects(k=2)
    gap = 6
    words, _, _ = wordmark_rects(pw + gap, (ph - wh) // 2, 2)
    return svg(pw + gap + ww, ph, pen + words)


def wordmark_svg():
    """BON / BLOOP alone (1 px a pixel)"""
    r, w, h = wordmark_rects()
    return svg(w, h, r)


# ------------------------------------------------------------------------------------------------ promo ---
EDGE = Path(r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe")

PROMO_CSS = """
:root { --bg: #0a0a0a; --fg: #fafafa; --solid: #cfcfcf; --muted: #8a8a8a; --dim: #5a5a5a; --tile: #141416;
        --line: rgba(255,255,255,.09); --blue: #6667AB; --green: #3BA488; --yellow: #8B5CF6; --orange: #D9DAF4; }
/* (BONBLOOP 1.9.2) its own colours, in the old names: Very Peri, the knobs' green, the violet of what is on, lilac */
* { box-sizing: border-box; margin: 0; }
html, body { width: WPX; height: HPX; overflow: hidden; background: var(--bg); color: var(--fg);
             font: 500 16px/1.25 "Inter Tight", sans-serif; -webkit-font-smoothing: antialiased; }
body { position: relative; }
.grain { position: absolute; inset: 0; opacity: .035; background-image: url("data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' width='160' height='160'><filter id='n'><feTurbulence type='fractalNoise' baseFrequency='.8' numOctaves='3'/><feColorMatrix type='saturate' values='0'/></filter><rect width='160' height='160' filter='url(%23n)'/></svg>"); }
.strip { position: absolute; left: 0; right: 0; top: 0; height: 8px; display: flex; }
.strip i { flex: 1; } .strip i:nth-child(1) { background: var(--blue); } .strip i:nth-child(2) { background: var(--green); }
.strip i:nth-child(3) { background: var(--yellow); } .strip i:nth-child(4) { background: var(--orange); }
.abs { position: absolute; }
.word { font-weight: 800; color: var(--solid); letter-spacing: -.035em; line-height: .8; white-space: nowrap; }
.big { font-weight: 800; letter-spacing: -.02em; text-transform: uppercase; line-height: 1; }
.sub { font-weight: 700; letter-spacing: .02em; text-transform: uppercase; color: var(--solid); }
.line { font-weight: 700; letter-spacing: .02em; text-transform: uppercase; }
.small { font-weight: 700; letter-spacing: .12em; text-transform: uppercase; color: var(--muted); }
.screen { position: absolute; image-rendering: pixelated; padding: 10px; background: #0d0d10; }
.screen img { display: block; width: 100%; height: 100%; image-rendering: pixelated; }
.card { position: absolute; background: var(--tile); }
"""


def promo(screens, out):
    """the promo images: the layout they had (the FM-1's screens, the cards and steps) in bonbap.com.br's type (Inter
    Tight, the lower-case word, #0a0a0a); BONBLOOP 1.9.2: its own colours and what it is now (a groovebox firmware
    based on SLOOP, not SLOOP plus a mode). HTML pages shot by Edge headless"""
    import subprocess
    import tempfile
    from PIL import Image

    out.mkdir(parents=True, exist_ok=True)
    html_dir = Path(tempfile.mkdtemp(prefix="bonbloop-promo-"))
    for name in ("synth-osc1-playing", "synth-osc3-vocal", "page-comp", "drums-clip", "page-scale-lights", "synth-boot",
                 "page-tracks"):
        Image.open(screens / f"{name}.ppm").convert("RGB").resize((480, 480), Image.NEAREST).save(html_dir / f"{name}.png")
    logo_png(html_dir / "logo.png")                     # (BONBLOOP 1.0) the logo, 1 px a pixel: scaled in the pages

    def logo(x, y, k, centre=False):
        w = LOGO_W * k
        left = (1080 - w) // 2 if centre else x
        return (f'<img class="abs" src="logo.png" style="left:{left}px;top:{y}px;width:{w}px;height:{LOGO_H * k}px;'
                f'image-rendering:pixelated">')

    def page(w, h, body):
        return (f'<!doctype html><html><head><meta charset="utf-8">'
                '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Inter+Tight:wght@500;700;800&display=block">'
                f'<style>{PROMO_CSS.replace("WPX", f"{w}px").replace("HPX", f"{h}px")}</style></head><body>'
                f'<div class="grain"></div><div class="strip"><i></i><i></i><i></i><i></i></div>{body}</body></html>')

    def scr(name, x, y, size, colour):
        return (f'<div class="screen" style="left:{x - 10}px;top:{y - 10}px;width:{size + 20}px;height:{size + 20}px;'
                f'border:3px solid var(--{colour})"><img src="{name}.png"></div>')

    def at(x, y, cls, text, style=""):
        return f'<div class="abs {cls}" style="left:{x}px;top:{y}px;{style}">{text}</div>'

    hero = (logo(70, 44, 5)
            + at(70, 262, "big", "Groovebox", "font-size:84px")
            + at(70, 366, "sub", "firmware for the M-VAVE FM-1", "font-size:36px")
            + at(70, 470, "line", "synth mode · 3 oscillators", "font-size:30px;color:var(--blue)")
            + at(70, 518, "line", "parallel fx · eq + clipper per track", "font-size:30px;color:var(--green)")
            + at(70, 566, "line", "the scale on the keys", "font-size:30px;color:var(--yellow)")
            + at(70, 614, "line", "notes end where you let go", "font-size:30px;color:var(--orange)")
            + at(70, 728, "small", "based on sloop · free · open source · installs from the browser", "font-size:20px")
            + at(70, 780, "line", URL, "font-size:40px;text-transform:none;letter-spacing:-.01em")
            + scr("synth-osc1-playing", 1010, 90, 480, "blue")
            + scr("drums-clip", 1010, 610, 240, "green") + scr("page-scale-lights", 1290, 610, 240, "yellow"))
    cards = ""
    for i, (title, l1, l2, s, c, foot) in enumerate((
            ("synth", "3 osc · 16 waves", "env + lfo each", "synth-osc3-vocal", "blue", "hold env"),
            ("mix", "dst mod dly rev", "eq on every track", "page-comp", "green", "+ glue comp"),
            ("clipper", "soft · hard · fold", "on every track", "drums-clip", "yellow", "its curve on screen"),
            ("scale", "lit on the keys", "root or 1-3-5", "page-scale-lights", "orange", "glo > scale"))):
        x = 40 + i * 390
        cards += (f'<div class="card" style="left:{x}px;top:150px;width:360px;height:710px;border-top:8px solid var(--{c})"></div>'
                  + at(x + 22, 186, "big", title, f"font-size:52px;color:var(--{c})")
                  + at(x + 22, 262, "sub", l1, "font-size:26px") + at(x + 22, 300, "sub", l2, "font-size:26px")
                  + f'<img class="abs" src="{s}.png" style="left:{x + 20}px;top:380px;width:320px;height:320px;image-rendering:pixelated">'
                  + at(x + 22, 742, "small", foot, "font-size:20px"))
    features = at(0, 40, "big", "what bonbloop adds", "width:1600px;text-align:center;font-size:58px") + cards
    steps = ""
    y = 340
    for n, s, c in (("1", "plug the FM-1 into your pc (usb)", "blue"), ("2", "open in chrome or edge:", "green"), ("", URL, ""),
                    ("3", "click install", "yellow"), ("4", "play: hold env for the synth", "orange")):
        if n:
            steps += (f'<div class="abs" style="left:70px;top:{y - 6}px;width:60px;height:60px;background:var(--{c});'
                      f'color:#0a0a0a;font:800 40px/60px Inter Tight;text-align:center">{n}</div>'
                      + at(170, y + 4, "line", s, "font-size:38px"))
            y += 92
        else:
            steps += at(170, y - 16, "line", s, "font-size:38px;text-transform:none;color:var(--fg)")
            y += 70
    install = (logo(70, 40, 4)
               + at(70, 196, "big", "install in 1 minute", "font-size:60px")
               + at(70, 268, "sub", "from the browser, over usb", "font-size:26px;color:var(--muted)")
               + steps
               + at(70, 820, "small", "based on sloop + felucca · gpl-3.0 · not affiliated with m-vave", "font-size:20px")
               + scr("synth-boot", 1060, 300, 480, "blue"))
    square = (logo(0, 60, 6, centre=True)
              + at(0, 300, "big", "groovebox", "width:1080px;text-align:center;font-size:84px")
              + at(0, 400, "sub", "for the M-VAVE FM-1 · based on sloop", "width:1080px;text-align:center;font-size:36px")
              + scr("synth-osc1-playing", 300, 490, 480, "blue")
              + at(0, 1012, "line", URL, "width:1080px;text-align:center;font-size:32px;text-transform:none"))
    cards = {"bonbloop-hero-1600x900.png": (1600, 900, hero), "bonbloop-features-1600x900.png": (1600, 900, features),
             "bonbloop-install-1600x900.png": (1600, 900, install), "bonbloop-square-1080.png": (1080, 1080, square)}
    made = []
    for name, (w, h, body) in cards.items():
        src = html_dir / (name + ".html")
        src.write_text(page(w, h, body), encoding="utf-8")
        dst = (out / name).resolve()                   # (absolute: Edge writes a relative one beside itself)
        if dst.exists():
            dst.unlink()
        p = subprocess.Popen([str(EDGE), "--headless=new", "--disable-gpu", "--no-first-run", "--no-default-browser-check",
                              "--hide-scrollbars", "--force-device-scale-factor=1", f"--window-size={w},{h}",
                              "--virtual-time-budget=6000", f"--user-data-dir={html_dir / 'edge'}", f"--screenshot={dst}",
                              src.as_uri()], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)   # (no pipes: Edge's helpers
        try:                                                                                        # keep them open)
            p.wait(timeout=90)
        except subprocess.TimeoutExpired:
            p.kill()
        if not dst.exists():
            raise SystemExit(f"promo: Edge made no {dst} (run it from PowerShell or cmd, not Git Bash)")
        made.append(dst)
    return made


def icons():
    """(BONBLOOP 1.0) the site's app icons (web/icon-*.png, web/make_site.py): the penguin's head in the middle of the
    site's black (#09090c), whole pixels only (nearest); the maskable one inside the middle circle (80 %), the square full"""
    from PIL import Image, ImageDraw
    pen = Image.open(PENGUIN).convert("RGBA")
    made = []
    for name, size, k, rounded in (("icon-192.png", 192, 3, True), ("icon-512.png", 512, 8, True),
                                   ("icon-maskable-512.png", 512, 6, False)):
        img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        d = ImageDraw.Draw(img)
        if rounded:
            d.rounded_rectangle([0, 0, size - 1, size - 1], radius=size // 5, fill="#09090c")
        else:
            d.rectangle([0, 0, size - 1, size - 1], fill="#09090c")
        big = pen.resize((pen.width * k, pen.height * k), Image.NEAREST)
        img.alpha_composite(big, ((size - big.width) // 2, (size - big.height) // 2))
        p = ROOT / "web" / name
        img.save(p)
        made.append(p)
    return made


def penguin_h():
    """(BONBLOOP 1.0) the penguin for the FM-1's boot screen (splash.c): firmware/src/bonbloop_penguin.h, its colours
    as RGB565 (0: none, the screen's black) and its pixels row by row as runs, (run - 1) << 4 | colour (SLOOP's logo's
    format)"""
    from PIL import Image
    im = Image.open(PENGUIN).convert("RGBA")
    px = im.load()
    seq = [px[x, y] for y in range(im.height) for x in range(im.width)]
    cols = {}
    for c in seq:
        if c[3] >= 128:
            cols[c[:3]] = cols.get(c[:3], 0) + 1
    pal = [None] + sorted(cols, key=lambda c: -cols[c])
    assert len(pal) <= 16, len(pal)
    idx = [0 if c[3] < 128 else pal.index(c[:3]) for c in seq]
    rle, i = [], 0
    while i < len(idx):
        j = i
        while j < len(idx) and idx[j] == idx[i] and j - i < 16:
            j += 1
        rle.append((j - i - 1) << 4 | idx[i])
        i = j
    rgb565 = ["0x0000"] + [f"0x{((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3):04X}" for r, g, b in pal[1:]]
    body = ", ".join(f"0x{b:02X}" for b in rle)
    lines, line = [], "    "
    for tok in body.split(", "):
        if len(line) + len(tok) + 2 > 118:
            lines.append(line.rstrip())
            line = "    "
        line += tok + ", "
    lines.append(line.rstrip().rstrip(","))
    out = ROOT / "firmware" / "src" / "bonbloop_penguin.h"
    out.write_text(
        "/* SPDX-License-Identifier: GPL-3.0-only */\n"
        "/* BONBLOOP 1.0: the boot screen's penguin (splash.c), made by tools/gen_bonbloop_brand.py logo from\n"
        " * assets/logo/bonbloop-penguin.png: its colours (RGB565, 0 none) and its pixels as runs, (run - 1) << 4 | colour */\n"
        f"#define PENGUIN_W {im.width}u\n#define PENGUIN_H {im.height}u\n"
        f"static const uint16_t PENGUIN_PAL[16] = {{{', '.join(rgb565)}}};\n"
        f"static const uint8_t PENGUIN_RLE[{len(rle)}] = {{\n" + "\n".join(lines) + "\n};\n", encoding="utf-8")
    return out, len(rle)


def main():
    if len(sys.argv) >= 2 and sys.argv[1] == "logo":
        out = ROOT / "assets" / "logo" / "bonbloop-logo.svg"
        out.write_text(logo_svg(), encoding="utf-8")
        print(f"logo: {out} ({out.stat().st_size} B)")
        print(f"editor: {editor_logo()}")
        print("boot screen: %s (%d B of runs)" % penguin_h())
        wm = ROOT / "assets" / "logo" / "bonbloop-wordmark.svg"
        wm.write_text(wordmark_svg(), encoding="utf-8")
        print(f"wordmark: {wm}")
        for p in icons():
            print(f"icon: {p}")
    elif len(sys.argv) == 4 and sys.argv[1] == "promo":
        for p in promo(Path(sys.argv[2]), Path(sys.argv[3])):
            print(f"promo: {p}")
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
