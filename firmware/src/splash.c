/* SPDX-License-Identifier: GPL-3.0-only */
/* SLOOP boot splash: the logo (tools/gen_logo.py -> sloop_logo.h: 16 colours, RLE runs of
 * (run - 1) << 4 | colour), drawn into the canvas in bands of at most 120 rows. */
#ifndef UI_SMALL                 /* (2.7) compiled for size: not the audio path (gfx.c UI_SMALL); the host tests: nothing */
#define UI_SMALL
#endif
#ifndef FELUCCA_LOGO_TEXT       /* (2.7) 1: the boot logo as text (4.6 KB of flash less than the picture). BONBLOOP: the
                                 * default, "BON" over "BLOOP" (the room the WAVE engine and the SYNTH MODE take); 0: SLOOP's
                                 * picture (tools/gen_logo.py) */
#define FELUCCA_LOGO_TEXT 1
#endif
#if FELUCCA_LOGO_TEXT
/* (BONBLOOP 1.0.1) the logo, as the site and the editor show it and as laid out in Figma: BON (light lilac) on top,
 * the Magellanic penguin's head (bonbloop_penguin.h, tools/gen_bonbloop_brand.py: its runs, (run - 1) << 4 | colour,
 * 0 the black) twice as big in the middle, BLOOP (Very Peri) under it, the words twice the screen's large type. The
 * first 1.0.1 build: the penguin three times as big over BON / BLOOP; up to 1.9.2: BON over BLOOP alone */
#include "bonbloop_penguin.h"
#define PENGUIN_K 2u
/* a word twice as big: drawn small at the top left, then doubled in place from the end (nothing read is overwritten
 * before), its ink centred (the advance has room around it), the band blitted at y */
UI_SMALL static void logo_word(const char *s, uint16_t col, uint32_t y)
{
    uint32_t w = text_w(&FONT_L, s), h = 24u, x0 = w, x1 = 0, off, yy, x;
    cv_begin(240, h * 2u, C_BLACK);
    cv_text(0, -8, &FONT_L, s, col);                    /* (the capitals from the band's top: the cell's 8 rows above) */
    for (yy = 0; yy < h; yy++)                          /* (the ink's first and last columns) */
        for (x = 0; x < w; x++)
            if (cv_px[yy * 240u + x] != swap16(C_BLACK)) {
                if (x < x0)
                    x0 = x;
                if (x > x1)
                    x1 = x;
            }
    if (x0 > x1)
        x0 = 0, x1 = w - 1u;
    off = 120u - (x1 - x0 + 1u) - x0 * 2u;
    for (yy = h * 2u; yy-- > 0;)
        for (x = w * 2u; x-- > 0;)
            cv_px[yy * 240u + off + x] = cv_px[(yy / 2u) * 240u + x / 2u];
    for (yy = 0; yy < h * 2u; yy++)                     /* (the small one's left-over columns) */
        for (x = 0; x < off; x++)
            cv_px[yy * 240u + x] = swap16(C_BLACK);
    cv_blit(0, y);
}
/* y0 6: BON's capitals from y 10 to 37, BLOOP's from 160 to 187, the penguin (90 x 72) centred across and halfway
 * between them, 63 to 134 */
UI_SMALL static void sloop_logo_draw(uint32_t y0)
{
    uint32_t i, pos = 0, off = (240u - PENGUIN_W * PENGUIN_K) / 2u;
    logo_word("BON", RGB(217, 218, 244), y0);
    logo_word("BLOOP", RGB(102, 103, 171), y0 + 150u);
    cv_begin(240, PENGUIN_H * PENGUIN_K, C_BLACK);      /* (72 rows: one band of the canvas) */
    for (i = 0; i < sizeof PENGUIN_RLE; i++) {
        uint32_t run = (PENGUIN_RLE[i] >> 4) + 1u, c = PENGUIN_RLE[i] & 15u;
        for (; run; run--, pos++)
            if (c) {
                uint32_t x = off + (pos % PENGUIN_W) * PENGUIN_K, y = (pos / PENGUIN_W) * PENGUIN_K, a, b;
                for (b = 0; b < PENGUIN_K; b++)
                    for (a = 0; a < PENGUIN_K; a++)
                        cv_px[(y + b) * 240u + x + a] = swap16(PENGUIN_PAL[c]);
            }
    }
    cv_blit(0, y0 + 57u);
}
#else
#include "sloop_logo.h"
UI_SMALL static void sloop_logo_draw(uint32_t y0)
{
    uint32_t r0;
    for (r0 = 0; r0 < SLOOP_SPLASH_H; r0 += 120u) {
        uint32_t rows = SLOOP_SPLASH_H - r0 > 120u ? 120u : SLOOP_SPLASH_H - r0;
        uint32_t lo = r0 * SLOOP_SPLASH_W, hi = (r0 + rows) * SLOOP_SPLASH_W, pos = 0, i;
        cv_begin(SLOOP_SPLASH_W, rows, C_BLACK);
        for (i = 0; i < sizeof SLOOP_SPLASH_RLE && pos < hi; i++) {
            uint32_t run = (SLOOP_SPLASH_RLE[i] >> 4) + 1u, c = SLOOP_SPLASH_RLE[i] & 15u, p;
            if (c && pos + run > lo)
                for (p = pos < lo ? lo : pos; p < pos + run && p < hi; p++)
                    cv_px[p - lo] = swap16(SLOOP_SPLASH_PAL[c]);
            pos += run;
        }
        cv_blit((240u - SLOOP_SPLASH_W) / 2u, y0 + r0);
    }
}
#endif

/* the boot screen: the logo and the version (BONBLOOP 1.0.1: "based on sloop + felucca" under it no more; the
 * README and the site credit them) */
UI_SMALL static void sloop_splash(void)
{
    lcd_fill(0, 0, 240, 240, C_BLACK);
    sloop_logo_draw(6);
    const char *v = FELUCCA_VERSION, *sp = v;
    while (*sp && *sp != ' ')
        sp++;
    if (*sp)
        v = sp + 1;                                     /* "BONBLOOP 1.0" -> "1.0" under the wordmark */
    cv_begin(240, 18, C_BLACK);
    cv_text(120 - text_w(&FONT_S, v) / 2, 0, &FONT_S, v, RGB(217, 218, 244));
    cv_blit(0, 202);
}
