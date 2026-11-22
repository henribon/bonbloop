/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 BONBLOOP contributors, after SLOOP (isod89) and Felucca (Leo Kuroshita, Hügelton Instruments) */
/* WAVE (BONBLOOP): a wavetable oscillator, the sound of the SYNTH MODE (hold ENV).
 * Ten tables of eight single-cycle frames (256 samples each). No flash for the waves: the table picked (TABL) is
 * built in the part's engine arena (engines.c eng_arena_of, shared with GRAIN / FM6 / PHYS: no RAM either), a frame
 * at a time in the audio ISR's per-part hook (wave_block: a few harmonics a block, ~35 ms for a whole table). A new
 * table replaces the one playing frame by frame, so a TABL change never goes silent.
 *   TABL  the table: BASIC (sine > tri > saw > square > pulse), PWM, HARM (organ: harmonics one by one), RESO (a
 *         resonant peak climbing a saw), VOCAL (vowels A E I O U), DIGI (PPG-style digital spectra), FOLD (a sine
 *         folded harder), FM (a sine modulated at 3x, the index rising), BELL (sparse glassy partials), CHIP (4-bit
 *         pulses and steps, aliasing on purpose)
 *   POS   where in the table it reads: frame 1 .. 8, read between two frames as POS moves. ENV > SHP and LFO > SHP
 *         (shown as POS on the synth pages) move it: the classic wavetable sweep
 *   WARP  the read phase bent: the first half of the cycle squeezed (brighter, PWM-like on any table)
 *   SUB   a sine an octave below
 *   CUT RES  the SVF low-pass (dsp.c tsvf_lp; ENV / LFO > FLT and CC 74 move CUT), key-tracked by half
 *   DTN   a second oscillator on the same table, DTN cents higher (0: off)
 *   DRV   drive before the filter
 * The additive tables are band-limited to 24 harmonics: clean up to ~900 Hz notes, folding softly above (as the
 * 8-bit wavetable synths of the 80s). */
#define WT_LEN 256u
#define WT_FR 8u
#define WT_NH 24u                /* harmonics of the additive tables */
#define WT_HPB 4u                /* harmonics added a block while a frame is built */
#define WT_PEAK 30000            /* each frame's peak */
enum { WT_BASIC, WT_PWM, WT_HARM, WT_RESO, WT_VOCAL, WT_DIGI, WT_FOLD, WT_FM, WT_BELL, WT_CHIP, WT_NTAB };
static const char *const N_WTAB[WT_NTAB] = {"BASIC", "PWM", "HARM", "RESO", "VOCAL", "DIGI", "FOLD", "FM", "BELL", "CHIP"};

typedef struct {                 /* a part's WAVE state: its engine arena */
    int16_t w[WT_FR][WT_LEN];    /* the table playing (a new one replaces it frame by frame) */
    int32_t acc[WT_LEN];         /* the frame being built (additive tables) */
    uint8_t want;                /* the table in w[] (or being built into it) + 1, 0 = none yet */
    uint8_t fr;                  /* the frame being built, WT_FR = done */
    uint8_t h;                   /* its next harmonic, 0 = not started */
    uint8_t ready;               /* w[] holds table want - 1 entirely */
    uint16_t gen;                /* bumped at each frame stored (the screen redraws) */
    int16_t hm[WT_FR];           /* each frame's mean over its first half (WARP's DC: wave_render takes it out) */
    int16_t pos;                 /* the position last rendered, 0..127 << 8, modulation included (the screen) */
} wt_part_t;
_Static_assert(sizeof(wt_part_t) <= sizeof(gr_part_t), "WAVE fits the arena GRAIN already takes (no RAM more)");

static wt_part_t *wt_of(const track_t *t) { return (wt_part_t *)eng_arena_of(t, ENGI_WAVE); }

/* ------------------------------------------------------------------------------------------- the tables --- */
static int32_t wt_shape(uint32_t s, uint32_t k)   /* harmonic k of a basic wave: 0 sine 1 tri 2 saw 3 square */
{
    switch (s) {
    case 0:
        return k == 1u ? 32767 : 0;
    case 1:
        return k & 1u ? (k & 2u ? -32767 : 32767) / (int32_t)(k * k) : 0;
    case 2:
        return 32767 / (int32_t)k;
    default:
        return k & 1u ? 32767 / (int32_t)k : 0;
    }
}

/* the formants of VOCAL's frames (A, A-E, E, I, I-O, O, U, ER), in quarters of a harmonic (a voice near 120 Hz) */
static const uint8_t WT_F1[WT_FR] = {27, 21, 15, 10, 13, 17, 11, 14};
static const uint8_t WT_F2[WT_FR] = {38, 50, 63, 77, 52, 28, 23, 34};
static int32_t wt_peakq(int32_t d)               /* a formant's response d quarters of a harmonic away, Q12 */
{
    return 4096 * 25 / (25 + d * d);
}

/* harmonic k (1..WT_NH) of frame f of an additive table: its amplitude (signed, |a| <= 32767) and phase (*ph,
 * 0..1023 of a turn) */
static int32_t wt_harm(uint32_t tab, uint32_t f, uint32_t k, uint32_t *ph)
{
    *ph = 0;
    switch (tab) {
    case WT_BASIC:                                    /* sine, tri, saw, square and the blends between, pulse 25 % */
        if (f == 7u) {
            *ph = 256u;
            return SINE[(k * 128u) & 1023u] / (int32_t)k;
        }
        return f & 1u ? (wt_shape(f >> 1, k) + wt_shape((f >> 1) + 1u, k)) / 2 : wt_shape(f >> 1, k);
    case WT_PWM: {                                    /* a pulse, 50 % .. 5 % */
        uint32_t d = 256u - f * 33u;
        *ph = 256u;
        return SINE[(k * d) & 1023u] / (int32_t)k;
    }
    case WT_HARM: {                                   /* an organ, one more harmonic a frame */
        static const uint8_t N[WT_FR] = {1, 2, 3, 4, 6, 8, 12, 16};
        return k <= N[f] ? 4 * 32767 / (int32_t)(k + 3u) : 0;
    }
    case WT_RESO: {                                   /* a saw through a resonant low-pass climbing the harmonics */
        static const uint8_t C[WT_FR] = {8, 12, 16, 24, 34, 48, 68, 92};   /* the cutoff, quarters of a harmonic */
        int32_t c = C[f], a = 32767 / (int32_t)k, d = (int32_t)(4u * k) - c;
        if ((int32_t)(4u * k) > c)
            a = a * c * c / (int32_t)(16u * k * k);
        return (a * (256 + 5 * 256 * 16 / (16 + d * d)) >> 8) / 6;
    }
    case WT_VOCAL: {                                  /* a glottal source through two formants */
        int32_t d1 = (int32_t)(4u * k) - WT_F1[f], d2 = (int32_t)(4u * k) - WT_F2[f];
        int32_t src = 2 * 32767 / (int32_t)(k + 1u);
        return src * (wt_peakq(d1) + wt_peakq(d2) * 7 / 10 + 200) / 7200;
    }
    case WT_DIGI: {                                   /* a digital spectrum of its own a frame (hashed, sparse) */
        uint32_t x = (f + 1u) * 2654435761u ^ (k * 40503u + 0x9E3779B9u);
        x ^= x >> 15;
        x *= 2246822519u;
        x ^= x >> 13;
        *ph = (x >> 20) & 1023u;
        return (x >> 16) & 3u ? (int32_t)(x & 32767u) * 4 / (int32_t)(k + 3u) : 0;
    }
    default: {                                        /* BELL: four sparse partials spreading up */
        int32_t a = 0;
        if (k == 1u)
            a = 32767;
        else if (k == 2u + f)
            a = 26000;
        else if (k == 5u + 2u * f)
            a = 18000;
        else if (k == 9u + 2u * f)
            a = 13000;
        return a;
    }
    }
}
static int wt_additive(uint32_t tab) { return tab != WT_FOLD && tab != WT_FM && tab != WT_CHIP; }

/* harmonic k at amplitude a, phase ph, into the frame being built */
static void wt_add(int32_t *acc, int32_t a, uint32_t k, uint32_t ph)
{
    uint32_t i, idx = ph, st = k * (1024u / WT_LEN);
    for (i = 0; i < WT_LEN; i++, idx += st)
        acc[i] += (a * SINE[idx & 1023u]) >> 8;
}

/* the frame built, normalised to WT_PEAK, into the table (32-bit only: the peak scaled under 2^15 first); its first
 * half's mean into *hm */
static void wt_store(int16_t *dst, const int32_t *acc, int16_t *hm)
{
    uint32_t i, s = 0;
    int32_t peak = 1, g, sum = 0;
    for (i = 0; i < WT_LEN; i++) {
        int32_t a = acc[i] < 0 ? -acc[i] : acc[i];
        if (a > peak)
            peak = a;
    }
    while ((peak >> s) > 32767)
        s++;
    g = (int32_t)(((uint32_t)WT_PEAK << 15) / (uint32_t)(peak >> s));
    for (i = 0; i < WT_LEN; i++)
        dst[i] = (int16_t)clamp(((acc[i] >> s) * g) >> 15, -32767, 32767);
    for (i = 0; i < WT_LEN / 2u; i++)
        sum += dst[i];
    *hm = (int16_t)(sum / (int32_t)(WT_LEN / 2u));
}

/* frame f of a table made in time (FOLD, FM, CHIP), into acc (|x| <= 32767), its DC taken out (CHIP's pulses) */
static void wt_direct(int32_t *acc, uint32_t tab, uint32_t f)
{
    uint32_t i;
    int32_t sum = 0;
    for (i = 0; i < WT_LEN; i++) {
        int32_t x, q;
        uint32_t th = i * (1024u / WT_LEN);
        if (tab == WT_FOLD) {                         /* sin(pi g sin x), g 0.5 .. 6.8: folded over and over */
            x = SINE[((SINE[th] * (int32_t)(256u + f * 460u)) >> 15) & 1023];
        } else if (tab == WT_FM) {                    /* sin(x + I sin 3x), I 0 .. 5.6 */
            x = SINE[(th + (uint32_t)((SINE[(th * 3u) & 1023u] * (int32_t)(f * 130u)) >> 15)) & 1023u];
        } else {                                      /* CHIP: 4-bit */
            switch (f) {
            case 0:
            case 1:
            case 2:                                   /* pulses 50, 25, 12.5 % */
                q = i < (128u >> f) ? 15 : 0;
                break;
            case 3:                                   /* the NES triangle */
                q = (int32_t)((i < 128u ? i : 255u - i) >> 3);
                break;
            case 4:                                   /* a stepped saw */
                q = (int32_t)(i >> 4);
                break;
            case 5:                                   /* a 3-bit sine */
                q = ((SINE[th] + 32768) >> 13) * 15 / 7;
                break;
            case 6:                                   /* a square and its octave */
                q = (i & 64u ? 5 : 0) + (i & 128u ? 10 : 0);
                break;
            default: {                                /* a looped 32-step noise */
                uint32_t r = ((i >> 3) + 1u) * 2654435761u;
                q = (int32_t)((r >> 13) & 15u);
                break;
            }
            }
            x = (q * 2 - 15) * 32767 / 15;
        }
        acc[i] = x;
        sum += x;
    }
    sum /= (int32_t)WT_LEN;
    for (i = 0; i < WT_LEN; i++)
        acc[i] -= sum;
}

/* the part's hook, once a block: the table TABL asks for, built WT_HPB harmonics (or a whole frame in time) a block */
static void wave_block(track_t *t)
{
    wt_part_t *w = wt_of(t);
    uint32_t tab, n;
    if (!w)
        return;
    tab = (uint32_t)t->p[P_E0] % WT_NTAB;
    if (w->want != tab + 1u) {                        /* another table: frame by frame over the one playing */
        w->want = (uint8_t)(tab + 1u);
        w->fr = 0;
        w->h = 0;
        w->ready = 0;
    }
    if (w->fr >= WT_FR)
        return;
    if (wt_additive(tab)) {
        if (!w->h) {
            for (n = 0; n < WT_LEN; n++)
                w->acc[n] = 0;
            w->h = 1;
        }
        for (n = 0; n < WT_HPB && w->h <= WT_NH; n++, w->h++) {
            uint32_t ph;
            int32_t a = wt_harm(tab, w->fr, w->h, &ph);
            if (a)
                wt_add(w->acc, a, w->h, ph);
        }
        if (w->h <= WT_NH)
            return;
    } else {
        wt_direct(w->acc, tab, w->fr);
    }
    wt_store(w->w[w->fr], w->acc, &w->hm[w->fr]);
    w->h = 0;
    if (++w->fr >= WT_FR)
        w->ready = 1;
    w->gen++;
}

/* ------------------------------------------------------------------------------------------ the voice --- */
static void wave_note_on(track_t *t, voice_t *v)
{
    (void)t;
    if (!v->env && !v->env_out) {                     /* a fresh note: from phase 0, the filter empty */
        v->ph[0] = 0;
        v->ph[1] = 0x2A000000u;                       /* (osc 2 not in phase with osc 1) */
        v->ph[2] = 0;
        v->s[0] = v->s[1] = 0;
    }
}

static inline int32_t wt_lerp(const int16_t *f, uint32_t ph)   /* a frame at phase ph, between two samples */
{
    uint32_t i = ph >> 24;
    int32_t a = f[i], b = f[(i + 1u) & (WT_LEN - 1u)];
    return a + (((b - a) * (int32_t)((ph >> 9) & 0x7FFFu)) >> 15);
}
/* WARP: the first half of the cycle squeezed into [0, bk), the second half stretched over the rest (k1, k2: slopes,
 * Q12; both halves stay below 2^32) */
static inline uint32_t wt_warp(uint32_t ph, uint32_t bk, uint32_t k1, uint32_t k2)
{
    return ph < bk ? (ph >> 12) * k1 : 0x80000000u + ((ph - bk) >> 12) * k2;
}
static inline int32_t wt_read(const int16_t *fa, const int16_t *fb, int32_t fr, uint32_t ph)
{
    int32_t s = wt_lerp(fa, ph);
    return fr ? s + (((wt_lerp(fb, ph) - s) * fr) >> 8) : s;
}

static void wave_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    wt_part_t *w = wt_of(t);
    const int16_t *fa, *fb;
    int32_t pos, fp, fr, sub = p[P_E3] * 258, drv = p[P_E7], drive = 32768 + p[P_E7] * 768, dw = p[P_E7] * 258;
    int32_t cut = (p[P_E4] << 8) + m->cutoff + ((v->pitch16 - 60 * 16) << 2), res = p[P_E5];
    uint32_t i, inc1 = m->inc, inc2 = 0, ph0 = v->ph[0], ph1 = v->ph[1], ph2 = v->ph[2];
    uint32_t warp = (uint32_t)p[P_E2], bk = 0x80000000u, k1 = 4096u, k2 = 4096u;
    int32_t ic1 = v->s[0], ic2 = v->s[1], filt = cut < (127 << 8) || res, last = 0, dcw = 0;
    tsvf_t flt;
    if (!w)
        return;
    pos = clamp((p[P_E1] << 8) + m->shape - (64 << 8), 0, 127 << 8);
    w->pos = (int16_t)pos;
    fp = pos * 7 / 127;                               /* frame << 8 | the share of the next one */
    fr = fp & 255;
    fa = w->w[(fp >> 8) & 7];
    fb = w->w[((fp >> 8) + (fr != 0)) & 7];
    if (warp) {
        int32_t hm = w->hm[(fp >> 8) & 7];
        bk = 0x80000000u - warp * 0xE70000u;          /* the half cycle down to ~5 % of it */
        k1 = 0x80000000u / (bk >> 12);
        k2 = 0x80000000u / ((0u - bk) >> 12);
        /* the halves now last bk and 1 - bk of the cycle: the mean moves to hm (2 bk - 1), hm the first half's mean
         * (the frames' own mean is 0). Taken out, between the two frames as they are read */
        hm += ((w->hm[((fp >> 8) + (fr != 0)) & 7] - hm) * fr) >> 8;
        dcw = (hm * ((int32_t)(bk >> 15) - 65536)) >> 16;
    }
    if (p[P_E6])                                      /* DTN cents (linear: 0.0578 % a cent) */
        inc2 = inc1 + (uint32_t)(((uint64_t)inc1 * (uint32_t)(p[P_E6] * 606)) >> 20);
    if (filt)
        tsvf_coef(&flt, cut, res);
    for (i = 0; i < n; i++) {
        int32_t s = wt_read(fa, fb, fr, warp ? wt_warp(ph0, bk, k1, k2) : ph0) - dcw;
        ph0 += inc1;
        if (inc2) {
            s = (s + wt_read(fa, fb, fr, warp ? wt_warp(ph1, bk, k1, k2) : ph1) - dcw) >> 1;
            ph1 += inc2;
        }
        if (sub) {
            s += mulq15(sine_i(ph2), sub);
            ph2 += inc1 >> 1;
        }
        if (drv)                                      /* (pre-shifts: drive is up to 4x, no overflow) */
            s += mulq15(softclip(((s >> 2) * (drive >> 2)) >> 11) - s, dw);
        if (filt) {                                   /* linear to half scale, then a soft knee (as ANALOG) */
            int32_t y = tsvf_lp(&flt, s >> 1, &ic1, &ic2), a = y < 0 ? -y : y;
            if (a > 16000) {
                a = 16000 + (softclip((a - 16000) * 2) >> 1);
                y = y < 0 ? -a : a;
            }
            s = y << 1;
        } else {
            s = soft_knee(s, 32000);
            last = s;
        }
        out[i] += mulq15(mulq15(s, amp_at(m, i)), VOICE_FS) << 1;
    }
    if (!filt) {                                      /* open and clean: the filter's states follow the signal, so it
                                                       * closes again without a step */
        ic1 = 0;
        ic2 = last >> 1;
    }
    v->ph[0] = ph0;
    v->ph[1] = inc2 ? ph1 : ph0 + 0x2A000000u;
    v->ph[2] = ph2;
    v->s[0] = ic1;
    v->s[1] = ic2;
}

/* (after Clipper 1.2) UNISON (voice.c unison_spread): both oscillators and the sub moved by r (Q32) of a turn */
static void wave_spread(track_t *t, voice_t *v, uint32_t r)
{
    (void)t;
    v->ph[0] += r;
    v->ph[1] += r;
    v->ph[2] += r >> 1;
}

static const preset_t WAVE_PRESETS[] = {
    /* {TABL, POS, WARP, SUB, CUT, RES, DTN, DRV}, {A D S R}, fenv, mono. LFO / ENV > SHP move POS */
    {"WT BASIC", {WT_BASIC, 64, 0, 0, 100, 10, 0, 0}, {2, 70, 100, 40}, 0, 0, FX(0, 20, 16, 24)},
    {"WT SWEEP", {WT_BASIC, 30, 0, 0, 96, 20, 10, 0}, {20, 80, 110, 60}, 0, 0, FX(0, 30, 24, 40),
     XP(P_LD_SHP + 1, 40, P_LRATE + 1, 30)},
    {"PPG PAD", {WT_DIGI, 40, 0, 0, 88, 10, 14, 0}, {70, 90, 115, 85}, 0, 0, FX(0, 45, 22, 60),
     XP(P_LD_SHP + 1, 26, P_LRATE + 1, 24)},
    {"VOX SWEEP", {WT_VOCAL, 20, 0, 0, 110, 8, 8, 0}, {40, 80, 112, 70}, 0, 0, FX(0, 35, 20, 50),
     XP(P_LD_SHP + 1, 50, P_LRATE + 1, 36, P_LWAVE + 1, 1)},
    {"WT CHOIR", {WT_VOCAL, 64, 0, 0, 100, 0, 10, 0}, {60, 90, 118, 80}, 0, 0, FX(0, 50, 20, 60),
     XP(P_LD_SHP + 1, 12, P_LRATE + 1, 20)},
    {"PWM STRNG", {WT_PWM, 20, 0, 0, 84, 6, 12, 0}, {50, 85, 115, 70}, 0, 0, FX(0, 55, 18, 55),
     XP(P_LD_SHP + 1, 30, P_LRATE + 1, 40)},
    {"WT ORGAN", {WT_HARM, 90, 0, 20, 115, 0, 0, 0}, {2, 60, 120, 30}, 0, 0, FX(0, 30, 10, 30),
     XP(P_LD_PIT + 1, 1, P_LRATE + 1, 88)},
    {"WT PLUCK", {WT_BASIC, 20, 20, 0, 90, 20, 0, 0}, {0, 70, 0, 50}, 30, 0, FX(0, 10, 30, 30), XP(P_ED_SHP + 1, 50)},
    {"WT BELLS", {WT_BELL, 30, 0, 0, 120, 0, 0, 0}, {0, 88, 0, 70}, 0, 0, FX(0, 10, 28, 45), XP(P_ED_SHP + 1, 30)},
    {"RESO BASS", {WT_RESO, 10, 0, 30, 110, 0, 0, 20}, {0, 70, 60, 20}, 0, 1, FX(10, 0, 0, 4),
     XP(P_ED_SHP + 1, 50, P_TRANS + 1, -24, P_GLIDE + 1, 30)},
    {"DIGI BASS", {WT_DIGI, 70, 0, 60, 70, 30, 0, 30}, {0, 60, 80, 20}, 30, 1, FX(10, 0, 0, 4), XP(P_TRANS + 1, -24)},
    {"FM GROWL", {WT_FM, 40, 0, 40, 90, 30, 0, 40}, {0, 64, 120, 20}, 0, 1, FX(8, 0, 0, 4),
     XP(P_LD_SHP + 1, 55, P_LSYNC + 1, 6, P_TRANS + 1, -24)},
    {"FOLD LEAD", {WT_FOLD, 50, 0, 0, 105, 15, 6, 10}, {2, 70, 110, 30}, 0, 1, FX(0, 0, 26, 22),
     XP(P_GLIDE + 1, 36, P_LD_PIT + 1, 1, P_LRATE + 1, 82, P_LFADE + 1, 60)},
    {"SUPER WAVE", {WT_BASIC, 70, 10, 0, 100, 10, 10, 0}, {4, 70, 112, 46}, 10, 1, FX(0, 20, 30, 36),
     XP(P_VOICE + 1, 3, P_DETUNE + 1, 64, P_GLIDE + 1, 30)},
    {"CHIP WAVE", {WT_CHIP, 0, 0, 0, 127, 0, 0, 0}, {0, 60, 100, 20}, 0, 1, FX(0, 0, 24, 10),
     XP(P_LD_SHP + 1, 20, P_LRATE + 1, 70)},
};

/* each preset's level trim, 1/2 dB (engines.c preset_trim: P_ED_FX), as tools/level_presets.py makes them for the others:
 * measured against ANALOG's trimmed presets (a note through its ADSR) */
static const int8_t WAVE_TRIM[] = {-3, 4, 17, 12, 8, -2, 1, 23, 22, 9, 4, -10, 5, 5, -12};
_Static_assert(sizeof WAVE_TRIM == sizeof WAVE_PRESETS / sizeof WAVE_PRESETS[0], "a trim per WAVE preset");

static const engine_t ENG_WAVE = {
    "WAVE", {"WAVE", "FILTER"},
    {
        {"TABL", F_ENUM, 0, WT_NTAB - 1, 0, N_WTAB, 0},
        {"POS", F_PCT, 0, 127, 0, 0, 0},
        {"WARP", F_PCT, 0, 127, 0, 0, 0},
        {"SUB", F_PCT, 0, 127, 0, 0, 0},
        {"CUT", F_CUTOFF, 0, 127, 110, 0, 0},
        {"RES", F_PCT, 0, 127, 10, 0, 0},
        {"DTN", F_INT, 0, 127, 0, 0, "ct"},
        {"DRV", F_PCT, 0, 127, 0, 0, 0},
    },
    WAVE_PRESETS, sizeof(WAVE_PRESETS) / sizeof(WAVE_PRESETS[0]), 1, wave_note_on, wave_render,
    0x901F, {P_E1, P_E2, P_E4, P_E5},
    0, 0, 0, wave_block, 0, 0,
};
