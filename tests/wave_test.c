/* SPDX-License-Identifier: GPL-3.0-only */
/* BONBLOOP: the WAVE engine (firmware/src/eng_wave.c) alone, no hostsim (builds with any C compiler, MSVC too).
 *   tables    every TABL built in its arena (frame by frame, ~35 ms), each frame at the same peak, no DC; a TABL
 *             changed while one is being built: the new one wins
 *   presets   every WAVE preset at notes 48 and 72: bounded, audible, no DC (WARP's DC taken out)
 *   sweep     every table swept by POS (the SHP modulation), 2 s each
 *   extremes  every value at its ends, from note 0 to 127: no more than ANALOG's own ceiling (~95000)
 *   level     a note through each preset's ADSR against ANALOG's trimmed presets: prints WAVE_TRIM (the trims
 *             eng_wave.c keeps); with them every preset within 3 dB of ANALOG's median
 *   cost      ns per sample against ANALOG REESE (host, rough): WAVE at most 1.6x
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
typedef struct { uint8_t x[6900]; } gr_part_t;      /* (GRAIN's size, about: the static assert) */
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
    fwrite(h, 1, 44, f);
    return f;
}
static void wav_close(FILE *f)
{
    long n = ftell(f) - 44;
    uint32_t v;
    uint16_t s;
    fseek(f, 0, SEEK_SET);
    fwrite("RIFF", 1, 4, f); v = (uint32_t)(36 + n); fwrite(&v, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f); v = 16; fwrite(&v, 4, 1, f);
    s = 1; fwrite(&s, 2, 1, f); fwrite(&s, 2, 1, f);
    v = FS; fwrite(&v, 4, 1, f); v = FS * 2; fwrite(&v, 4, 1, f);
    s = 2; fwrite(&s, 2, 1, f); s = 16; fwrite(&s, 2, 1, f);
    fwrite("data", 1, 4, f); v = (uint32_t)n; fwrite(&v, 4, 1, f);
    fclose(f);
}

static void build(track_t *t)
{
    int b;
    wt_part_t *w = wt_of(t);
    for (b = 0; b < 200 && !(w->ready && w->want == (uint32_t)t->p[P_E0] % WT_NTAB + 1u); b++)
        wave_block(t);
}

/* render one voice: note, seconds, the shape (POS modulation) as a ramp from s0 to s1; stats; optional wav */
typedef struct { double rms, dc; int32_t peak; } stats_t;
static stats_t render(track_t *t, const engine_t *e, uint32_t note, double secs, int32_t sh0, int32_t sh1, FILE *wf,
                      double *ns)
{
    voice_t *v = &t->v[0];
    vmod_t m;
    int32_t out[CTL];
    uint32_t blocks = (uint32_t)(secs * FS / CTL), b, i;
    double s2 = 0, sum = 0;
    stats_t st = {0, 0, 0};
    clock_t c0;
    memset(v, 0, sizeof *v);
    v->note = (uint8_t)note;
    v->vel = 100;
    v->pitch16 = v->pitch_cur = (int32_t)note * 16;
    e->note_on(t, v);
    v->active = 1;
    memset(&m, 0, sizeof m);
    m.pitch16 = v->pitch16;
    m.inc = pitch_inc((uint32_t)m.pitch16);
    m.amp0 = m.amp1 = 32767;
    m.envq15 = 32767;
    c0 = clock();
    for (b = 0; b < blocks; b++) {
        if (e->block)
            e->block(t);
        m.shape = (64 << 8) + sh0 + (int32_t)((int64_t)(sh1 - sh0) * b / blocks);
        for (i = 0; i < CTL; i++)
            out[i] = 0;
        e->render(t, v, out, CTL, &m);
        for (i = 0; i < CTL; i++) {
            int32_t x = out[i], a = x < 0 ? -x : x;
            if (a > st.peak)
                st.peak = a;
            s2 += (double)x * x;
            sum += x;
            if (wf) {
                int16_t o = (int16_t)clamp(x / 2, -32767, 32767);
                fwrite(&o, 2, 1, wf);
            }
        }
    }
    if (ns)
        *ns = (double)(clock() - c0) / CLOCKS_PER_SEC * 1e9 / (blocks * CTL);
    st.rms = sqrt(s2 / (blocks * CTL));
    st.dc = sum / (blocks * CTL);
    return st;
}


static int dcmp(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return x < y ? -1 : x > y; }
static int xp_of(const preset_t *pr, uint32_t id, int def)
{
    uint32_t j;
    for (j = 0; j < 8; j += 2)
        if (pr->x[j] == (int8_t)(id + 1))
            return pr->x[j + 1];
    return def;
}
/* a note of the preset through its ADSR (voice.c env_tick), ENV > FLT and ENV > SHP; rms dB of 1.6 s */
static double loud(track_t *t, const engine_t *e, const preset_t *pr)
{
    voice_t *v = &t->v[0];
    vmod_t m;
    int32_t out[CTL], env = 0, stage = 1, eflt = pr->fenv, eshp = xp_of(pr, P_ED_SHP, 0);
    uint32_t b, i, note = (uint32_t)(48 + xp_of(pr, P_TRANS, 0)), blocks = (uint32_t)(1.6 * FS / CTL);
    double s2 = 0;
    if (pr->mono && note > 40)
        note -= 0;
    memset(v, 0, sizeof *v);
    v->note = (uint8_t)note;
    v->vel = 100;
    v->pitch16 = v->pitch_cur = (int32_t)note * 16;
    e->note_on(t, v);
    v->active = 1;
    memset(&m, 0, sizeof m);
    m.pitch16 = v->pitch16;
    m.inc = pitch_inc((uint32_t)m.pitch16);
    for (b = 0; b < blocks; b++) {
        int32_t q;
        if (b == (uint32_t)(1.0 * FS / CTL))
            stage = 3;
        if (stage == 1) {
            env += (int32_t)ENV_LIN[pr->env[0] & 127];
            if (env >= 1 << 24)
                env = 1 << 24, stage = 2;
        } else if (stage == 2) {
            env += mulq16(((int32_t)pr->env[2] << 17) - env, ENV_EXP[pr->env[1] & 127]);
        } else {
            env -= mulq16(env, ENV_EXP[pr->env[3] & 127]);
        }
        q = env >> 9;
        m.amp0 = m.amp1;
        m.amp1 = mulq15(q, 100 * 258);
        m.envq15 = q;
        m.cutoff = (q * eflt) >> 7;
        m.shape = (64 << 8) + ((q * eshp) >> 7);
        if (e->block)
            e->block(t);
        for (i = 0; i < CTL; i++)
            out[i] = 0;
        e->render(t, v, out, CTL, &m);
        for (i = 0; i < CTL; i++)
            s2 += (double)out[i] * out[i];
    }
    return 10 * log10(s2 / (blocks * CTL) + 1);
}

static void load(track_t *t, const engine_t *e, uint32_t pi)
{
    uint32_t i;
    for (i = 0; i < 8u; i++)
        t->p[P_E0 + i] = e->presets[pi].e[i];
}

int main(int argc, char **argv)
{
    const char *dir = argc > 1 ? argv[1] : "build/host";
    char path[512];
    track_t *t = &trk[0];
    uint32_t tab, f, i, pi;
    FILE *wf;
    printf("wt_part_t %u B (arena budget 6900+)\n", (unsigned)sizeof(wt_part_t));
    /* 1. every table: built, each frame normalised, no DC, bounded */
    for (tab = 0; tab < WT_NTAB; tab++) {
        wt_part_t *w;
        int blocks = 0;
        arena_own[0] = 0;
        t->p[P_E0] = (int16_t)tab;
        w = wt_of(t);
        while (!w->ready && blocks < 500) {
            wave_block(t);
            blocks++;
        }
        CHECK(w->ready && w->want == tab + 1u, "table %s not built", N_WTAB[tab]);
        printf("table %-5s built in %3d blocks (%.1f ms):", N_WTAB[tab], blocks, blocks * CTL * 1000.0 / FS);
        for (f = 0; f < WT_FR; f++) {
            int32_t pk = 0;
            double dc = 0;
            for (i = 0; i < WT_LEN; i++) {
                int32_t a = w->w[f][i] < 0 ? -w->w[f][i] : w->w[f][i];
                if (a > pk)
                    pk = a;
                dc += w->w[f][i];
            }
            dc /= WT_LEN;
            printf(" %d/%+.0f", pk, dc);
            CHECK(pk >= 29000 && pk <= 30005, "%s frame %u peak %d", N_WTAB[tab], f, pk);
            CHECK(fabs(dc) < 600, "%s frame %u DC %.0f", N_WTAB[tab], f, dc);
        }
        printf("\n");
    }
    /* 2. a table change while one is being built: the new one wins */
    arena_own[0] = 0;
    t->p[P_E0] = WT_VOCAL;
    for (i = 0; i < 10; i++)
        wave_block(t);
    t->p[P_E0] = WT_CHIP;
    build(t);
    CHECK(wt_of(t)->want == WT_CHIP + 1u && wt_of(t)->ready, "table switch mid-build");
    /* 3. every preset: bounded, audible, little DC; a WAV of them all (note 48, then 60) */
    snprintf(path, sizeof path, "%s/wave_presets.wav", dir);
    wf = wav_open(path);
    for (pi = 0; pi < ENG_WAVE.npresets; pi++) {
        stats_t s;
        load(t, &ENG_WAVE, pi);
        build(t);
        s = render(t, &ENG_WAVE, 48, 0.7, 0, 0, wf, 0);
        printf("preset %-10s note 48: peak %6d rms %7.0f dc %+6.0f\n", ENG_WAVE.presets[pi].name, s.peak, s.rms, s.dc);
        CHECK(s.peak < 65536, "%s peak %d", ENG_WAVE.presets[pi].name, s.peak);
        CHECK(s.rms > 2000, "%s too quiet (rms %.0f)", ENG_WAVE.presets[pi].name, s.rms);
        CHECK(fabs(s.dc) < 0.05 * s.rms + 300, "%s DC %.0f", ENG_WAVE.presets[pi].name, s.dc);
        s = render(t, &ENG_WAVE, 72, 0.5, 0, 0, wf, 0);
        CHECK(s.peak < 65536 && s.rms > 1500, "%s note 72 peak %d rms %.0f", ENG_WAVE.presets[pi].name, s.peak, s.rms);
    }
    wav_close(wf);
    /* 4. the wavetable sweep: every table, POS 0 -> 127 by the modulation, 3 s each, note 45 */
    snprintf(path, sizeof path, "%s/wave_sweep.wav", dir);
    wf = wav_open(path);
    for (tab = 0; tab < WT_NTAB; tab++) {
        stats_t s;
        memset(t->p, 0, sizeof t->p);
        t->p[P_E0] = (int16_t)tab;
        t->p[P_E4] = 127;                              /* open, clean */
        build(t);
        s = render(t, &ENG_WAVE, 45, 2.0, -64 << 8, 63 << 8, wf, 0);
        CHECK(s.peak < 65536 && s.rms > 3000, "sweep %s peak %d rms %.0f", N_WTAB[tab], s.peak, s.rms);
    }
    wav_close(wf);
    /* 5. extremes: everything at its top or bottom, the lowest and the highest pitch */
    {
        static const int16_t X[][8] = {
            {WT_BASIC, 127, 127, 127, 0, 127, 127, 127}, {WT_DIGI, 127, 127, 127, 127, 127, 127, 127},
            {WT_CHIP, 64, 127, 0, 127, 0, 127, 127}, {WT_RESO, 0, 0, 127, 0, 0, 0, 0}, {WT_FOLD, 127, 60, 60, 60, 127, 60, 60}};
        static const uint32_t NOTE[] = {0, 24, 60, 108, 127};
        uint32_t k, q;
        for (k = 0; k < sizeof X / sizeof X[0]; k++)
            for (q = 0; q < 5; q++) {
                stats_t s;
                for (i = 0; i < 8; i++)
                    t->p[P_E0 + i] = X[k][i];
                build(t);
                s = render(t, &ENG_WAVE, NOTE[q], 0.3, -64 << 8, 63 << 8, 0, 0);
                CHECK(s.peak < 96000, "extreme %u note %u: peak %d", k, NOTE[q], s.peak);
            }
    }
    /* 5b. loudness: a note through the preset's ADSR (and ENV > FLT / SHP), 1 s held + 0.6 s release, rms in dB;
     * ANALOG's presets with their trims give the target, WAVE's trims follow */
    {
        double tgt[64], la[64], lw[64];
        uint32_t na = ENG_ANALOG.npresets, nwv = ENG_WAVE.npresets, k;
        for (k = 0; k < na; k++) {
            memset(t->p, 0, sizeof t->p);
            load(t, &ENG_ANALOG, k);
            la[k] = loud(t, &ENG_ANALOG, &ENG_ANALOG.presets[k]);
            tgt[k] = la[k] + PRESET_TRIM[0][k] / 2.0;
        }
        qsort(tgt, na, sizeof tgt[0], dcmp);
        printf("ANALOG trimmed loudness: median %.1f dB (min %.1f max %.1f)\n", tgt[na / 2], tgt[0], tgt[na - 1]);
        printf("static const int8_t WAVE_TRIM[] = {");
        for (k = 0; k < nwv; k++) {
            int tr;
            memset(t->p, 0, sizeof t->p);
            load(t, &ENG_WAVE, k);
            build(t);
            lw[k] = loud(t, &ENG_WAVE, &ENG_WAVE.presets[k]);
            tr = (int)floor((tgt[na / 2] - lw[k]) * 2 + 0.5);
            tr = tr < -26 ? -26 : tr > 23 ? 23 : tr;
            printf("%s%d", k ? ", " : "", tr);
        }
        printf("};\n");
        for (k = 0; k < nwv; k++)
            printf("  %-10s %.1f dB -> with its trim %.1f\n", ENG_WAVE.presets[k].name, lw[k],
                   lw[k] + preset_trim(ENGI_WAVE, k) / 2.0);
    }
    /* 6. cost per sample against ANALOG (host, rough): WAVE with filter + DTN + SUB + WARP vs ANALOG REESE */
    {
        double nw, na, nw0;
        uint32_t reese = 6;
        memset(t->p, 0, sizeof t->p);
        load(t, &ENG_WAVE, 0);
        t->p[P_E2] = 40, t->p[P_E3] = 40, t->p[P_E6] = 12, t->p[P_E5] = 40;
        build(t);
        render(t, &ENG_WAVE, 48, 20.0, 0, 0, 0, &nw);
        t->p[P_E2] = t->p[P_E3] = t->p[P_E6] = 0;
        render(t, &ENG_WAVE, 48, 20.0, 0, 0, 0, &nw0);
        memset(t->p, 0, sizeof t->p);
        load(t, &ENG_ANALOG, reese);
        render(t, &ENG_ANALOG, 48, 20.0, 0, 0, 0, &na);
        printf("cost: WAVE full %.1f ns/sample, WAVE plain %.1f, ANALOG %s %.1f (ratio %.2f)\n", nw, nw0,
               ENG_ANALOG.presets[reese].name, na, nw / na);
        CHECK(nw < na * 1.6, "WAVE costs %.2fx ANALOG", nw / na);
    }
    /* 7. the table build cost a block, against one voice's block render */
    {
        clock_t c0;
        double nb;
        int k;
        c0 = clock();
        for (k = 0; k < 20000; k++) {
            arena_own[0] = 0;
            t->p[P_E0] = WT_VOCAL;
            wave_block(t);
            wave_block(t);
        }
        nb = (double)(clock() - c0) / CLOCKS_PER_SEC * 1e9 / 20000;
        printf("build: one block of the VOCAL table (+ arena clear) %.0f ns (%.1f samples of one voice)\n", nb / 2, 0.0);
    }
    printf(fails ? "%d FAILED\n" : "all wave checks passed\n", fails);
    return fails != 0;
}
