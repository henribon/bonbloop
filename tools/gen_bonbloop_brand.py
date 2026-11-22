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
:root { --bg: #0a0a0a; --fg: #fafafa; --solid: #cfcfcf; --dim: #464646; --muted: #8a8a8a; --line: rgba(255,255,255,.09); }
* { box-sizing: border-box; margin: 0; }
html, body { width: WPX; height: HPX; overflow: hidden; background: var(--bg); color: var(--fg);
             font: 500 16px/1.4 "Inter Tight", sans-serif; -webkit-font-smoothing: antialiased; }
body { position: relative; }
.glow { position: absolute; inset: 0; background: radial-gradient(900px 700px at 30% 20%, rgba(255,255,255,.06), transparent 62%); }
.grain { position: absolute; inset: 0; opacity: .035; background-image: url("data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' width='160' height='160'><filter id='n'><feTurbulence type='fractalNoise' baseFrequency='.8' numOctaves='3'/><feColorMatrix type='saturate' values='0'/></filter><rect width='160' height='160' filter='url(%23n)'/></svg>"); }
.page { position: absolute; inset: 0; display: flex; flex-direction: column; }
.mark { overflow: hidden; padding-top: 28px; }
.mark span { display: block; width: max-content; white-space: nowrap; line-height: .74; letter-spacing: -.035em;
             font-size: 200px; font-weight: 800; color: var(--solid); }
.pad { padding: 0 64px; }
.kicker { font-size: 18px; font-weight: 700; letter-spacing: .14em; text-transform: uppercase; color: var(--muted); }
.lead { font-size: 40px; line-height: 1.15; font-weight: 700; letter-spacing: -.02em; }
.rows { border-bottom: 1px solid var(--line); }
.row { display: flex; justify-content: space-between; align-items: baseline; gap: 32px; padding: 16px 0; border-top: 1px solid var(--line); }
.row b { font-size: 34px; font-weight: 700; letter-spacing: -.015em; text-transform: uppercase; white-space: nowrap; }
.row span { font-size: 21px; color: var(--muted); text-align: right; }
.row i { font-style: normal; font-size: 22px; font-weight: 700; color: var(--muted); width: 56px; display: inline-block; }
.screen { display: block; image-rendering: pixelated; border: 1px solid rgba(255,255,255,.14); }
.foot { display: flex; justify-content: space-between; font-size: 17px; letter-spacing: .14em; text-transform: uppercase; color: var(--muted); }
.foot b { color: var(--fg); font-weight: 700; }
"""
FIT_JS = """
<script>
(function () {
  function fit() {
    document.querySelectorAll('[data-fit]').forEach(function (el) {
      var target = el.parentElement.clientWidth;
      el.style.fontSize = '200px';
      var natural = el.getBoundingClientRect().width;
      el.style.fontSize = (200 * target / natural).toFixed(2) + 'px';
    });
  }
  fit();
  document.fonts.ready.then(fit);
})();
</script>"""


def promo(screens, out):
    """the promo images in bonbap.com.br's look: HTML pages (Inter Tight from Google Fonts, the site's colours, its
    giant word, its lines) shot by Edge headless at their size; the FM-1's screens in grey, as the site is"""
    import subprocess
    import tempfile
    from PIL import Image, ImageOps

    out.mkdir(parents=True, exist_ok=True)
    html_dir = Path(tempfile.mkdtemp(prefix="bonbloop-promo-"))
    for name in ("synth-osc1-playing", "synth-osc3-vocal", "synth-drive", "synth-rev-swell", "synth-boot", "synth-osc2-saw"):
        im = ImageOps.grayscale(Image.open(screens / f"{name}.ppm").convert("RGB"))
        im = ImageOps.autocontrast(im, cutoff=0)
        im.resize((480, 480), Image.NEAREST).save(html_dir / f"{name}.png")

    def page(w, h, body):
        return (f'<!doctype html><html><head><meta charset="utf-8">'
                '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Inter+Tight:wght@500;700;800&display=block">'
                f'<style>{PROMO_CSS.replace("WPX", f"{w}px").replace("HPX", f"{h}px")}</style></head><body>'
                f'<div class="glow"></div><div class="grain"></div><div class="page">{body}</div>{FIT_JS}</body></html>')

    rows = lambda items: '<div class="rows">' + "".join(f'<div class="row"><b>{a}</b><span>{b}</span></div>' for a, b in items) + "</div>"
    foot = (f'<div class="foot pad" style="margin-top:auto;padding-bottom:40px"><b>{URL} &#8599;</b>'
            '<span>free &middot; open source &middot; gpl-3.0</span></div>')
    cards = {
        "bonbloop-hero-1600x900.png": (1600, 900,
            '<div class="mark"><span data-fit>bonbloop</span></div>'
            '<div class="pad" style="display:flex;gap:56px;margin-top:40px">'
            '<div style="flex:1"><p class="kicker">Firmware for the M-VAVE FM-1</p>'
            '<p class="lead" style="margin:18px 0 28px">SLOOP + a synth mode. Hold ENV: your track becomes a '
            'three-oscillator wavetable synth.</p>'
            + rows([("3 oscillators", "16 waves each"), ("Reverse", "play it backwards"), ("FX", "drive, transient, phaser")]) +
            '</div><img class="screen" src="synth-osc1-playing.png" style="width:400px;height:400px"></div>' + foot),
        "bonbloop-features-1600x900.png": (1600, 900,
            '<div class="pad" style="display:flex;justify-content:space-between;align-items:baseline;padding-top:56px">'
            '<p class="kicker">What the synth mode adds</p><p class="kicker" style="color:var(--fg)">bonbloop</p></div>'
            '<div class="pad" style="display:flex;gap:56px;margin-top:36px">'
            '<div style="flex:1">'
            + rows([("3 oscillators", "each tuned, levelled, reversible"),
                    ("16 waves", "sin tri saw sqr noise + 10 wavetables"),
                    ("Wavetables", "basic pwm harm reso vocal digi fold fm bell chip"),
                    ("Reverse", "an oscillator swells in, backwards"),
                    ("Drive", "five styles, tone, punish"),
                    ("Transient", "attack and sustain"),
                    ("Phaser · chorus", "delay · reverb · filter lp bp hp")]) +
            '</div><div style="display:flex;flex-direction:column;gap:24px">'
            '<img class="screen" src="synth-osc3-vocal.png" style="width:300px;height:300px">'
            '<img class="screen" src="synth-drive.png" style="width:300px;height:300px"></div></div>' + foot),
        "bonbloop-install-1600x900.png": (1600, 900,
            '<div class="mark"><span data-fit>bonbloop</span></div>'
            '<div class="pad" style="display:flex;gap:56px;margin-top:40px">'
            '<div style="flex:1"><p class="kicker">Install in a minute, from the browser</p>'
            '<div class="rows" style="margin-top:22px">'
            + "".join(f'<div class="row"><b><i>{n}</i>{s}</b></div>' for n, s in (
                ("01", "Plug the FM-1 in over USB"), ("02", "Open " + URL + " in Chrome or Edge"),
                ("03", "Click install"), ("04", "Hold ENV: synth mode"))) +
            '</div></div><img class="screen" src="synth-boot.png" style="width:400px;height:400px"></div>' + foot),
        "bonbloop-square-1080.png": (1080, 1080,
            '<div class="mark"><span data-fit>bonbloop</span></div>'
            '<div class="pad" style="margin-top:36px"><p class="kicker">Firmware for the M-VAVE FM-1</p>'
            '<p class="lead" style="margin:16px 0 0">SLOOP + a synth mode: 3 oscillators, 16 waves, reverse, FX.</p></div>'
            '<div class="pad" style="display:flex;justify-content:center;margin-top:44px">'
            '<img class="screen" src="synth-osc1-playing.png" style="width:440px;height:440px"></div>' + foot),
    }
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
            raise SystemExit(f"promo: Edge made no {dst}")
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
