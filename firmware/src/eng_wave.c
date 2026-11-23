/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 BONBLOOP contributors, after SLOOP (isod89) and Felucca (Leo Kuroshita, Hügelton Instruments) */
/* WAVE (BONBLOOP 1.1): the synth of the SYNTH MODE (hold ENV). Three oscillators, a filter, a transient shaper; the
 * track's DIST becomes a five-style DRIVE (wave_drive) and its INSERT is a PHASER at hand.
 *   OSC 1..3  WAVE: OFF, SIN, TRI, SAW, SQR (POS: the pulse width), NOISE (POS: its colour), or a table: BASIC
 *             (sine > tri > saw > square > pulse), PWM, HARM, RESO, VOCAL, DIGI, FOLD, FM, BELL, CHIP. POS: where in
 *             the table (frame 1 .. 8, read between two); ENV > SHP and LFO > SHP (POS on the synth pages) move every
 *             oscillator's. LVL, REV. OSC 2 and 3: SEMI (-24 .. 24) and FINE (cents)
 *   REV       the oscillator played backwards, as a sampler's reverse: its cycle read the other way and its note
 *             swelling in, the decay turned round (DEC's time, from -40 dB up to full, then the sustain), released
 *             with the others
 *   WARP      every table read with its phase bent (the first half squeezed: brighter), its DC taken out
 *   FILTER    CUT RES and TYPE (P_FTYPE, as ANALOG's: LP BP HP), key-tracked by half
 *   (TRANSNT, 1.1 .. 1.4: taken out in 1.4.1, its bits in words 4 and 5 free)
 * The tables are built in the part's engine arena (engines.c eng_arena_of, shared with GRAIN / FM6 / PHYS: no RAM
 * more), one for each oscillator that plays one, a few harmonics a block (~23 ms a table), frame by frame over the one
 * playing. 8 frames of 128 samples, the additive ones band-limited to 24 harmonics.
 * The parameters: 25, packed in P_E0 .. P_E7, 16 bits each (WVF): projects, user presets and locks keep the words as
 * they are; the synth pages and HOME edit the fields (params.c page_desc: page ids VP0 + WV_*). The web editor shows
 * the words, it does not edit them. */
#ifndef UI_SMALL
#define UI_SMALL
#endif
#define WT_LEN 128u
#define WT_FR 8u
#define WT_NH 24u                /* harmonics of the additive tables */
#define WT_HPB 6u                /* harmonics added a block while a frame is built */
#define WT_PEAK 30000            /* each frame's peak */
#define WT_NOSC 3u
enum { WT_BASIC, WT_PWM, WT_HARM, WT_RESO, WT_VOCAL, WT_DIGI, WT_FOLD, WT_FM, WT_BELL, WT_CHIP, WT_NTAB };
/* an oscillator's WAVE: off, the analog shapes, then the tables */
enum { WM_OFF, WM_SIN, WM_TRI, WM_SAW, WM_SQR, WM_NOISE, WM_TAB, WM_N = WM_TAB + WT_NTAB };
static const char *const N_WMODE[WM_N] = {"OFF", "SIN", "TRI", "SAW", "SQR", "NOISE", "BASIC", "PWM", "HARM", "RESO",
                                          "VOCAL", "DIGI", "FOLD", "FM", "BELL", "CHIP"};
static const char *const N_WOFFON[] = {"OFF", "ON"};
/* DRIVE's styles, after Decapitator's: A tape, E tube, N transformer, T triode, P pentode */
static const char *const N_DSTY[] = {"A", "E", "N", "T", "P"};

typedef struct {                 /* an oscillator's table */
    uint8_t want;                /* the table in w[] (or being built into it) + 1, 0 = none yet */
    uint8_t fr;                  /* the frame being built, WT_FR = done */
    uint8_t h;                   /* its next harmonic, 0 = not started */
    uint8_t ready;               /* w[] holds table want - 1 entirely */
    uint16_t gen;                /* bumped at each frame stored (the screen redraws) */
    int16_t hm[WT_FR];           /* each frame's mean over its first half (WARP's DC) */
} wt_slot_t;
typedef struct {                 /* a part's WAVE state: its engine arena */
    int16_t w[WT_NOSC][WT_FR][WT_LEN];
    int32_t acc[WT_LEN];         /* the frame being built (one at a time, the slot acc_o's) */
    wt_slot_t sl[WT_NOSC];
    uint8_t acc_o;
    int16_t pos[WT_NOSC];        /* each oscillator's position last rendered, 0..127 << 8 (the screen) */
    int32_t oenv[NVOICE][WT_NOSC];   /* (1.3) each voice's oscillator envelopes, Q24, and their stage (1 attack, 2 decay) */
    uint8_t ost[NVOICE][WT_NOSC];
    uint32_t ek[WT_NOSC][3];     /* (1.3) their ATK step, DEC and REL coefficients this block (voice.c esync_coef) */
    uint32_t olph[WT_NOSC];      /* (1.2) the oscillators' LFOs: the part's, free running; their value, Q15 */
    int32_t olv[WT_NOSC];
} wt_part_t;
_Static_assert(sizeof(wt_part_t) <= sizeof(gr_part_t), "WAVE fits the arena GRAIN already takes (no RAM more)");

static wt_part_t *wt_of(const track_t *t) { return (wt_part_t *)eng_arena_of(t, ENGI_WAVE); }

/* ------------------------------------------------------------------------------------- the parameters --- */
enum { WV_W1, WV_P1, WV_R1, WV_W2, WV_P2, WV_R2, WV_W3, WV_P3, WV_R3, WV_L1, WV_L2, WV_L3, WV_S2, WV_F2, WV_S3, WV_F3,
       WV_CUT, WV_RES, WV_WARP, WV_DSTY, WV_DTONE, WV_PUN, WV_N };
#define VP0 0xA0u                /* page / macro ids VP0 + WV_*: a packed field (params.c page_desc, ui.c home_param) */
_Static_assert(VP0 > P_COUNT && VP0 + WV_N <= 0xFEu, "the packed fields' ids: past the parameters, under 0xFF");
static const uint8_t WVF[WV_N][3] = {   /* word (P_E0 + w), shift, bits */
    {0, 0, 5}, {0, 5, 7}, {0, 12, 1}, {1, 0, 5}, {1, 5, 7}, {1, 12, 1}, {2, 0, 5}, {2, 5, 7}, {2, 12, 1},
    {3, 0, 5}, {3, 5, 5}, {3, 10, 5}, {4, 0, 6}, {4, 6, 5}, {5, 0, 6}, {5, 6, 5},
    {6, 0, 7}, {6, 7, 7}, {7, 0, 7}, {0, 13, 3}, {7, 7, 5}, {1, 13, 1},
};
static const param_desc_t WVD[WV_N] = {
    {"WAVE", F_ENUM, 0, WM_N - 1, WM_TAB, N_WMODE, 0}, {"POS", F_PCT, 0, 127, 64, 0, 0}, {"REV", F_ENUM, 0, 1, 0, N_WOFFON, 0},
    {"WAVE", F_ENUM, 0, WM_N - 1, 0, N_WMODE, 0}, {"POS", F_PCT, 0, 127, 64, 0, 0}, {"REV", F_ENUM, 0, 1, 0, N_WOFFON, 0},
    {"WAVE", F_ENUM, 0, WM_N - 1, 0, N_WMODE, 0}, {"POS", F_PCT, 0, 127, 64, 0, 0}, {"REV", F_ENUM, 0, 1, 0, N_WOFFON, 0},
    {"LVL", F_PCT, 0, 31, 31, 0, 0}, {"LVL", F_PCT, 0, 31, 24, 0, 0}, {"LVL", F_PCT, 0, 31, 24, 0, 0},
    {"SEMI2", F_SEMI, -24, 24, 0, 0, 0}, {"FINE2", F_INT, -16, 15, 0, 0, "ct"},
    {"SEMI3", F_SEMI, -24, 24, 0, 0, 0}, {"FINE3", F_INT, -16, 15, 0, 0, "ct"},
    {"CUT", F_CUTOFF, 0, 127, 110, 0, 0}, {"RES", F_PCT, 0, 127, 10, 0, 0}, {"WARP", F_PCT, 0, 127, 0, 0, 0},
    {"STYLE", F_ENUM, 0, 4, 0, N_DSTY, 0}, {"TONE", F_INT, -16, 15, 0, 0, 0}, {"PUNSH", F_ENUM, 0, 1, 0, N_WOFFON, 0},
};
static int32_t wv_get(const int16_t *p, uint32_t k)
{
    const uint8_t *f = WVF[k];
    return (int32_t)(((uint32_t)(uint16_t)p[P_E0 + f[0]] >> f[1]) & ((1u << f[2]) - 1u)) + WVD[k].min;
}
static void wv_set(int16_t *p, uint32_t k, int32_t v)
{
    const uint8_t *f = WVF[k];
    uint32_t m = ((1u << f[2]) - 1u) << f[1], w = (uint16_t)p[P_E0 + f[0]];
    w = (w & ~m) | (((uint32_t)(clamp(v, WVD[k].min, WVD[k].max) - WVD[k].min) << f[1]) & m);
    p[P_E0 + f[0]] = (int16_t)(uint16_t)w;
}
static int wave_track(const track_t *t) { return t < &trk[NPART] && t >= &trk[0] && t->eng_req % NENGINES == ENGI_WAVE; }
/* BONBLOOP 1.2 / 1.3: what a synth part keeps past its 8 words, 14 bytes (synx[]): OSC 2 and 3's envelopes (ATK DEC
 * SUS REL as SLOOP's ENV, which is OSC 1's: wv_envp), each oscillator's LFO (RATE, DEPTH, DEST: its POS, PITCH or LVL)
 * and the FX chain (7 slots, the synth pages' FX: fx.c runs DRIVE, PHASER and FILTER in its order). Kept in the side
 * records of the projects (project.c bbx_*) and of the user preset banks (upreset.c: w[0..2] there, w[3]'s 16 bits in
 * the record's spare parameter); none: sx_default */
typedef struct { uint32_t w[4]; } synx_t;
static synx_t synx[NPART];
enum { SX_LR1, SX_LD1, SX_LT1, SX_LR2, SX_LD2, SX_LT2, SX_LR3, SX_LD3, SX_LT3,
       SX_A2, SX_D2, SX_S2, SX_R2, SX_A3, SX_D3, SX_S3, SX_R3,
       SX_C1, SX_C2, SX_C3, SX_C4, SX_C5, SX_C6, SX_C7, SX_N };
#define SX_LFO 3u                                     /* (an oscillator's LFO: SX_LR1 + SX_LFO k ..) */
#define SX_NSLOT 7u                                   /* (the chain: one slot per effect) */
enum { FXI_NONE, FXI_DRIVE, FXI_PHASER, FXI_FILTER, FXI_CHORUS, FXI_DELAY, FXI_REVERB, FXI_N };   /* (1.4.1: no TRANS) */
static const char *const N_FXID[FXI_N] = {"--", "DRIVE", "PHASR", "FILTR", "CHORS", "DELAY", "REVRB"};
static const char *const N_LDEST[] = {"POS", "PITCH", "LVL"};
#define SXS 0x80u                                     /* (bits: a signed field) */
static const uint8_t SXF[SX_N][3] = {   /* word, shift, bits | SXS. w[3]: 16 bits only (a user preset's spare parameter) */
    {2, 0, 4}, {2, 4, 4 | SXS}, {0, 28, 2}, {2, 8, 4}, {2, 12, 4 | SXS}, {1, 28, 2}, {2, 16, 4}, {2, 20, 4 | SXS}, {1, 30, 2},
    {0, 0, 7}, {0, 7, 7}, {0, 14, 7}, {0, 21, 7}, {1, 0, 7}, {1, 7, 7}, {1, 14, 7}, {1, 21, 7},
    {2, 24, 3}, {2, 27, 3}, {3, 0, 3}, {3, 3, 3}, {3, 6, 3}, {3, 9, 3}, {3, 12, 3},
};
#define SXD_LFO {"RATE", F_INT, 0, 15, 0, 0, 0}, {"DEPTH", F_INT, -8, 7, 0, 0, 0}, {"DEST", F_ENUM, 0, 2, 0, N_LDEST, 0}
#define SXD_ENV {"ATK", F_INT, 0, 127, 0, 0, 0}, {"DEC", F_INT, 0, 127, 0, 0, 0}, {"SUS", F_INT, 0, 127, 0, 0, 0}, \
                {"REL", F_INT, 0, 127, 0, 0, 0}   /* (shown as SLOOP's ENV: params.c wave_field) */
#define SXD_SLOT(l) {l, F_ENUM, 0, FXI_N - 1, 0, N_FXID, 0}
static const param_desc_t SXD[SX_N] = {SXD_LFO, SXD_LFO, SXD_LFO, SXD_ENV, SXD_ENV, SXD_SLOT("SLOT1"), SXD_SLOT("SLOT2"),
                                       SXD_SLOT("SLOT3"), SXD_SLOT("SLOT4"), SXD_SLOT("SLOT5"), SXD_SLOT("SLOT6"),
                                       SXD_SLOT("SLOT7")};
static int32_t sx_get(uint32_t part, uint32_t k)
{
    const uint8_t *f = SXF[k];
    uint32_t b = f[2] & 15u, v = (synx[part % NPART].w[f[0]] >> f[1]) & ((1u << b) - 1u);
    return f[2] & SXS ? (int32_t)(v ^ (1u << (b - 1u))) - (int32_t)(1u << (b - 1u)) : (int32_t)v;
}
static void sx_set(uint32_t part, uint32_t k, int32_t v)
{
    const uint8_t *f = SXF[k];
    uint32_t b = f[2] & 15u, m = ((1u << b) - 1u);
    v = clamp(v, SXD[k].min, SXD[k].max);
    synx[part % NPART].w[f[0]] = (synx[part % NPART].w[f[0]] & ~(m << f[1])) | (((uint32_t)v & m) << f[1]);
}
/* (1.3) oscillator o's envelope, i: 0 ATK, 1 DEC, 2 SUS, 3 REL. OSC 1's is the track's ENV (P_ATK .. P_REL) */
static int32_t wv_envp(const track_t *t, uint32_t o, uint32_t i)
{
    return o ? sx_get((uint32_t)(t - trk), SX_A2 + 4u * (o - 1u) + i) : t->p[P_ATK + i];
}
/* a part's synx as a sound starts (a preset, or none stored): no LFO, the chain empty, OSC 2 and 3's envelopes as
 * OSC 1's */
static void sx_default(const track_t *t)
{
    uint32_t part = (uint32_t)(t - trk), i;
    if (part >= NPART)
        return;
    memset(&synx[part], 0, sizeof synx[part]);
    for (i = 0; i < 8u; i++)
        sx_set(part, SX_A2 + i, t->p[P_ATK + (i & 3u)]);
}
/* (1.3) to / from a user preset (upreset.c): w[0..2] marked by w[0]'s top bits 10 (1.2 wrote 01, an erased side record
 * reads 11, none 00), w[3] beside; not marked: sx_default */
#define SX_MARK 0x80000000u
static void sx_give(uint32_t part, uint32_t *w)
{
    if (part < NPART)
        w[0] = synx[part].w[0] | SX_MARK, w[1] = synx[part].w[1], w[2] = synx[part].w[2];
}
static void sx_take(const track_t *t, const uint32_t *w, uint32_t w3)
{
    uint32_t part = (uint32_t)(t - trk);
    if (part >= NPART)
        return;
    if (w && (w[0] >> 30) == 2u)
        synx[part].w[0] = w[0] & ~SX_MARK, synx[part].w[1] = w[1], synx[part].w[2] = w[2], synx[part].w[3] = w3 & 0xFFFFu;
    else
        sx_default(t);
}
static uint32_t sx_chain_has(uint32_t part, uint32_t fx)   /* fx in the chain: its slot + 1, 0 = not there */
{
    uint32_t i;
    for (i = 0; i < SX_NSLOT; i++)
        if ((uint32_t)sx_get(part, SX_C1 + i) == fx)
            return i + 1u;
    return 0;
}
#define VX0 0xC0u                                     /* page ids VX0 + SX_*: a field of synx */
_Static_assert(VP0 + WV_N <= VX0 && VX0 + SX_N <= 0xE0u, "the synth's page ids: words, then synx, then the globals");
/* the pages' window on a field (vid: VP0 + WV_* or VX0 + SX_*): its value in wv_tmp (shown, turned), wv_commit writes
 * it back */
static int16_t wv_tmp;
static uint8_t wv_tmp_k;
UI_SMALL static const param_desc_t *wv_desc(const track_t *t, uint32_t vid, int16_t **vp)
{
    wv_tmp_k = (uint8_t)vid;
    *vp = &wv_tmp;
    if (vid >= VX0) {
        wv_tmp = (int16_t)sx_get((uint32_t)(t - trk), vid - VX0);
        if (wv_tmp > SXD[vid - VX0].max)
            wv_tmp = 0;                               /* (1.4.1: a slot of 1.4 on TRANS: --) */
        return &SXD[vid - VX0];
    }
    wv_tmp = (int16_t)wv_get(t->p, vid - VP0);
    return &WVD[vid - VP0];
}
/* (1.2) the synth's FX chain: an effect in (on: at a usable amount if it was off) or out (off) */
static void syn_fx_set(track_t *t, uint32_t fx, int on)
{
    static const uint8_t ID[FXI_N] = {0, P_DIST, P_ITYPE, P_TFLT, P_CHOR, P_DLY, P_REV};
    static const int8_t AMT[FXI_N] = {0, 40, 6, -24, 50, 40, 45}   /* (6: fx.c IT_PHASER) */;
    int16_t *p = t->p;
    if (!fx || fx >= FXI_N)
        return;
    if (fx == FXI_PHASER)                               /* (the INSERT: on as a PHASER at its MIX, off: none) */
        p[P_ITYPE] = 0;
    if (!on)
        p[ID[fx]] = 0;
    else if (!p[ID[fx]])
        p[ID[fx]] = AMT[fx];
}
/* a slot of the chain went from effect `was` to `now` (eng_wave.c wv_commit): `now` in, `was` out unless another slot
 * still holds it. The detail pages follow (params.c page_shown) */
static void syn_chain_changed(track_t *t, uint32_t was, uint32_t now)
{
    if (was == now)
        return;
    if (was && !sx_chain_has((uint32_t)(t - trk), was))
        syn_fx_set(t, was, 0);
    if (now)
        syn_fx_set(t, now, 1);
}
UI_SMALL static int wv_commit(track_t *t, const int16_t *vp)
{
    uint32_t part = (uint32_t)(t - trk), k = wv_tmp_k;
    if (vp != &wv_tmp)
        return 0;
    if (k >= VX0) {
        uint32_t was = (uint32_t)sx_get(part, k - VX0);
        sx_set(part, k - VX0, wv_tmp);
        if (k >= VX0 + SX_C1)                         /* a slot of the chain: what it adds or takes out */
            syn_chain_changed(t, was, (uint32_t)wv_tmp);
    } else {
        wv_set(t->p, k - VP0, wv_tmp);
    }
    return 1;
}
/* SLOOP's EDIT pages on a WAVE track: EDIT 1 OSC 1 and 2, EDIT 2 the filter and OSC 3 */
static const uint8_t WAVE_EDIT[8] = {WV_W1, WV_P1, WV_W2, WV_P2, WV_CUT, WV_RES, WV_W3, WV_P3};

/* the words of a sound (the presets below: preset_t.e holds bytes) */
#define WW_OSC(w, p, r, x) ((w) | (p) << 5 | (r) << 12 | (x) << 13)   /* WAVE POS REV, + STYLE (OSC 1) / PUNSH (OSC 2) */
#define WW_LVL(a, b, c) ((a) | (b) << 5 | (c) << 10)
#define WW_TUNE(s, f, t) (((s) + 24) | ((f) + 16) << 6)   /* SEMI FINE (t: the TRANSNT of 1.1 .. 1.4, unused) */
#define WW_FLT(c, r) ((c) | (r) << 7)
#define WW_MISC(wp, tone) ((wp) | ((tone) + 16) << 7)
#define WWS(x) ((int16_t)(uint16_t)(x))
#define WT_T(n) (WM_TAB + (n))

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
        uint32_t th = i * (1024u / WT_LEN), j = i * (256u / WT_LEN);   /* (j: as 256 samples a cycle) */
        if (tab == WT_FOLD) {                         /* sin(pi g sin x), g 0.5 .. 6.8: folded over and over */
            x = SINE[((SINE[th] * (int32_t)(256u + f * 460u)) >> 15) & 1023];
        } else if (tab == WT_FM) {                    /* sin(x + I sin 3x), I 0 .. 5.6 */
            x = SINE[(th + (uint32_t)((SINE[(th * 3u) & 1023u] * (int32_t)(f * 130u)) >> 15)) & 1023u];
        } else {                                      /* CHIP: 4-bit */
            switch (f) {
            case 0:
            case 1:
            case 2:                                   /* pulses 50, 25, 12.5 % */
                q = j < (128u >> f) ? 15 : 0;
                break;
            case 3:                                   /* the NES triangle */
                q = (int32_t)((j < 128u ? j : 255u - j) >> 3);
                break;
            case 4:                                   /* a stepped saw */
                q = (int32_t)(j >> 4);
                break;
            case 5:                                   /* a 3-bit sine */
                q = ((SINE[th] + 32768) >> 13) * 15 / 7;
                break;
            case 6:                                   /* a square and its octave */
                q = (j & 64u ? 5 : 0) + (j & 128u ? 10 : 0);
                break;
            default: {                                /* a looped 32-step noise */
                uint32_t r = ((j >> 3) + 1u) * 2654435761u;
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

/* the part's hook, once a block: each oscillator's table as its WAVE asks, the first one with work first, WT_HPB
 * harmonics (or a whole frame in time) a block */
static void wave_block(track_t *t)
{
    wt_part_t *w = wt_of(t);
    uint32_t o, n, part = (uint32_t)(t - trk);
    if (!w)
        return;
    for (o = 0; o < WT_NOSC; o++) {                   /* (1.2) each oscillator's LFO, a block on */
        if (sx_get(part, SX_LD1 + SX_LFO * o)) {
            w->olph[o] += LFO_INC[(uint32_t)sx_get(part, SX_LR1 + SX_LFO * o) * 8u + 7u];
            w->olv[o] = sine_i(w->olph[o]);
        }
    }
    for (o = 0; o < WT_NOSC; o++) {
        wt_slot_t *s = &w->sl[o];
        int32_t tab = wv_get(t->p, WV_W1 + 3u * o) - WM_TAB;
        if (tab < 0)
            continue;                                 /* (OFF or an analog shape: no table) */
        if (s->want != (uint32_t)tab + 1u) {          /* another table: frame by frame over the one playing */
            s->want = (uint8_t)(tab + 1);
            s->fr = 0;
            s->h = 0;
            s->ready = 0;
        }
        if (s->fr >= WT_FR)
            continue;
        if (wt_additive((uint32_t)tab)) {
            if (w->acc_o != o)                        /* (the frame being built was another slot's: this one's again) */
                s->h = 0;
            if (!s->h) {
                for (n = 0; n < WT_LEN; n++)
                    w->acc[n] = 0;
                s->h = 1;
                w->acc_o = (uint8_t)o;
            }
            for (n = 0; n < WT_HPB && s->h <= WT_NH; n++, s->h++) {
                uint32_t ph;
                int32_t a = wt_harm((uint32_t)tab, s->fr, s->h, &ph);
                if (a)
                    wt_add(w->acc, a, s->h, ph);
            }
            if (s->h <= WT_NH)
                return;
        } else {
            wt_direct(w->acc, (uint32_t)tab, s->fr);
        }
        wt_store(w->w[o][s->fr], w->acc, &s->hm[s->fr]);
        s->h = 0;
        w->acc_o = 0xFF;
        if (++s->fr >= WT_FR)
            s->ready = 1;
        s->gen++;
        return;
    }
}

/* ------------------------------------------------------------------------------------------ the voice --- */
/* voice state: ph[0..2] the oscillators, s[0] s[1] the filter, s[2] NOISE's generator, s[3] its colour filter; the
 * oscillators' envelopes: the arena's oenv / ost (1.3) */
#define WR_START 328             /* the REV swell's start: -40 dB */
static void wave_note_on(track_t *t, voice_t *v)
{
    if (!v->env && !v->env_out) {                     /* a fresh note: from phase 0, the filter empty */
        v->ph[0] = 0;
        v->ph[1] = 0x2A000000u;                       /* (the oscillators not in phase) */
        v->ph[2] = 0x55000000u;
        v->s[0] = v->s[1] = v->s[3] = 0;
    }
    if (!v->s[2])
        v->s[2] = 0x2468ACE + (int32_t)v->age;
    {   /* (1.3) the oscillators' envelopes: their attack (from 0 on a fresh note, from where they are retriggered;
         * REV: the swell from -40 dB) */
        wt_part_t *w = wt_of(t);
        uint32_t o, vi = (uint32_t)(v - t->v) % NVOICE;
        for (o = 0; w && o < WT_NOSC; o++) {
            if (wv_get(t->p, WV_R1 + 3u * o))
                w->oenv[vi][o] = WR_START << 9;
            else if (!v->env)
                w->oenv[vi][o] = 0;
            w->ost[vi][o] = 1;
        }
    }
}

/* the voice's amplitude (voice.c: once a block; ENG_WAVE.ownenv: env is 1.0, or the fade of a voice given up): the
 * velocity. (1.3) Each oscillator's envelope a step on, as voice.c env_tick steps
 * SLOOP's ENV (its coefficients: w->ek, esync_coef, ENV SYNC too); REV: its decay turned round, a swell from -40 dB
 * as fast as DEC falls, then SUS */
static int32_t wave_amp(track_t *t, voice_t *v, int32_t env)
{
    const int16_t *p = t->p;
    wt_part_t *w = wt_of(t);
    uint32_t o, vi = (uint32_t)(v - t->v) % NVOICE;
    for (o = 0; w && o < WT_NOSC; o++) {
        int32_t *e = &w->oenv[vi][o], sus = wv_envp(t, o, 2u) << 17, rev = wv_get(p, WV_R1 + 3u * o);
        const uint32_t *ek = w->ek[o];
        if (v->stage >= 3u) {
            *e -= mulq16(*e, ek[2]);
        } else if (w->ost[vi][o] == 1u) {
            *e += rev ? mulq16(*e, ek[1]) + (1 << 9) : (int32_t)ek[0];
            if (*e >= 1 << 24) {
                *e = rev ? sus : 1 << 24;
                w->ost[vi][o] = 2;
            }
        } else if (!rev) {
            *e += mulq16(sus - *e, ek[1]);
        }
    }
    return mulq15(env, v->vel * 258);
}
/* (1.3) released, the voice ends when every oscillator that sounds has faded (voice.c: ENG_WAVE.ownenv) */
static int wave_done(track_t *t, voice_t *v)
{
    wt_part_t *w = wt_of(t);
    uint32_t o, vi = (uint32_t)(v - t->v) % NVOICE;
    if (v->stage < 3u)
        return !w;
    for (o = 0; w && o < WT_NOSC; o++)
        if (wv_get(t->p, WV_W1 + 3u * o) != WM_OFF && wv_get(t->p, WV_L1 + o) && w->oenv[vi][o] >= 1 << 12)
            return 0;
    return 1;
}

static inline int32_t wt_lerp(const int16_t *f, uint32_t ph)   /* a frame at phase ph, between two samples */
{
    uint32_t i = ph >> 25;
    int32_t a = f[i], b = f[(i + 1u) & (WT_LEN - 1u)];
    return a + (((b - a) * (int32_t)((ph >> 10) & 0x7FFFu)) >> 15);
}
/* WARP: the first half of the cycle squeezed into [0, bk), the second half stretched over the rest (k1, k2: slopes,
 * Q12; both halves stay below 2^32) */
static inline uint32_t wt_warp(uint32_t ph, uint32_t bk, uint32_t k1, uint32_t k2)
{
    return ph < bk ? (ph >> 12) * k1 : 0x80000000u + ((ph - bk) >> 12) * k2;
}

/* oscillator k's increment: OSC 1 the note's; OSC 2 and 3 SEMI semitones and FINE cents from it (the unison detune,
 * TUNE and the glide's fraction as the note's: m->fine) */
static uint32_t wt_inc(const int16_t *p, const vmod_t *m, uint32_t k, int32_t off)   /* off: its LFO, 1/16 semitone */
{
    uint32_t inc;
    if (!k && !off)
        return m->inc;
    inc = pitch_inc((uint32_t)clamp(m->pitch16 + (k ? 16 * wv_get(p, WV_S2 + 2u * (k - 1u)) : 0) + off, 0, 2047));
    return inc + (uint32_t)((int32_t)(inc >> 12) * (m->fine + (k ? (wv_get(p, WV_F2 + 2u * (k - 1u)) * 2424) >> 10 : 0)));
}

static void wave_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    wt_part_t *w = wt_of(t);
    uint32_t i, k, warp = (uint32_t)wv_get(p, WV_WARP), bk = 0x80000000u, k1 = 4096u, k2 = 4096u;
    uint32_t ft = (uint32_t)clamp(p[P_FTYPE], 0, 2), part = (uint32_t)(t - trk), vi = (uint32_t)(v - t->v) % NVOICE;
    int32_t me = w ? w->oenv[vi][0] >> 9 : 0;         /* (1.3) OSC 1's envelope moves the filter and POS (ENV DEST) */
    int32_t buf[CTL], shift = m->shape - (64 << 8) + ((me * p[P_ED_SHP]) >> 7), last = 0;
    int32_t cut = (wv_get(p, WV_CUT) << 8) + m->cutoff + ((v->pitch16 - 60 * 16) << 2) + ((me * p[P_ED_FLT]) >> 7) +
                  (v->vel > 110 ? (me * 24) >> 7 : 0), res = wv_get(p, WV_RES);
    int32_t ic1 = v->s[0], ic2 = v->s[1], filt = ft || cut < (127 << 8) || res;
    tsvf_t flt;
    if (!w)
        return;
    if (warp) {
        bk = 0x80000000u - warp * 0xE70000u;          /* the half cycle down to ~5 % of it */
        k1 = 0x80000000u / (bk >> 12);
        k2 = 0x80000000u / ((0u - bk) >> 12);
    }
    for (i = 0; i < n; i++)
        buf[i] = 0;
    for (k = 0; k < WT_NOSC; k++) {
        uint32_t mode = (uint32_t)wv_get(p, WV_W1 + 3u * k), lv = (uint32_t)wv_get(p, WV_L1 + k);
        uint32_t rev = (uint32_t)wv_get(p, WV_R1 + 3u * k), inc, ph = v->ph[k], sx = SX_LFO * k;
        int32_t ld = sx_get(part, SX_LD1 + sx), lt = sx_get(part, SX_LT1 + sx), lfo = ld ? w->olv[k] : 0;   /* (1.2) its LFO */
        int32_t pos = clamp((wv_get(p, WV_P1 + 3u * k) << 8) + shift + (lt == 0 ? (lfo * ld * 6) >> 7 : 0), 0, 127 << 8), g, x;
        if (mode == WM_OFF || !lv)
            continue;
        w->pos[k] = (int16_t)pos;
        g = mulq15((int32_t)(lv * lv * 22u), w->oenv[vi][k] >> 9);   /* (1.3) at its own envelope */
        if (lt == 2 && ld)                            /* (1.2) LFO > LVL: a tremolo, DEPTH 8 down to silence */
            g = mulq15(g, 32767 - ((((32767 - lfo) >> 1) * (ld < 0 ? -ld : ld)) >> 3));
        inc = wt_inc(p, m, k, lt == 1 ? (lfo * ld) >> 12 : 0);   /* (LFO > PITCH: DEPTH 7, about 3.5 semitones) */
        if (mode >= WM_TAB) {                         /* a table: between two frames, WARP, read backwards on REV */
            int32_t fp = pos * 7 / 127, fr = fp & 255, dcw = 0;
            const int16_t *fa = w->w[k][(fp >> 8) & 7], *fb = w->w[k][((fp >> 8) + (fr != 0)) & 7];
            if (warp) {                               /* the halves now last bk and 1 - bk: the mean moves to hm (2 bk - 1) */
                int32_t hm = w->sl[k].hm[(fp >> 8) & 7];
                hm += ((w->sl[k].hm[((fp >> 8) + (fr != 0)) & 7] - hm) * fr) >> 8;
                dcw = (hm * ((int32_t)(bk >> 15) - 65536)) >> 16;
            }
            for (i = 0; i < n; i++, ph += inc) {
                uint32_t q = rev ? 0u - ph : ph;
                if (warp)
                    q = wt_warp(q, bk, k1, k2);
                x = wt_lerp(fa, q);
                if (fr)
                    x += ((wt_lerp(fb, q) - x) * fr) >> 8;
                buf[i] += mulq15(x - dcw, g);
            }
        } else if (mode == WM_NOISE) {                /* POS: from white (0) to dark */
            int32_t lp = v->s[3], c = 32767 - (pos >> 8) * 250;
            for (i = 0; i < n; i++) {
                lp += mulq15(((int32_t)(noise32(&v->s[2]) >> 17) - 16384) * 2 - lp, c);
                buf[i] += mulq15(lp, g);
            }
            v->s[3] = lp;
            ph += inc * n;
        } else {                                      /* SIN TRI SAW SQR (POS: its width, 50 .. 6 %); REV: backwards */
            uint32_t pw = 0x80000000u - (uint32_t)pos * 57950u;
            for (i = 0; i < n; i++, ph += inc) {
                switch (mode) {
                case WM_SIN:
                    x = sine_i(rev ? 0u - ph : ph);
                    break;
                case WM_TRI:
                    x = osc_tri(rev ? 0u - ph : ph);
                    break;
                case WM_SAW:
                    x = osc_saw(ph, inc) * 29 >> 5;   /* (a saw's 2.9 dB more than a table's peak: as loud) */
                    break;
                default:
                    x = osc_pulse(ph, inc, pw);
                    break;
                }
                buf[i] += mulq15(mode >= WM_SAW && rev ? -x : x, g);   /* (a saw or pulse backwards: upside down) */
            }
        }
        v->ph[k] = ph;
    }
    if (filt) {
        if (cut > (127 << 8))
            cut = 127 << 8;
        tsvf_coef(&flt, cut, res);
    }
    for (i = 0; i < n; i++) {
        int32_t s = buf[i];
        if (filt) {                                   /* linear to half scale, then a soft knee (as ANALOG) */
            int32_t y = ft ? analog_tap(&flt, ANALOG_K(res), s >> 1, &ic1, &ic2, ft) : tsvf_lp(&flt, s >> 1, &ic1, &ic2);
            int32_t a = y < 0 ? -y : y;
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
    v->s[0] = ic1;
    v->s[1] = ic2;
}

/* (after Clipper 1.2) UNISON (voice.c unison_spread): the three oscillators moved by r (Q32) of a turn */
static void wave_spread(track_t *t, voice_t *v, uint32_t r)
{
    (void)t;
    v->ph[0] += r;
    v->ph[1] += r;
    v->ph[2] += r;
}

/* DRIVE (BONBLOOP, after Soundtoys' Decapitator): the track's DIST on a WAVE track (fx.c track_dist). Low cut, the
 * drive (DIST, x8 with PUNSH), a style's saturation, TONE (the low-pass after it, dark .. open), make-up. A tape
 * (soft, even), E tube (soft, lopsided: even harmonics), N transformer (more drive), T triode (fuzzier, lopsided),
 * P pentode (half hard clip). The state: the track's dist_* (as DIST) */
static const int16_t WD_BIAS[5] = {0, 4000, 1200, 7000, 600};
static const uint8_t WD_GAIN[5] = {2, 2, 3, 4, 3};        /* the drive x this / 2 */
static void wave_drive(track_t *t, int32_t *b, uint32_t n, int32_t d)
{
    const int16_t *p = t->p;
    uint32_t st = (uint32_t)wv_get(p, WV_DSTY) % 5u, i;
    /* (1.2: heavier) 1x .. ~25x (A) / ~50x (T); PUNSH x8 more (+18 dB, Decapitator's PUNISH), the pentode harder. g: Q6 */
    int32_t g = ((4096 + d * d * 6) * WD_GAIN[st]) >> 7, bias = WD_BIAS[st], b0 = softclip(bias);
    int32_t k = clamp(26000 + wv_get(p, WV_DTONE) * 420 - d * 40, 3000, 32767), mk = 30000 - d * 100;
    if (wv_get(p, WV_PUN)) {
        g <<= 3;
        mk -= mk >> 2;
    }
    if (st == 4u)
        b0 = (b0 + bias) >> 1;
    for (i = 0; i < n; i++) {
        int32_t x = b[i], y;
        t->dist_hp += (x - t->dist_hp + 64) >> 7;       /* ~55 Hz low cut */
        x = (clamp(x - t->dist_hp, -65535, 65535) * g) >> 6;
        y = softclip(x + bias);
        if (st == 4u)
            y = (y + clamp(x + bias, -16000, 16000)) >> 1;
        y -= b0;
        t->dist_lp1 += mulq15(y - t->dist_lp1, k);      /* TONE: two poles */
        t->dist_lp2 += mulq15(t->dist_lp1 - t->dist_lp2, k);
        b[i] = mulq15(t->dist_lp2, mk);
    }
}

/* the sounds: preset_t.e holds bytes, so each one's words are here (params.c preset_extras: wave_preset_words) */
static const preset_t WAVE_PRESETS[] = {
    /* LFO / ENV > SHP move every oscillator's POS. (1.2) Every effect off: the synth pages' FX chain adds them */
    {"WT BASIC", {0}, {2, 70, 100, 40}, 0, 0, FX(0, 0, 0, 0)},
    {"WT SWEEP", {0}, {20, 80, 110, 60}, 0, 0, FX(0, 0, 0, 0), XP(P_LD_SHP + 1, 40, P_LRATE + 1, 30)},
    {"PPG PAD", {0}, {70, 90, 115, 85}, 0, 0, FX(0, 0, 0, 0), XP(P_LD_SHP + 1, 26, P_LRATE + 1, 24)},
    {"VOX SWEEP", {0}, {40, 80, 112, 70}, 0, 0, FX(0, 0, 0, 0), XP(P_LD_SHP + 1, 50, P_LRATE + 1, 36, P_LWAVE + 1, 1)},
    {"WT CHOIR", {0}, {60, 90, 118, 80}, 0, 0, FX(0, 0, 0, 0), XP(P_LD_SHP + 1, 12, P_LRATE + 1, 20)},
    {"PWM STRNG", {0}, {50, 85, 115, 70}, 0, 0, FX(0, 0, 0, 0), XP(P_LD_SHP + 1, 30, P_LRATE + 1, 40)},
    {"WT ORGAN", {0}, {2, 60, 120, 30}, 0, 0, FX(0, 0, 0, 0), XP(P_LD_PIT + 1, 1, P_LRATE + 1, 88)},
    {"WT PLUCK", {0}, {0, 84, 0, 50}, 30, 0, FX(0, 0, 0, 0), XP(P_ED_SHP + 1, 50)},   /* (1.4.1: DEC 70 -> 84, OSC 2
                                                       * louder: its punch was the TRANSNT's) */
    {"WT BELLS", {0}, {0, 88, 0, 70}, 0, 0, FX(0, 0, 0, 0), XP(P_ED_SHP + 1, 30)},
    {"RESO BASS", {0}, {0, 70, 60, 20}, 0, 1, FX(0, 0, 0, 0), XP(P_ED_SHP + 1, 50, P_TRANS + 1, -24, P_GLIDE + 1, 30)},
    {"DIGI BASS", {0}, {0, 60, 80, 20}, 30, 1, FX(0, 0, 0, 0), XP(P_TRANS + 1, -24)},
    {"FM GROWL", {0}, {0, 64, 120, 20}, 0, 1, FX(0, 0, 0, 0), XP(P_LD_SHP + 1, 55, P_LSYNC + 1, 6, P_TRANS + 1, -24)},
    {"FOLD LEAD", {0}, {2, 70, 110, 30}, 0, 1, FX(0, 0, 0, 0),
     XP(P_GLIDE + 1, 36, P_LD_PIT + 1, 1, P_LRATE + 1, 82, P_LFADE + 1, 60)},
    {"SUPER WAVE", {0}, {4, 70, 112, 46}, 10, 1, FX(0, 0, 0, 0), XP(P_VOICE + 1, 3, P_DETUNE + 1, 64, P_GLIDE + 1, 30)},
    {"CHIP WAVE", {0}, {0, 60, 100, 20}, 0, 1, FX(0, 0, 0, 0), XP(P_LD_SHP + 1, 20, P_LRATE + 1, 70)},
    /* BONBLOOP 1.1: OSC 1 backwards (it swells in over DEC and stops), OSC 2 forwards an octave up: a reversed pluck */
    {"REV SWELL", {0}, {0, 100, 0, 60}, 0, 0, FX(0, 0, 0, 0), XP(P_ED_SHP + 1, 20)},
    /* BONBLOOP 1.4: the SYNTH MODE's start (ui_input.c syn_sine): OSC 1 a sine, OSC 2 and 3 off, the filter open, no
     * effect, no LFO */
    {"SINE", {0}, {2, 60, 127, 30}, 0, 0, FX(0, 0, 0, 0)},
};
#define WAVE_SINE (NELEM(WAVE_PRESETS) - 1u)
static const uint16_t WAVE_WORDS[][8] = {
    /* OSC 1, OSC 2, OSC 3, LVL 1 2 3, TUNE 2 (+ ATTCK), TUNE 3 (+ SUSTN), CUT RES, WARP TONE */
    {WW_OSC(WT_T(WT_BASIC), 64, 0, 0), WW_OSC(0, 64, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 24, 24),
     WW_TUNE(0, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(100, 10), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_BASIC), 30, 0, 0), WW_OSC(WT_T(WT_BASIC), 90, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 18, 24),
     WW_TUNE(12, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(96, 20), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_DIGI), 40, 0, 0), WW_OSC(WT_T(WT_DIGI), 80, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 26, 24),
     WW_TUNE(0, 7, 0), WW_TUNE(0, 0, 0), WW_FLT(88, 10), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_VOCAL), 20, 0, 0), WW_OSC(WM_SAW, 64, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 8, 24),
     WW_TUNE(-12, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(110, 8), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_VOCAL), 64, 0, 0), WW_OSC(WT_T(WT_VOCAL), 64, 0, 0), WW_OSC(WT_T(WT_VOCAL), 64, 0, 0),
     WW_LVL(31, 24, 24), WW_TUNE(0, 8, 0), WW_TUNE(0, -8, 0), WW_FLT(100, 0), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_PWM), 20, 0, 0), WW_OSC(WT_T(WT_PWM), 60, 0, 0), WW_OSC(WM_SAW, 64, 0, 0), WW_LVL(31, 26, 12),
     WW_TUNE(0, 10, 0), WW_TUNE(-12, 0, 0), WW_FLT(84, 6), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_HARM), 90, 0, 0), WW_OSC(WM_SIN, 64, 0, 0), WW_OSC(WT_T(WT_HARM), 40, 0, 0), WW_LVL(31, 20, 14),
     WW_TUNE(-12, 0, 0), WW_TUNE(12, 0, 0), WW_FLT(115, 0), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_BASIC), 20, 0, 0), WW_OSC(WM_SQR, 30, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 20, 24),
     WW_TUNE(12, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(90, 20), WW_MISC(20, 0)},
    {WW_OSC(WT_T(WT_BELL), 30, 0, 0), WW_OSC(WT_T(WT_BELL), 70, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 16, 24),
     WW_TUNE(12, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(120, 0), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_RESO), 10, 0, 0), WW_OSC(WM_SIN, 64, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 24, 24),
     WW_TUNE(-12, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(110, 0), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_DIGI), 70, 0, 0), WW_OSC(WM_SQR, 0, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 20, 24),
     WW_TUNE(-12, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(70, 30), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_FM), 40, 0, 3), WW_OSC(WM_SAW, 64, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 18, 24),
     WW_TUNE(-12, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(90, 30), WW_MISC(0, -4)},
    {WW_OSC(WT_T(WT_FOLD), 50, 0, 0), WW_OSC(WM_SAW, 64, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 18, 24),
     WW_TUNE(0, 6, 0), WW_TUNE(0, 0, 0), WW_FLT(105, 15), WW_MISC(0, 0)},
    {WW_OSC(WM_SAW, 64, 0, 0), WW_OSC(WM_SAW, 64, 0, 0), WW_OSC(WM_SAW, 64, 0, 0), WW_LVL(31, 28, 28),
     WW_TUNE(0, 12, 0), WW_TUNE(0, -12, 0), WW_FLT(100, 10), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_CHIP), 0, 0, 0), WW_OSC(WT_T(WT_CHIP), 30, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 14, 24),
     WW_TUNE(12, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(127, 0), WW_MISC(0, 0)},
    {WW_OSC(WT_T(WT_BASIC), 50, 1, 0), WW_OSC(WM_TRI, 64, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 14, 24),
     WW_TUNE(12, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(105, 10), WW_MISC(0, 0)},
    {WW_OSC(WM_SIN, 64, 0, 0), WW_OSC(0, 64, 0, 0), WW_OSC(0, 64, 0, 0), WW_LVL(31, 24, 24),
     WW_TUNE(0, 0, 0), WW_TUNE(0, 0, 0), WW_FLT(127, 0), WW_MISC(0, 0)},   /* SINE */
};
_Static_assert(sizeof WAVE_WORDS / sizeof WAVE_WORDS[0] == sizeof WAVE_PRESETS / sizeof WAVE_PRESETS[0],
               "a set of words per WAVE preset");
static void wave_preset_words(int16_t *p, uint32_t k)
{
    uint32_t i, part = (uint32_t)(((uintptr_t)p - (uintptr_t)trk[0].p) / sizeof(track_t));
    for (i = 0; i < 8u; i++)
        p[P_E0 + i] = WWS(WAVE_WORDS[k][i]);
    p[P_ITYPE] = 0;                                   /* (1.2) no INSERT: the FX chain's PHASER brings it (syn_fx_set) */
    if (part < NPART && p == trk[part].p)             /* (1.3) a sound's synx: no LFO of the oscillators, the chain
                                                       * empty, OSC 2 and 3's envelopes as the preset's (OSC 1's) */
        sx_default(&trk[part]);
}

/* each preset's level trim, 1/2 dB (engines.c preset_trim: P_ED_FX), as tools/level_presets.py makes them for the others:
 * measured against ANALOG's trimmed presets (a note through its ADSR; tests/wave_test.c prints them) */
static const int8_t WAVE_TRIM[] = {3, 0, 13, 12, 2, -3, 3, 23, 23, 10, 15, -3, 7, -5, -6, 21, -5};
_Static_assert(sizeof WAVE_TRIM == sizeof WAVE_PRESETS / sizeof WAVE_PRESETS[0], "a trim per WAVE preset");

static const engine_t ENG_WAVE = {
    "WAVE", {"OSC 1+2", "FLT+OSC3"},
    {   /* the packed words (WVF): the synth pages edit their fields */
        {"OSC1", F_INT, -32768, 32767, WWS(WW_OSC(WT_T(WT_BASIC), 64, 0, 0)), 0, 0},
        {"OSC2", F_INT, -32768, 32767, WWS(WW_OSC(0, 64, 0, 0)), 0, 0},
        {"OSC3", F_INT, -32768, 32767, WWS(WW_OSC(0, 64, 0, 0)), 0, 0},
        {"LVLS", F_INT, -32768, 32767, WWS(WW_LVL(31, 24, 24)), 0, 0},
        {"TUN2", F_INT, -32768, 32767, WWS(WW_TUNE(0, 0, 0)), 0, 0},
        {"TUN3", F_INT, -32768, 32767, WWS(WW_TUNE(0, 0, 0)), 0, 0},
        {"FILT", F_INT, -32768, 32767, WWS(WW_FLT(110, 10)), 0, 0},
        {"MISC", F_INT, -32768, 32767, WWS(WW_MISC(0, 0)), 0, 0},
    },
    WAVE_PRESETS, sizeof(WAVE_PRESETS) / sizeof(WAVE_PRESETS[0]), 1, wave_note_on, wave_render,
    0x901F, {VP0 + WV_P1, VP0 + WV_P2, VP0 + WV_CUT, VP0 + WV_RES},
    0, wave_amp, 0, wave_block, 1, wave_done,
};
