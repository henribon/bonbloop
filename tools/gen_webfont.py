#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""The web editor's pixel font: BONBLOOP 1.0 uses the FM-1's own, Pixelify Sans (assets/fonts/PixelifySans-Variable.ttf,
SIL OFL 1.1, assets/fonts/PixelifySans-OFL.txt), for the titles, the pages and the main buttons. Written into
web/editor.html as a data: URI between the /*BONBLOOP-FONT*/ markers (no extra file: the editor also runs from file://,
where a font file next to it may not load). Up to 1.9.2 the editor had SLOOP's Terminus, built here from its BDF.
  tools/gen_webfont.py [editor.html]"""
import base64
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TTF = ROOT / "assets" / "fonts" / "PixelifySans-Variable.ttf"
MARK = r"/\*BONBLOOP-FONT\*/url\([^)]*\)/\*BONBLOOP-FONT\*/"


def main():
    editor = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "web" / "editor.html"
    ttf = TTF.read_bytes()
    uri = "data:font/ttf;base64," + base64.b64encode(ttf).decode()
    s = editor.read_text(encoding="utf-8")
    new, k = re.subn(MARK, lambda m: f"/*BONBLOOP-FONT*/url({uri})/*BONBLOOP-FONT*/", s)
    if k != 1:
        sys.exit(f"{editor}: no /*BONBLOOP-FONT*/url(...)/*BONBLOOP-FONT*/ marker")
    editor.write_text(new, encoding="utf-8")
    print(f"web font: Pixelify Sans, {len(ttf)} B TTF -> {editor.name}")


if __name__ == "__main__":
    main()
