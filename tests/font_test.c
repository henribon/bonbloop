/* SPDX-License-Identifier: GPL-3.0-only */
/* The screen font (gfx.c cv_text, tools/gen_font.py). BONBLOOP 1.7: Pixelify Sans (assets/fonts/PixelifySans-
 * Variable.ttf, SIL OFL 1.1), a pixel font at its own grid (11 px, capitals 7 px) in Terminus' 16-pixel cell (baseline
 * on row 12), capitals only, every pixel on or off (bits = 1); FONT_L is FONT_S at 2x; FONT_T the names and units, in
 * an 11-pixel cell. Checked: the format (cell, range, the 2x font on S's bitmaps); every glyph drawn has ink inside its
 * cell and nothing outside its bitmap; lower case shows as capitals in the same place; the figures are one width; the
 * U with umlaut is its own glyph (127), not '?'; the 5-letter labels fit a column's 41 pixels; one lit level (no
 * blur); the capitals 7 pixels tall, on the baseline; the data size. Exit status: the failed checks. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static void lcd_sync(void) {}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p) { (void)x; (void)y; (void)w; (void)h; (void)p; }
#define __attribute__(x)         /* (as tests/hostsim.c: gfx.c puts its canvas in the pool section, which clang on
                                 * macOS refuses for a host build; sloop-lab, 11/10/2026) */
#include "../firmware/src/gfx.c"
static uint32_t st_crc32(const void *p, uint32_t n) { (void)p; (void)n; return 0; }   /* (storage.c's: not here) */
static int bad;
static void check(int ok, const char *what) { printf("font: %-70s %s\n", what, ok ? "ok" : "FAIL"); bad += !ok; }
static uint32_t hash_cv(uint32_t h)
{
    uint32_t i;
    for (i = 0; i < cv_w * cv_h; i++)
        h = (h ^ cv_px[i]) * 16777619u;
    return h;
}
/* one character drawn at (8, 4) on a 40 x 40 black canvas in white: its lit pixels, and whether all of them lie in
 * x 8 - pad .. 8 - pad + bw - 1 and in the cell's rows */
static uint32_t ink(const felucca_font_t *f, uint32_t c, int *inside)
{
    char s[2] = {(char)c, 0};
    uint32_t gi = glyph(f, c), x, y, n = 0;
    int32_t x0 = 8 - (int32_t)f->pad, x1 = x0 + (int32_t)f->bw[gi];
    cv_begin(40, 40, 0);
    cv_text(8, 4, f, s, 0xFFFFu);
    *inside = 1;
    for (y = 0; y < 40u; y++)
        for (x = 0; x < 40u; x++)
            if (cv_px[y * 40u + x]) {
                n++;
                if ((int32_t)x < x0 || (int32_t)x >= x1 || y < 4u || y >= 4u + f->h)
                    *inside = 0;
            }
    return n;
}

int main(void)
{
    uint32_t c, bare = 0, out = 0, levels = 0, i;
    int inside;
    check(FONT_S.h == 16u && FONT_S.first == 32u && FONT_S.last == 127u && FONT_S.bits == 1u,
          "FONT_S: Pixelify Sans in the 16-pixel cell, 32..127, 1 bit a pixel");
    check(FONT_L.data == FONT_S.data && FONT_L.sh == 1u && FONT_L.h == 32u && FONT_L.bits == 1u,
          "FONT_L: FONT_S's bitmaps drawn at 2x (no copy)");
    for (c = 33; c < 127u; c++) {                     /* every character but the space: ink, inside its box */
        if ((c >= 'a' && c <= 'z') || c == '{' || c == '}')
            continue;
        if (!ink(&FONT_S, c, &inside))
            bare++;
        out += !inside;
    }
    check(!bare && !out, "every character drawn: some ink, all of it inside its bitmap and the cell");
    {
        uint32_t a, b;
        ink(&FONT_S, 0xDCu, &inside);
        a = hash_cv(1u);
        ink(&FONT_S, '?', &inside);
        b = hash_cv(1u);
        check(a != b && glyph(&FONT_S, 0xDCu) == 127u - 32u && glyph(&FONT_S, 0xFCu) == 127u - 32u,
              "the U with umlaut (HUGELTON) is its own glyph, not '?' (upper and lower case)");
    }
    {
        cv_begin(120, 20, 0);
        cv_text(2, 2, &FONT_S, "kick 12", 0xFFFFu);
        c = hash_cv(1u);
        cv_begin(120, 20, 0);
        cv_text(2, 2, &FONT_S, "KICK 12", 0xFFFFu);
        check(c == hash_cv(1u) && text_w(&FONT_S, "kick") == text_w(&FONT_S, "KICK"),
              "lower case shows as capitals, in the same place");
    }
    for (c = '1'; c <= '9'; c++)
        if (FONT_S.adv[c - 32u] != FONT_S.adv[0])
            break;
    for (c = '1'; c <= '9' && FONT_S.adv[c - 32u] == FONT_S.adv['0' - 32u]; c++)
        ;
    check(c > '9', "the figures all one width (a value does not wobble as it turns)");
    {
        static const char *const W[] = {"PUNSH", "DTUNE", "SLOT1", "RANGE", "SYNTH", "SWING", "DEPTH", "RATIO"};
        int32_t widest = 0;
        for (i = 0; i < sizeof W / sizeof W[0]; i++)
            widest = text_w(&FONT_S, W[i]) > widest ? text_w(&FONT_S, W[i]) : widest;
        check(widest <= 41, "the 5-letter labels fit a column's 41 pixels");
    }
    {   /* a pixel font: an O on black, in white, has one lit level (no half-lit edge) */
        uint16_t seen[4] = {0, 0, 0, 0};
        uint32_t n = 0;
        cv_begin(40, 40, 0);
        cv_text(8, 4, &FONT_S, "O", 0xFFFFu);
        for (i = 0; i < 1600u; i++) {
            uint16_t v = cv_px[i], k;
            if (!v)
                continue;
            for (k = 0; k < n && seen[k] != v; k++)
                ;
            if (k == n && n < 4u)
                seen[n++] = v;
        }
        levels = n;
        check(levels == 1u, "one lit level: every pixel on or off, sharp");
    }
    {   /* the capitals 7 pixels tall, standing on row 12 (the baseline, as Terminus and Inter Tight before) */
        uint32_t top = 99, bot = 0, y, x;
        cv_begin(40, 40, 0);
        cv_text(8, 4, &FONT_S, "H", 0xFFFFu);
        for (y = 0; y < 40u; y++)
            for (x = 0; x < 40u; x++)
                if (cv_px[y * 40u + x]) {
                    top = y < top ? y : top;
                    bot = y > bot ? y : bot;
                }
        check(bot - top + 1u == 7u && bot == 4u + 11u, "the capitals 7 pixels tall, on the baseline (row 12)");
    }
    check(sizeof FONT_S_DATA <= 1536u, "the bitmaps in 1.5 KB at most (Terminus took 5376 bytes)");
    /* (1.7) FONT_T: the names over the values and the units beside them, the same letters a pixel apart */
    check(FONT_T.h == 11u && FONT_T.first == 32u && FONT_T.last == 127u && FONT_T.bits == 1u && FONT_T.sh == 0u &&
          FONT_T.data != FONT_S.data, "FONT_T: its own bitmaps, the 11-pixel cell, 32..127, 1 bit a pixel");
    for (c = 33, bare = out = 0; c < 127u; c++) {
        if ((c >= 'a' && c <= 'z') || c == '{' || c == '}')
            continue;
        if (!ink(&FONT_T, c, &inside))
            bare++;
        out += !inside;
    }
    check(!bare && !out, "FONT_T: every character drawn, all of it inside its bitmap and the cell");
    {
        static const char *const N[] = {"PUNSH", "DTUNE", "SLOT1", "RANGE", "PHASR", "DELAY", "RELEASE", "THRESH"};
        int32_t widest = 0, spaced = 1;
        for (i = 0; i < sizeof N / sizeof N[0]; i++) {
            if (i < 6u)                              /* (the column names; RELEASE, THRESH: the COMP's, given room) */
                widest = text_w(&FONT_T, N[i]) > widest ? text_w(&FONT_T, N[i]) : widest;
            spaced &= text_w(&FONT_T, N[i]) == text_w(&FONT_S, N[i]) + (int32_t)strlen(N[i]);   /* (a pixel apart) */
        }
        check(widest <= 41 && spaced, "FONT_T: the letters a pixel further apart, a column's names in its 41 pixels");
    }
    check(sizeof FONT_T_DATA <= 1024u, "FONT_T: the bitmaps in 1 KB at most");
    printf("%s\n", bad ? "FONT TEST FAILED" : "font test passed");
    return bad;
}
