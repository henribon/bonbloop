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
/* one word of the wordmark: small at the top left, letter i in col[i], then twice as big in place (from the end:
 * nothing read is overwritten before), centred, at y */
UI_SMALL static void logo_word(const char *s, const uint16_t *col, uint32_t y)
{
    uint32_t i, w = text_w(&FONT_L, s), k = 2u, h = 32u, off = (240u - w * k) / 2u, yy, x;
    int32_t px = 0;
    char c[2] = {0, 0};
    cv_begin(240, h * k, C_BLACK);
    for (i = 0; s[i]; i++) {
        c[0] = s[i];
        px = cv_text(px, 0, &FONT_L, c, col[i]);
    }
    for (yy = h * k; yy-- > 0;)
        for (x = w * k; x-- > 0;)
            cv_px[yy * 240u + off + x] = cv_px[(yy / k) * 240u + x / k];
    for (yy = 0; yy < h * k; yy++)                      /* (the small one's left-over columns) */
        for (x = 0; x < off; x++)
            cv_px[yy * 240u + x] = swap16(C_BLACK);
    cv_blit(0, y);
}
UI_SMALL static void sloop_logo_draw(uint32_t y0)
{
    const uint16_t bon[3] = {C_WHITE, C_WHITE, C_WHITE};
    const uint16_t bloop[5] = {C_WHITE, TE_COL_DEF[0], TE_COL_DEF[1], TE_COL_DEF[2], TE_COL_DEF[3]};
    logo_word("BON", bon, y0 + 52u);                    /* BON in white, BLOOP: B white, L O O P the tracks' colours */
    logo_word("BLOOP", bloop, y0 + 118u);
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
    sloop_logo_draw(10);
    const char *v = FELUCCA_VERSION, *sp = v;
    while (*sp && *sp != ' ')
        sp++;
    if (*sp)
        v = sp + 1;                                     /* "BONBLOOP 1.0" -> "1.0" under the wordmark */
    cv_begin(240, 36, C_BLACK);
    cv_text(120 - text_w(&FONT_S, v) / 2, 0, &FONT_S, v, RGB(196, 196, 204));
    cv_text(120 - text_w(&FONT_S, "based on sloop + felucca") / 2, 18, &FONT_S, "based on sloop + felucca",
            RGB(96, 96, 104));
    cv_blit(0, 202);
}
