#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""BONBLOOP's look outside the FM-1, in the FM-1's own font (Terminus Bold 8x16, assets/fonts) and colours.

  tools/gen_bonbloop_brand.py logo              assets/logo/bonbloop-logo.svg and web/icon-*.png (the site's, web/make_site.py)
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
EDGE = Path(r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe")

PROMO_CSS = """
:root { --bg: #0a0a0a; --fg: #fafafa; --solid: #cfcfcf; --muted: #8a8a8a; --dim: #5a5a5a; --tile: #141416;
        --line: rgba(255,255,255,.09); --blue: #287cff; --green: #1ecc70; --yellow: #ffc618; --orange: #ff621a; }
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
    """the promo images: the layout they had (the tracks' colours, the FM-1's screens in colour, the cards and steps)
    in bonbap.com.br's type: Inter Tight, the lower-case word, #0a0a0a. HTML pages shot by Edge headless"""
    import subprocess
    import tempfile
    from PIL import Image

    out.mkdir(parents=True, exist_ok=True)
    html_dir = Path(tempfile.mkdtemp(prefix="bonbloop-promo-"))
    for name in ("synth-osc1-playing", "synth-osc3-vocal", "synth-drive", "synth-rev-swell", "synth-boot", "synth-osc2-saw"):
        Image.open(screens / f"{name}.ppm").convert("RGB").resize((480, 480), Image.NEAREST).save(html_dir / f"{name}.png")

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

    hero = (at(70, 62, "word", "bonbloop", "font-size:150px")
            + at(70, 262, "big", "Synth mode", "font-size:84px")
            + at(70, 366, "sub", "for the M-VAVE FM-1", "font-size:36px")
            + at(70, 470, "line", "3 oscillators · 16 waves", "font-size:30px;color:var(--blue)")
            + at(70, 518, "line", "wavetables + reverse", "font-size:30px;color:var(--green)")
            + at(70, 566, "line", "drive · transient · phaser", "font-size:30px;color:var(--yellow)")
            + at(70, 614, "line", "chorus · delay · reverb", "font-size:30px;color:var(--orange)")
            + at(70, 728, "small", "free · open source · installs from the browser", "font-size:20px")
            + at(70, 780, "line", URL, "font-size:40px;text-transform:none;letter-spacing:-.01em")
            + scr("synth-osc1-playing", 1010, 90, 480, "blue")
            + scr("synth-drive", 1010, 610, 240, "yellow") + scr("synth-osc3-vocal", 1290, 610, 240, "green"))
    cards = ""
    for i, (title, l1, l2, s, c, foot) in enumerate((
            ("3 osc", "sin tri saw sqr", "noise + 10 tables", "synth-osc2-saw", "blue", "hold env"),
            ("wavetable", "basic pwm vocal", "digi fold fm chip", "synth-osc3-vocal", "green", "presets: 16 sounds"),
            ("reverse", "play an oscillator", "backwards", "synth-rev-swell", "yellow", "rev swell"),
            ("fx", "drive a-e-n-t-p", "transient phaser", "synth-drive", "orange", "+ chorus dly rev"))):
        x = 40 + i * 390
        cards += (f'<div class="card" style="left:{x}px;top:150px;width:360px;height:710px;border-top:8px solid var(--{c})"></div>'
                  + at(x + 22, 186, "big", title, f"font-size:52px;color:var(--{c})")
                  + at(x + 22, 262, "sub", l1, "font-size:26px") + at(x + 22, 300, "sub", l2, "font-size:26px")
                  + f'<img class="abs" src="{s}.png" style="left:{x + 20}px;top:380px;width:320px;height:320px;image-rendering:pixelated">'
                  + at(x + 22, 742, "small", foot, "font-size:20px"))
    features = at(0, 40, "big", "what synth mode adds", "width:1600px;text-align:center;font-size:58px") + cards
    steps = ""
    y = 340
    for n, s, c in (("1", "plug the FM-1 into your pc (usb)", "blue"), ("2", "open in chrome or edge:", "green"), ("", URL, ""),
                    ("3", "click install", "yellow"), ("4", "hold env: synth mode", "orange")):
        if n:
            steps += (f'<div class="abs" style="left:70px;top:{y - 6}px;width:60px;height:60px;background:var(--{c});'
                      f'color:#0a0a0a;font:800 40px/60px Inter Tight;text-align:center">{n}</div>'
                      + at(170, y + 4, "line", s, "font-size:38px"))
            y += 92
        else:
            steps += at(170, y - 16, "line", s, "font-size:38px;text-transform:none;color:var(--fg)")
            y += 70
    install = (at(70, 62, "word", "bonbloop", "font-size:110px")
               + at(70, 196, "big", "install in 1 minute", "font-size:60px")
               + at(70, 268, "sub", "from the browser, over usb", "font-size:26px;color:var(--muted)")
               + steps
               + at(70, 820, "small", "based on sloop + felucca · gpl-3.0 · not affiliated with m-vave", "font-size:20px")
               + scr("synth-boot", 1060, 300, 480, "blue"))
    square = (at(0, 70, "word", "bonbloop", "width:1080px;text-align:center;font-size:170px")
              + at(0, 300, "big", "synth mode", "width:1080px;text-align:center;font-size:84px")
              + at(0, 400, "sub", "for the M-VAVE FM-1", "width:1080px;text-align:center;font-size:36px")
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
    """the site's app icons (web/icon-*.png, web/make_site.py), in bonbap.com.br's look: bonbap's mark is a grey dot on
    #0a0a0a; bonbloop's, a grey ring (the loop). The maskable one: the ring inside the middle 60 %, the square full"""
    from PIL import Image, ImageDraw
    made = []
    for name, size, frac, rounded in (("icon-192.png", 192, .5, True), ("icon-512.png", 512, .5, True),
                                      ("icon-maskable-512.png", 512, .38, False)):
        k = 4                                          # drawn 4x, then down: smooth edges
        S = size * k
        img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
        d = ImageDraw.Draw(img)
        if rounded:
            d.rounded_rectangle([0, 0, S - 1, S - 1], radius=S // 4, fill="#0a0a0a")
        else:
            d.rectangle([0, 0, S - 1, S - 1], fill="#0a0a0a")
        r, w = int(S * frac / 2), int(S * frac * .22)
        d.ellipse([S // 2 - r, S // 2 - r, S // 2 + r, S // 2 + r], outline="#cfcfcf", width=w)
        img = img.resize((size, size), Image.LANCZOS)
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
