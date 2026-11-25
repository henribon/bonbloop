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
/* (BONBLOOP 1.0) the logo: the Magellanic penguin's head (bonbloop_penguin.h, tools/gen_bonbloop_brand.py: its runs,
 * (run - 1) << 4 | colour, 0 the black) three times as big, then BON (light lilac) over BLOOP (Very Peri) in the
 * screen's font, as the site and the editor show them. Up to 1.9.2: BON over BLOOP alone, twice as big */
#include "bonbloop_penguin.h"
#define PENGUIN_K 3u
UI_SMALL static void sloop_logo_draw(uint32_t y0)
{
    uint32_t i, pos = 0, off = (240u - PENGUIN_W * PENGUIN_K) / 2u;
    cv_begin(240, PENGUIN_H * PENGUIN_K, C_BLACK);      /* (108 rows: one band of the canvas) */
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
    cv_blit(0, y0);
    cv_begin(240, 60, C_BLACK);
    cv_text(120 - text_w(&FONT_L, "BON") / 2, 0, &FONT_L, "BON", RGB(217, 218, 244));
    cv_text(120 - text_w(&FONT_L, "BLOOP") / 2, 26, &FONT_L, "BLOOP", RGB(102, 103, 171));
    cv_blit(0, y0 + PENGUIN_H * PENGUIN_K + 2u);
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

/* the boot screen: the logo, the version, where it comes from */
UI_SMALL static void sloop_splash(void)
{
    lcd_fill(0, 0, 240, 240, C_BLACK);
    sloop_logo_draw(6);
    const char *v = FELUCCA_VERSION, *sp = v;
    while (*sp && *sp != ' ')
        sp++;
    if (*sp)
        v = sp + 1;                                     /* "BONBLOOP 1.0" -> "1.0" under the wordmark */
    cv_begin(240, 36, C_BLACK);
    cv_text(120 - text_w(&FONT_S, v) / 2, 0, &FONT_S, v, RGB(217, 218, 244));
    cv_text(120 - text_w(&FONT_S, "based on sloop + felucca") / 2, 18, &FONT_S, "based on sloop + felucca",
            RGB(102, 103, 171));
    cv_blit(0, 202);
}
