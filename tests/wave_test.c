/* SPDX-License-Identifier: GPL-3.0-only */
/* BONBLOOP 1.1: the WAVE engine (firmware/src/eng_wave.c) alone, no hostsim (builds with any C compiler, MSVC too).
 *   fields    the 24 packed fields: each one set and read back alone, the others unchanged, clamped to its range
 *   tables    every table built in each oscillator's slot (frame by frame), each frame at the same peak, no DC; a
 *             table changed while another oscillator's is being built: both end right
 *   presets   every WAVE preset through its ADSR at notes 48 and 72: bounded, audible, no DC
 *   sweep     every table and analog shape swept by POS (the SHP modulation), 2 s each
 *   REV       a backwards oscillator swells in (quiet first, loud at the end of DEC), stops at its peak
 *   TRANSNT   ATTCK +15 lifts the first 20 ms, -16 fades them in; SUSTN -16 takes the tail down
 *   DRIVE     every style, PUNSH, TONE: bounded, no DC left, louder with more drive
 *   extremes  every field at its ends, three oscillators on, notes 0 .. 127: under ANALOG's own ceiling (~95000)
 *   level     each preset against ANALOG's trimmed presets: prints WAVE_TRIM; with it, within 3 dB of ANALOG's median
 *   cost      ns per sample, three table oscillators against ANALOG REESE (host, rough): at most 2.6x
 *   wave_test [DIR]   (DIR/wave_presets.wav, DIR/wave_sweep.wav) */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define __attribute__(x)
#include "felucca_tables.h"
#include "core.h"
#include "dsp.c"
/* engines.c pieces the engines use */
static inline int32_t soft_knee(int32_t y, int32_t k)
{
    int32_t a = y < 0 ? -y : y;
    if (a <= k)
        return y;
    a = k + (softclip((a - k) * 2) >> 1);
    return y < 0 ? -a : a;
}
static uint32_t uni_turns(uint32_t r, int32_t p16, int32_t d16)
{
    uint32_t b = pitch_inc((uint32_t)clamp(p16, 0, 2047));
    return b ? (uint32_t)(((uint64_t)r * pitch_inc((uint32_t)clamp(p16 + d16, 0, 2047))) / b) : r;
}
typedef struct { uint8_t x[7000]; } gr_part_t;      /* (GRAIN's size, about: the static assert) */
static uint8_t arena[NPART][8192];
static uint8_t arena_own[NPART];
static void *eng_arena_of(const track_t *t, uint32_t eng)
{
    uint32_t p = (uint32_t)(t - trk);
    if (p >= NPART)
        return 0;
    if (arena_own[p] != eng + 1u) {
        memset(arena[p], 0, sizeof arena[p]);
        arena_own[p] = (uint8_t)(eng + 1u);
    }
    return arena[p];
}
#include "eng_analog.c"
#include "preset_trim.h"
#include "eng_wave.c"
static int16_t preset_trim(uint32_t e, uint32_t pi) { return e == ENGI_WAVE && pi < NELEM(WAVE_TRIM) ? WAVE_TRIM[pi] : 0; }

static int fails;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL: " __VA_ARGS__); printf("\n"); fails++; } } while (0)

static FILE *wav_open(const char *path)
{
    FILE *f = fopen(path, "wb");
    uint8_t h[44] = {0};
    if (f)
        fwrite(h, 1, 44, f);
    return f;
}
static void wav_close(FILE *f)
{
    long n;
    uint32_t v;
    uint16_t s;
    if (!f)
        return;
    n = ftell(f) - 44;
    fseek(f, 0, SEEK_SET);
    fwrite("RIFF", 1, 4, f); v = (uint32_t)(36 + n); fwrite(&v, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f); v = 16; fwrite(&v, 4, 1, f);
    s = 1; fwrite(&s, 2, 1, f); fwrite(&s, 2, 1, f);
    v = FS; fwrite(&v, 4, 1, f); v = FS * 2; fwrite(&v, 4, 1, f);
    s = 2; fwrite(&s, 2, 1, f); s = 16; fwrite(&s, 2, 1, f);
    fwrite("data", 1, 4, f); v = (uint32_t)n; fwrite(&v, 4, 1, f);
    fclose(f);
}

static int built(track_t *t)                         /* every table oscillator's slot holds its table */
{
    wt_part_t *w = wt_of(t);
    uint32_t o;
    for (o = 0; o < WT_NOSC; o++) {
        int32_t tab = wv_get(t->p, WV_W1 + 3u * o) - WM_TAB;
        if (tab >= 0 && !(w->sl[o].ready && w->sl[o].want == (uint32_t)tab + 1u))
            return 0;
    }
    return 1;
}
static int build(track_t *t)                         /* blocks it took */
{
    int b;
    for (b = 0; b < 600 && !built(t); b++)
        wave_block(t);
    return b;
}

static int xp_of(const preset_t *pr, uint32_t id, int def)
{
    uint32_t j;
    for (j = 0; j < 8; j += 2)
        if (pr->x[j] == (int8_t)(id + 1))
            return pr->x[j + 1];
    return def;
}
static void load(track_t *t, const engine_t *e, uint32_t pi)
{
    uint32_t i;
    memset(t->p, 0, sizeof t->p);
    t->p[P_SUS] = 127;
    t->p[P_DEC] = 60;
    t->p[P_REL] = 50;
    if (e == &ENG_WAVE)
        wave_preset_words(t->p, pi);
    else
        for (i = 0; i < 8u; i++)
            t->p[P_E0 + i] = e->presets[pi].e[i];
    t->p[P_ED_SHP] = (int16_t)xp_of(&e->presets[pi], P_ED_SHP, 0);
    t->p[P_ED_FLT] = e->presets[pi].fenv;
    t->p[P_ATK] = e->presets[pi].env[0];
    t->p[P_DEC] = e->presets[pi].env[1];
    t->p[P_SUS] = e->presets[pi].env[2];
    t->p[P_REL] = e->presets[pi].env[3];
}

/* a note as voice.c plays it: the ADSR (env_tick), the engine's amp hook, ENV > FLT / SHP; held `held` s then
 * released; per block: the peak, the rms; the whole note's stats; samples into wf */
typedef struct { double rms, dc, first20, at; int32_t peak; } stats_t;
static stats_t play(track_t *t, const engine_t *e, uint32_t note, double held, double total, int32_t sh0, int32_t sh1,
                    FILE *wf, double *ns, double *curve, uint32_t ncurve)
{
    voice_t *v = &t->v[0];
    vmod_t m;
    int32_t out[CTL], env = 0;
    uint32_t b, i, blocks = (uint32_t)(total * FS / CTL), off = (uint32_t)(held * FS / CTL);
    uint32_t b20 = (uint32_t)(0.02 * FS / CTL);
    double s2 = 0, sum = 0, s20 = 0;
    stats_t st = {0, 0, 0, 0, 0};
    clock_t c0;
    memset(v, 0, sizeof *v);
    v->note = (uint8_t)note;
    v->vel = 100;
    v->pitch16 = v->pitch_cur = (int32_t)note * 16;
    e->note_on(t, v);
    v->active = 1;
    v->stage = 1;
    memset(&m, 0, sizeof m);
    m.pitch16 = v->pitch16;
    m.inc = pitch_inc((uint32_t)m.pitch16);
    c0 = clock();
    for (b = 0; b < blocks; b++) {
        int32_t q, a;
        double bs = 0;
        if (b == off)
            v->stage = 3;
        if (v->stage == 1) {
            env += (int32_t)ENV_LIN[t->p[P_ATK] & 127];
            if (env >= 1 << 24)
                env = 1 << 24, v->stage = 2;
        } else if (v->stage == 2) {
            env += mulq16(((int32_t)t->p[P_SUS] << 17) - env, ENV_EXP[t->p[P_DEC] & 127]);
        } else {
            env -= mulq16(env, ENV_EXP[t->p[P_REL] & 127]);
        }
        q = env >> 9;
        a = e->amp ? e->amp(t, v, q) : q;
        m.amp0 = m.amp1;
        m.amp1 = mulq15(a, 100 * 258);
        m.envq15 = a > 32768 ? 32768 : a;
        m.cutoff = (m.envq15 * t->p[P_ED_FLT]) >> 7;
        m.shape = (64 << 8) + sh0 + (int32_t)((int64_t)(sh1 - sh0) * b / blocks) + ((m.envq15 * t->p[P_ED_SHP]) >> 7);
        if (e->block)
            e->block(t);
        for (i = 0; i < CTL; i++)
            out[i] = 0;
        e->render(t, v, out, CTL, &m);
        for (i = 0; i < CTL; i++) {
            int32_t x = out[i], ax = x < 0 ? -x : x;
            if (ax > st.peak)
                st.peak = ax;
            s2 += (double)x * x;
            bs += (double)x * x;
            sum += x;
            if (b < b20)
                s20 += (double)x * x;
            if (wf) {
                int16_t o = (int16_t)clamp(x / 2, -32767, 32767);
                fwrite(&o, 2, 1, wf);
            }
        }
        if (curve && b * ncurve / blocks < ncurve)
            curve[b * ncurve / blocks] += bs;
    }
    if (ns)
        *ns = (double)(clock() - c0) / CLOCKS_PER_SEC * 1e9 / (blocks * CTL);
    st.rms = sqrt(s2 / (blocks * CTL));
    st.dc = sum / (blocks * CTL);
    st.first20 = sqrt(s20 / (b20 * CTL));
    return st;
}
static double db(double x) { return 20 * log10(x + 1); }
static int dcmp(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return x < y ? -1 : x > y; }

int main(int argc, char **argv)
{
    const char *dir = argc > 1 ? argv[1] : "build/host";
    char path[512];
    track_t *t = &trk[0];
    uint32_t tab, f, i, k, pi;
    FILE *wf;
    printf("wt_part_t %u B (arena: GRAIN's ~7 KB)\n", (unsigned)sizeof(wt_part_t));
    /* 1. the packed fields: each alone, read back, the others unchanged, clamped */
    memset(t->p, 0, sizeof t->p);
    for (k = 0; k < WV_N; k++) {
        int32_t before[WV_N], v, j;
        for (j = 0; j < WV_N; j++)
            before[j] = wv_get(t->p, (uint32_t)j);
        for (v = WVD[k].min - 3; v <= WVD[k].max + 3; v++) {
            int32_t want = clamp(v, WVD[k].min, WVD[k].max), ok = 1;
            wv_set(t->p, k, v);
            CHECK(wv_get(t->p, k) == want, "field %u set %d read %d", k, v, wv_get(t->p, k));
            for (j = 0; j < WV_N; j++)
                if (j != (int32_t)k && wv_get(t->p, (uint32_t)j) != before[j])
                    ok = 0;
            CHECK(ok, "field %u set %d moved another field", k, v);
        }
    }
    /* 2. every table in every slot */
    for (tab = 0; tab < WT_NTAB; tab++)
        for (k = 0; k < WT_NOSC; k++) {
            wt_part_t *w;
            int blocks;
            arena_own[0] = 0;
            memset(t->p, 0, sizeof t->p);
            wv_set(t->p, WV_W1 + 3u * k, WM_TAB + (int32_t)tab);
            w = wt_of(t);
            blocks = build(t);
            CHECK(built(t), "table %s osc %u not built", N_WMODE[WM_TAB + tab], k + 1);
            if (k == 0)
                printf("table %-5s built in %3d blocks (%.1f ms):", N_WMODE[WM_TAB + tab], blocks, blocks * CTL * 1000.0 / FS);
            for (f = 0; f < WT_FR; f++) {
                int32_t pk = 0;
                double dc = 0;
                for (i = 0; i < WT_LEN; i++) {
                    int32_t a = w->w[k][f][i] < 0 ? -w->w[k][f][i] : w->w[k][f][i];
                    if (a > pk)
                        pk = a;
                    dc += w->w[k][f][i];
                }
                dc /= WT_LEN;
                if (k == 0)
                    printf(" %d", pk);
                CHECK(pk >= 29000 && pk <= 30005, "%s osc %u frame %u peak %d", N_WMODE[WM_TAB + tab], k + 1, f, pk);
                CHECK(fabs(dc) < 600, "%s osc %u frame %u DC %.0f", N_WMODE[WM_TAB + tab], k + 1, f, dc);
            }
            if (k == 0)
                printf("\n");
        }
    /* 3. two oscillators' tables changed at once, one of them mid-build */
    arena_own[0] = 0;
    memset(t->p, 0, sizeof t->p);
    wv_set(t->p, WV_W1, WM_TAB + WT_VOCAL);
    wv_set(t->p, WV_W2, WM_TAB + WT_DIGI);
    for (i = 0; i < 10; i++)
        wave_block(t);
    wv_set(t->p, WV_W1, WM_TAB + WT_HARM);
    wv_set(t->p, WV_W3, WM_TAB + WT_BELL);
    build(t);
    {
        int32_t ref[WT_LEN];
        wt_part_t *w = wt_of(t);
        uint32_t ph;
        int16_t hm;
        int16_t chk[WT_LEN];
        CHECK(built(t), "three tables changed mid-build");
        for (i = 0; i < WT_LEN; i++)                    /* slot 2 (DIGI) frame 3 as built alone */
            ref[i] = 0;
        for (k = 1; k <= WT_NH; k++) {
            int32_t a = wt_harm(WT_DIGI, 3, k, &ph);
            if (a)
                wt_add(ref, a, k, ph);
        }
        wt_store(chk, ref, &hm);
        CHECK(!memcmp(chk, w->w[1][3], sizeof chk), "DIGI in osc 2 built right while osc 1 changed");
    }
    /* 4. every preset through its ADSR: bounded, audible, no DC; the WAV */
    snprintf(path, sizeof path, "%s/wave_presets.wav", dir);
    wf = wav_open(path);
    for (pi = 0; pi < ENG_WAVE.npresets; pi++) {
        stats_t s;
        arena_own[0] = 0;
        load(t, &ENG_WAVE, pi);
        build(t);
        s = play(t, &ENG_WAVE, 48 + (uint32_t)xp_of(&ENG_WAVE.presets[pi], P_TRANS, 0) + 24 * (ENG_WAVE.presets[pi].mono != 0),
                 0.9, 1.4, 0, 0, wf, 0, 0, 0);
        printf("preset %-10s peak %6d rms %7.0f dc %+6.0f\n", ENG_WAVE.presets[pi].name, s.peak, s.rms, s.dc);
        CHECK(s.peak < 96000, "%s peak %d", ENG_WAVE.presets[pi].name, s.peak);
        CHECK(s.rms > 1000, "%s too quiet (rms %.0f)", ENG_WAVE.presets[pi].name, s.rms);
        CHECK(fabs(s.dc) < 0.05 * s.rms + 300, "%s DC %.0f", ENG_WAVE.presets[pi].name, s.dc);
        s = play(t, &ENG_WAVE, 72, 0.5, 0.8, 0, 0, wf, 0, 0, 0);
        CHECK(s.peak < 96000 && s.rms > 800, "%s note 72 peak %d rms %.0f", ENG_WAVE.presets[pi].name, s.peak, s.rms);
    }
    wav_close(wf);
    /* 5. every WAVE swept by POS, 2 s each, OSC 1 alone, open filter, note 45 */
    snprintf(path, sizeof path, "%s/wave_sweep.wav", dir);
    wf = wav_open(path);
    for (k = WM_SIN; k < WM_N; k++) {
        stats_t s;
        arena_own[0] = 0;
        load(t, &ENG_WAVE, 0);
        t->p[P_ATK] = 0, t->p[P_SUS] = 127;
        wv_set(t->p, WV_W1, (int32_t)k);
        wv_set(t->p, WV_CUT, 127);
        wv_set(t->p, WV_RES, 0);
        build(t);
        s = play(t, &ENG_WAVE, 45, 2.0, 2.0, -64 << 8, 63 << 8, wf, 0, 0, 0);
        CHECK(s.peak < 96000 && s.rms > 3000 && fabs(s.dc) < 0.05 * s.rms + 300, "sweep %s peak %d rms %.0f dc %.0f",
              N_WMODE[k], s.peak, s.rms, s.dc);
    }
    wav_close(wf);
    /* 6. REV: OSC 1 backwards alone swells in over DEC and stops */
    {
        double c[10] = {0};
        stats_t s;
        arena_own[0] = 0;
        load(t, &ENG_WAVE, 0);
        t->p[P_ATK] = 0, t->p[P_DEC] = 90, t->p[P_SUS] = 0, t->p[P_REL] = 40;
        wv_set(t->p, WV_R1, 1);
        build(t);
        s = play(t, &ENG_WAVE, 60, 2.0, 2.0, 0, 0, 0, 0, c, 10);
        printf("REV swell, 0.2 s windows (dB): ");
        for (i = 0; i < 10; i++)
            printf("%.0f ", 10 * log10(c[i] + 1));
        printf("\n");
        CHECK(c[0] * 100 < c[2] || c[0] * 30 < c[3], "REV: quiet at first, louder later");
        {
            uint32_t pk = 0;
            for (i = 1; i < 10; i++)
                if (c[i] > c[pk])
                    pk = i;
            CHECK(pk >= 2 && pk <= 8 && c[9] * 1000 < c[pk], "REV: rises to a peak inside the note (%u), then stops", pk);
        }
        (void)s;
        wv_set(t->p, WV_R1, 0);                         /* forwards: loud first */
        memset(c, 0, sizeof c);
        play(t, &ENG_WAVE, 60, 2.0, 2.0, 0, 0, 0, 0, c, 10);
        CHECK(c[0] > c[5] * 10, "forwards: the same sound decays");
    }
    /* 7. TRANSNT */
    {
        stats_t s0, s1, s2, s3;
        arena_own[0] = 0;
        load(t, &ENG_WAVE, 0);
        t->p[P_ATK] = 0, t->p[P_SUS] = 127;
        build(t);
        s0 = play(t, &ENG_WAVE, 60, 0.5, 0.5, 0, 0, 0, 0, 0, 0);
        wv_set(t->p, WV_TATK, 15);
        s1 = play(t, &ENG_WAVE, 60, 0.5, 0.5, 0, 0, 0, 0, 0, 0);
        wv_set(t->p, WV_TATK, -16);
        s2 = play(t, &ENG_WAVE, 60, 0.5, 0.5, 0, 0, 0, 0, 0, 0);
        wv_set(t->p, WV_TATK, 0);
        wv_set(t->p, WV_TSUS, -16);
        s3 = play(t, &ENG_WAVE, 60, 0.5, 0.5, 0, 0, 0, 0, 0, 0);
        printf("TRANSNT: first 20 ms %.1f dB, ATTCK +15 %.1f, ATTCK -16 %.1f; whole note %.1f, SUSTN -16 %.1f\n",
               db(s0.first20), db(s1.first20), db(s2.first20), db(s0.rms), db(s3.rms));
        CHECK(db(s1.first20) > db(s0.first20) + 6, "ATTCK +15: the attack louder");
        CHECK(db(s2.first20) < db(s0.first20) - 3, "ATTCK -16: the attack faded in");
        CHECK(db(s3.rms) < db(s0.rms) - 10, "SUSTN -16: the body down");
        CHECK(s1.peak < 190000, "ATTCK +15 peak %d", s1.peak);
        wv_set(t->p, WV_TSUS, 0);
    }
    /* 8. DRIVE: every style, PUNSH, TONE */
    {
        static int32_t b[CTL];
        uint32_t st, pun;
        for (st = 0; st < 5; st++)
            for (pun = 0; pun < 2; pun++) {
                double s2 = 0, sum = 0, s2lo = 0;
                int32_t pk = 0, d;
                memset(t->p, 0, sizeof t->p);
                wv_set(t->p, WV_DSTY, (int32_t)st);
                wv_set(t->p, WV_PUN, (int32_t)pun);
                for (d = 20; d <= 127; d += 107) {
                    uint32_t blk, ph = 0;
                    t->dist_hp = t->dist_lp1 = t->dist_lp2 = 0;
                    for (blk = 0; blk < 2000; blk++) {
                        for (i = 0; i < CTL; i++, ph += 0x02000000u)
                            b[i] = osc_saw(ph, 0x02000000u) / 2 + (int32_t)(sine_i(ph >> 3) / 3);
                        wave_drive(t, b, CTL, d);
                        for (i = 0; i < CTL; i++) {
                            int32_t a = b[i] < 0 ? -b[i] : b[i];
                            if (blk > 200) {
                                if (a > pk)
                                    pk = a;
                                if (d == 127)
                                    s2 += (double)b[i] * b[i], sum += b[i];
                                else
                                    s2lo += (double)b[i] * b[i];
                            }
                        }
                    }
                }
                printf("DRIVE %s%s: drive 20 %.1f dB, 127 %.1f dB, peak %d, DC %.0f\n", N_DSTY[st], pun ? " PUNSH" : "",
                       db(sqrt(s2lo / (1799.0 * CTL))), db(sqrt(s2 / (1799.0 * CTL))), pk, sum / (1799.0 * CTL));
                CHECK(pk < 40000, "DRIVE %s peak %d", N_DSTY[st], pk);
                CHECK(fabs(sum / (1799.0 * CTL)) < 3000, "DRIVE %s DC", N_DSTY[st]);
            }
    }
    /* 9. extremes: every field at its ends, three oscillators on */
    {
        static const uint32_t NOTE[] = {0, 24, 60, 96, 127};
        uint32_t q, r;
        for (r = 0; r < 64; r++) {
            arena_own[0] = 0;
            memset(t->p, 0, sizeof t->p);
            t->p[P_SUS] = 127, t->p[P_DEC] = 60, t->p[P_REL] = 40;
            t->p[P_FTYPE] = (int16_t)(r % 3u);
            for (k = 0; k < WV_N; k++)
                wv_set(t->p, k, ((r >> (k % 6)) & 1u) ? WVD[k].max : WVD[k].min);
            for (k = 0; k < WT_NOSC; k++)               /* every oscillator on, a table or a shape */
                wv_set(t->p, WV_W1 + 3u * k, 1 + (int32_t)((r * 3u + k * 5u) % (WM_N - 1u)));
            build(t);
            for (q = 0; q < 5; q++) {
                stats_t s = play(t, &ENG_WAVE, NOTE[q], 0.15, 0.25, -64 << 8, 63 << 8, 0, 0, 0, 0);
                CHECK(s.peak < 190000, "extreme %u note %u: peak %d", r, NOTE[q], s.peak);
            }
        }
    }
    /* 10. level: each preset through its ADSR against ANALOG's trimmed presets (1 s held + 0.6 s release, rms) */
    {
        double tgt[64], lw[64];
        uint32_t na = ENG_ANALOG.npresets, nwv = ENG_WAVE.npresets;
        for (k = 0; k < na; k++) {
            load(t, &ENG_ANALOG, k);
            tgt[k] = db(play(t, &ENG_ANALOG, 48 + (uint32_t)xp_of(&ENG_ANALOG.presets[k], P_TRANS, 0) + 24 * (ENG_ANALOG.presets[k].mono != 0),
                             1.0, 1.6, 0, 0, 0, 0, 0, 0).rms) + PRESET_TRIM[0][k] / 2.0;
        }
        qsort(tgt, na, sizeof tgt[0], dcmp);
        printf("ANALOG trimmed: median %.1f dB (%.1f .. %.1f)\nstatic const int8_t WAVE_TRIM[] = {", tgt[na / 2], tgt[0],
               tgt[na - 1]);
        for (k = 0; k < nwv; k++) {
            int tr;
            arena_own[0] = 0;
            load(t, &ENG_WAVE, k);
            build(t);
            lw[k] = db(play(t, &ENG_WAVE, 48 + (uint32_t)xp_of(&ENG_WAVE.presets[k], P_TRANS, 0) + 24 * (ENG_WAVE.presets[k].mono != 0),
                            1.0, 1.6, 0, 0, 0, 0, 0, 0).rms);
            tr = (int)floor((tgt[na / 2] - lw[k]) * 2 + 0.5);
            printf("%s%d", k ? ", " : "", tr < -26 ? -26 : tr > 23 ? 23 : tr);
        }
        printf("};\n");
        for (k = 0; k < nwv; k++) {
            double d = lw[k] + preset_trim(ENGI_WAVE, k) / 2.0 - tgt[na / 2];
            printf("  %-10s %.1f dB, with its trim %+.1f dB from ANALOG's median\n", ENG_WAVE.presets[k].name, lw[k], d);
            CHECK(fabs(d) < 3.0, "%s level %+.1f dB", ENG_WAVE.presets[k].name, d);
        }
    }
    /* 11. cost: three table oscillators, filter, WARP vs ANALOG REESE */
    {
        double nw, nw1, na;
        arena_own[0] = 0;
        load(t, &ENG_WAVE, 0);
        t->p[P_SUS] = 127;
        wv_set(t->p, WV_W2, WM_TAB + WT_DIGI);
        wv_set(t->p, WV_W3, WM_TAB + WT_VOCAL);
        wv_set(t->p, WV_P1, 40), wv_set(t->p, WV_P2, 70), wv_set(t->p, WV_P3, 90);
        wv_set(t->p, WV_WARP, 30);
        wv_set(t->p, WV_RES, 40);
        build(t);
        play(t, &ENG_WAVE, 48, 20.0, 20.0, 0, 0, 0, &nw, 0, 0);
        wv_set(t->p, WV_W2, WM_OFF), wv_set(t->p, WV_W3, WM_OFF), wv_set(t->p, WV_WARP, 0);
        play(t, &ENG_WAVE, 48, 20.0, 20.0, 0, 0, 0, &nw1, 0, 0);
        load(t, &ENG_ANALOG, 6);
        play(t, &ENG_ANALOG, 48, 20.0, 20.0, 0, 0, 0, &na, 0, 0);
        printf("cost: WAVE 3 tables %.1f ns/sample, 1 table %.1f, ANALOG %s %.1f (ratio %.2f / %.2f)\n", nw, nw1,
               ENG_ANALOG.presets[6].name, na, nw / na, nw1 / na);
        CHECK(nw < na * 2.6, "WAVE (3 tables) costs %.2fx ANALOG", nw / na);
    }
    printf(fails ? "%d FAILED\n" : "all wave checks passed\n", fails);
    return fails != 0;
}
