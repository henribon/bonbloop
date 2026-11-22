#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""BONBLOOP's look outside the FM-1, in the FM-1's own font (Terminus Bold 8x16, assets/fonts) and colours.

  tools/gen_bonbloop_brand.py logo              assets/logo/bonbloop-logo.svg and web/icon-*.png (the site's, web/make_site.py)
  tools/gen_bonbloop_brand.py promo SCREENS OUT  the promo images (1600x900 and 1080x1080 PNG) into OUT, from the
                                                device screens SCREENS/synth-*.ppm (tests/synth_mode_test.c writes them)

Needs Pillow for the promo images. Everything drawn here is BONBLOOP's own: the text in Terminus (SIL OFL 1.1),
the screens rendered by the firmware's code; no third-party logo or photo."""
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


def logo_svg():
    """BON over BLOOP, as the boot screen draws it (splash.c): BON white, BLOOP's B white and L O O P the tracks'
    colours. Pixel runs as rects; the first rect is the background (the installer's CSS makes it transparent)"""
    w, h, gap = 5 * 8, 2 * 16 + 2, 2
    rects = [f'<rect width="{w}" height="{h}" fill="#000"/>']
    lines = [("BON", (w - 24) // 2, 0, [WHITE] * 3), ("BLOOP", 0, 16 + gap, [WHITE] + TRACK)]
    for text, ox, oy, cols in lines:
        for i, ch in enumerate(text):
            rows = glyph(ch)
            for y, bits in enumerate(rows):
                x = 0
                while x < 8:
                    if bits & (0x80 >> x):
                        x0 = x
                        while x < 8 and bits & (0x80 >> x):
                            x += 1
                        rects.append(f'<rect x="{ox + i * 8 + x0}" y="{oy + y}" width="{x - x0}" height="1" fill="{cols[i]}"/>')
                    else:
                        x += 1
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" shape-rendering="crispEdges" '
            f'role="img" aria-label="BONBLOOP">' + "".join(rects) + "</svg>\n")


# ------------------------------------------------------------------------------------------------ promo ---
def promo(screens, out):
    from PIL import Image, ImageDraw

    out.mkdir(parents=True, exist_ok=True)

    def text(img, xy, s, scale, colour, cols=None):
        """s in Terminus at an integer scale, top-left xy; cols: a colour per character"""
        d = ImageDraw.Draw(img)
        for i, ch in enumerate(s):
            c = cols[i] if cols else colour
            for y, bits in enumerate(glyph(ch)):
                for x in range(8):
                    if bits & (0x80 >> x):
                        px, py = xy[0] + (i * 8 + x) * scale, xy[1] + y * scale
                        d.rectangle([px, py, px + scale - 1, py + scale - 1], fill=c)
        return xy[0] + len(s) * 8 * scale

    def width(s, scale):
        return len(s) * 8 * scale

    def centre(img, y, s, scale, colour, cols=None):
        text(img, ((img.width - width(s, scale)) // 2, y), s, scale, colour, cols)

    def logo(img, x, y, scale):
        text(img, (x + 8 * scale, y), "BON", scale, WHITE)
        text(img, (x, y + 17 * scale), "BLOOP", scale, WHITE, [WHITE] + TRACK)

    def strip(img, y, h=8):
        d = ImageDraw.Draw(img)
        w = img.width // 4
        for i, c in enumerate(TRACK):
            d.rectangle([i * w, y, (i + 1) * w - 1, y + h - 1], fill=c)

    def screen(name, scale=2):
        im = Image.open(screens / f"{name}.ppm").convert("RGB")
        return im.resize((240 * scale, 240 * scale), Image.NEAREST)

    def framed(img, scr, x, y, accent):
        d = ImageDraw.Draw(img)
        d.rectangle([x - 10, y - 10, x + scr.width + 9, y + scr.height + 9], fill="#0d0d10", outline=accent, width=3)
        img.paste(scr, (x, y))

    made = []

    # 1. the hero: the logo, what it is, three screens
    img = Image.new("RGB", (1600, 900), BG)
    strip(img, 0)
    logo(img, 70, 70, 6)
    text(img, (70, 300), "SYNTH MODE", 5, WHITE)
    text(img, (70, 400), "FOR THE M-VAVE FM-1", 3, GRAY)
    y = 480
    for s, c in (("3 OSCILLATORS . 16 WAVES", BLUE), ("WAVETABLES + REVERSE", GREEN), ("DRIVE . TRANSIENT . PHASER", YELLOW),
                 ("CHORUS . DELAY . REVERB", ORANGE)):
        text(img, (70, y), s, 2, c)
        y += 48
    text(img, (70, 730), "FREE . OPEN SOURCE . INSTALLS FROM THE BROWSER", 2, DIM)
    text(img, (70, 790), URL, 3, WHITE)
    framed(img, screen("synth-osc1-playing"), 1010, 90, BLUE)
    small = [screen(n, 1) for n in ("synth-drive", "synth-osc3-vocal")]
    framed(img, small[0], 1010, 610, YELLOW)
    framed(img, small[1], 1290, 610, GREEN)
    p = out / "bonbloop-hero-1600x900.png"
    img.save(p)
    made.append(p)

    # 2. the features: four cards, a screen each
    img = Image.new("RGB", (1600, 900), BG)
    strip(img, 0)
    centre(img, 40, "WHAT SYNTH MODE ADDS", 4, WHITE)
    cards = [("3 OSC", "SIN TRI SAW SQR", "NOISE + 10 TABLES", "synth-osc2-saw", BLUE),
             ("WAVETABLE", "BASIC PWM VOCAL", "DIGI FOLD FM CHIP", "synth-osc3-vocal", GREEN),
             ("REVERSE", "PLAY AN OSCILLATOR", "BACKWARDS", "synth-rev-swell", YELLOW),
             ("FX", "DRIVE A-E-N-T-P", "TRANSIENT PHASER", "synth-drive", ORANGE)]
    for i, (title, l1, l2, scr, c) in enumerate(cards):
        x = 40 + i * 390
        d = ImageDraw.Draw(img)
        d.rectangle([x, 150, x + 360, 860], fill=TILE)
        d.rectangle([x, 150, x + 360, 157], fill=c)
        text(img, (x + 20, 180), title, 4, c)
        text(img, (x + 20, 260), l1, 2, GRAY)
        text(img, (x + 20, 300), l2, 2, GRAY)
        im = screen(scr, 1)
        img.paste(im.resize((320, 320), Image.NEAREST), (x + 20, 380))
        text(img, (x + 20, 740), "HOLD ENV" if i == 0 else ("PRESETS: 16 SOUNDS" if i == 1 else
             ("REV SWELL" if i == 2 else "+ CHORUS DLY REV")), 2, DIM)
    p = out / "bonbloop-features-1600x900.png"
    img.save(p)
    made.append(p)

    # 3. how to install
    img = Image.new("RGB", (1600, 900), BG)
    strip(img, 0)
    logo(img, 70, 70, 4)
    text(img, (420, 100), "INSTALL IN 1 MINUTE", 4, WHITE)
    text(img, (420, 170), "FROM THE BROWSER, OVER USB", 2, GRAY)
    steps = [("1", "PLUG THE FM-1 INTO YOUR PC (USB)", BLUE), ("2", "OPEN IN CHROME OR EDGE:", GREEN),
             ("", URL, WHITE), ("3", "CLICK INSTALL", YELLOW), ("4", "HOLD ENV: SYNTH MODE", ORANGE)]
    y = 320
    for n, s, c in steps:
        if n:
            d = ImageDraw.Draw(img)
            d.rectangle([70, y - 6, 70 + 60, y + 54], fill=c)
            text(img, (70 + 14, y - 2), n, 3, BG)
            text(img, (170, y + 4), s, 3, WHITE)
        else:
            text(img, (170, y - 14), s, 3, c)
        y += 100 if n else 80
    text(img, (70, 820), "BASED ON SLOOP + FELUCCA . GPL-3.0 . NOT AFFILIATED WITH M-VAVE", 2, DIM)
    framed(img, screen("synth-boot", 2), 1060, 300, BLUE)
    p = out / "bonbloop-install-1600x900.png"
    img.save(p)
    made.append(p)

    # 4. a square one (feeds, avatars of posts)
    img = Image.new("RGB", (1080, 1080), BG)
    strip(img, 0, 10)
    logo(img, (1080 - 40 * 6) // 2, 70, 6)
    centre(img, 300, "SYNTH MODE", 5, WHITE)
    centre(img, 395, "FOR THE M-VAVE FM-1", 3, GRAY)
    framed(img, screen("synth-osc1-playing", 2), (1080 - 480) // 2, 480, BLUE)
    centre(img, 1000, URL, 2, WHITE)
    p = out / "bonbloop-square-1080.png"
    img.save(p)
    made.append(p)
    return made


def icons():
    """the site's app icons (web/icon-*.png, web/make_site.py): BON over BLOOP on black; the maskable one with room
    for the launcher's mask (the logo inside the middle 60 %)"""
    from PIL import Image, ImageDraw
    made = []
    for name, size, frac in (("icon-192.png", 192, 0.8), ("icon-512.png", 512, 0.8), ("icon-maskable-512.png", 512, 0.6)):
        sc = max(1, int(size * frac) // 40)
        img = Image.new("RGB", (size, size), BG)
        d = ImageDraw.Draw(img)
        x0, y0 = (size - 40 * sc) // 2, (size - 34 * sc) // 2
        for text, ox, oy, cols in (("BON", 8, 0, [WHITE] * 3), ("BLOOP", 0, 18, [WHITE] + TRACK)):
            for i, ch in enumerate(text):
                for y, bits in enumerate(glyph(ch)):
                    for x in range(8):
                        if bits & (0x80 >> x):
                            px, py = x0 + (ox + i * 8 + x) * sc, y0 + (oy + y) * sc
                            d.rectangle([px, py, px + sc - 1, py + sc - 1], fill=cols[i])
        p = ROOT / "web" / name
        img.save(p)
        made.append(p)
    return made


def main():
    if len(sys.argv) >= 2 and sys.argv[1] == "logo":
        out = ROOT / "assets" / "logo" / "bonbloop-logo.svg"
        out.write_text(logo_svg(), encoding="utf-8")
        print(f"logo: {out} ({out.stat().st_size} B)")
        for p in icons():
            print(f"icon: {p}")
    elif len(sys.argv) == 4 and sys.argv[1] == "promo":
        for p in promo(Path(sys.argv[2]), Path(sys.argv[3])):
            print(f"promo: {p}")
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
