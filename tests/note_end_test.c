/* SPDX-License-Identifier: GPL-3.0-only */
/* BONBLOOP 1.9.2: a note's own end (core.h SF_END: to 1/8 of a step, track_t nend), not its step's GATE.
 *   live      a note held L/8 of a step while the pattern records plays back L/8 long (1/8 at most off), TIEs where it
 *             reaches into the next steps (1.9.1: the GATE's: a note let go before the middle of its next step lost it)
 *   free      the same in a free take (REC in an empty project), POLY and LEGATO
 *   play      a step's end ends its notes there (NOTE, TIE); a ratchet's hits keep their GATE
 *   project   saved and read back: the ends after 1.9.1's record (its sum as ever); none (a 1.9.1 save): no step has one;
 *             the autosave sees an end change
 * Exit status: the number of failed checks. */
#define FELUCCA_ARRANGER 1
#define main hostsim_main
#include "hostsim.c"
#undef main
#define PROJ_HOST 1
static uint32_t trk_def_engine(uint32_t i) { return i < NPART ? i : 0; }
#include "../firmware/src/project.c"
static struct { uint8_t force; } ui;
static uint8_t sync_reload;
#include "../firmware/src/arranger_scene.c"

static int fails;
static void check(int ok, const char *what) { printf("end: %-96s %s\n", what, ok ? "ok" : "FAIL"); fails += !ok; }
static void block(void) { int32_t out[CTL * 2]; mix_block(out, CTL); }
static int gated(track_t *t, uint32_t note)
{
    uint32_t i;
    for (i = 0; i < NVOICE; i++)
        if (t->v[i].active && t->v[i].note == note && t->v[i].gate)
            return 1;
    return 0;
}
static uint32_t SB;                                     /* blocks a step (1/16 at 120 BPM) */
static void reset(uint32_t ti)
{
    uint32_t i;
    host_tracks_init();
    for (i = 0; i < NTRK; i++) { steps_clear(&trk[i]); trk_all_off(&trk[i]); memset(trk[i].nend, 0, NSTEP); }
    memset(&drums, 0, sizeof drums);
    drums.set = 0;
    song.g[G_BPM] = 120;
    host_preset(&trk[ti], ti == 0 ? 0 : 1, 0);
    trk[ti].p[P_SLEN] = 16;
    trk[ti].p[P_SGATE] = 64;
    song.sel = (uint8_t)ti;
    SB = div_samples(2) / CTL;
}
/* blocks note sounds (gated) in the next n blocks */
static uint32_t gated_for(track_t *t, uint32_t note, uint32_t n)
{
    uint32_t k, g = 0;
    for (k = 0; k < n; k++) {
        block();
        g += (uint32_t)gated(t, note);
    }
    return g;
}

/* live: REC on, the transport from step 0; the note down on step 0, held L/8 of a step; then a pass playing it back */
static void live_len(uint32_t ti, uint32_t L8, const char *what)
{
    track_t *t = &trk[ti];
    uint32_t k, g, want, tol;
    char b[200];
    reset(ti);
    want = L8 * SB / 8u, tol = SB / 8u + SB / 16u;
    song.rec = (uint8_t)(1u << ti);
    rec_wait = 0;
    transport_req = 1;
    block();
    input_on(t, 60, 100);
    for (k = 0; k < want; k++) block();
    input_off(t, 60);
    for (; k < 16u * SB - SB / 2u; k++) block();        /* to half a step before the loop's end */
    song.rec = 0;
    g = gated_for(t, 60, 8u * SB);                       /* (it starts a little early: the latency, its nudge) */
    snprintf(b, sizeof b, "%s, live: held %u/8 of a step, plays back %u blocks (want %u +- %u)", what, L8, g, want, tol);
    check(g + tol >= want && g <= want + tol, b);
    if (L8 == 12u) {
        snprintf(b, sizeof b, "%s, live: 1 1/2 steps: a TIE on step 2, it ends 4/8 into it (1.9.1: one step at its GATE)", what);
        check(t->step[0].time == ST_NOTE && t->step[1].time == ST_TIE && (t->step[1].flags & SF_END) && t->nend[1] == 3u &&
              t->step[2].time == ST_REST, b);
    }
    transport_req = 2;
    block();
}

/* free take: C4 held 1 1/2 steps from the first note, E4 on step 9 held 3/8 of a step; REC closes it after a bar */
static void free_take(uint32_t ti, const char *what)
{
    track_t *t = &trk[ti];
    uint32_t k, g1, g2, tol;
    char b[200];
    reset(ti);
    tol = SB / 8u + SB / 16u;
    rec_tempo = 0;
    rec_count = 0;
    song.rec = 0;
    rec_wait = 1;
    block();
    input_on(t, 60, 100);
    check(ft_on == 1u, "free take: the first note starts it");
    for (k = 0; k < 16u * SB; k++) {
        if (k == 12u * SB / 8u) input_off(t, 60);
        if (k == 8u * SB) input_on(t, 64, 100);
        if (k == 8u * SB + 3u * SB / 8u) input_off(t, 64);
        block();
    }
    ft_close();
    g1 = gated_for(t, 60, 4u * SB);
    for (k = 0; k < 4u * SB; k++) block();
    g2 = gated_for(t, 64, 4u * SB);
    snprintf(b, sizeof b, "%s, free take: 1 1/2 steps plays %u blocks, 3/8 of a step %u (want %u, %u +- %u)", what, g1, g2,
             12u * SB / 8u, 3u * SB / 8u, tol);
    check(g1 + tol >= 12u * SB / 8u && g1 <= 12u * SB / 8u + tol && g2 + tol >= 3u * SB / 8u && g2 <= 3u * SB / 8u + tol, b);
    snprintf(b, sizeof b, "%s, free take: a TIE on step 2 ending 4/8 into it; step 9 ends 3/8 into it", what);
    check(t->step[1].time == ST_TIE && (t->step[1].flags & SF_END) && t->nend[1] == 3u && (t->step[8].flags & SF_END) &&
          t->nend[8] == 2u, b);
    transport_req = 2;
    block();
}

static void play(void)
{
    track_t *t = &trk[1];
    step_t *s = &t->step[3];
    reset(1);
    memset(s, 0, sizeof *s);
    s->n = 1, s->note[0] = 60, s->time = ST_NOTE, s->flags = SF_END;
    t->nend[3] = 2;
    t->seq_idx = 3;
    seq_step(t, s, 800u, 0);
    check(t->seq_off == 300u && !t->seq_hold, "a NOTE with its own end 2 (3/8): its notes end 3/8 into the step");
    s->flags = 0;
    seq_step(t, s, 800u, 0);
    check(t->seq_off == 400u, "without SF_END: the GATE's (50 %), as ever");
    s->flags = SF_END, s->rat = 1;
    seq_step(t, s, 800u, 0);
    check(t->seq_off == 200u, "a ratchet (x2): each hit its GATE, not the end");
    s->rat = 0;
    seq_step(t, s, 800u, 0);
    s->time = ST_TIE, t->nend[3] = 7;
    seq_step(t, s, 800u, 0);
    check(t->seq_n && t->seq_off == 800u, "a TIE with its own end 7: the notes end with the step (a GATE's: half a step later)");
    trk_all_off(t);
}

static project_t pj;
static void project(void)
{
    bbx_t *x = bbx_of(&pj);
    uint32_t tag, ok = 1, i, k;
    reset(1);
    trk[1].step[5] = (step_t){{60}, 1, ST_NOTE, SF_END, 0, 0, 0};
    trk[1].nend[5] = 6;
    trk[0].step[63] = (step_t){{0}, 0, ST_TIE, SF_END, 0, 0, 0};
    trk[0].nend[63] = 7;
    trk[2].step[0] = (step_t){{48}, 1, ST_NOTE, SF_END | SF_ACCENT, 0, 0, 0};
    trk[2].nend[0] = 1;
    trk[2].nend[1] = 5;                                  /* (no SF_END there: not one of its ends) */
    proj_capture(&pj);
    tag = proj_tag(&pj);
    check(__builtin_offsetof(bbx_t, sum) == 102u && sizeof(bbx_t) <= 192u && bbx_ok(x),
          "the side record: 1.9.1's 104 bytes as ever (its sum: of the 102 before it), the ends after it, 192 at most");
    for (i = 0; i < NPART; i++) { memset(trk[i].nend, 0, NSTEP); for (k = 0; k < NSTEP; k++) trk[i].step[k].flags &= (uint8_t)~SF_END; }
    proj_apply(&pj, 1);
    ok = trk[1].nend[5] == 6u && (trk[1].step[5].flags & SF_END) && trk[0].nend[63] == 7u && (trk[0].step[63].flags & SF_END) &&
         trk[2].nend[0] == 1u && trk[2].step[0].flags == (SF_END | SF_ACCENT) && trk[2].nend[1] == 5u;
    check(ok, "saved and read back: each track's ends (3 bits a step), the steps' SF_END");
    trk[1].nend[5] = 3;
    proj_capture(&pj);
    check(proj_tag(&pj) != tag, "an end changed: the autosave sees it");
    memset(x->nend, 0xFF, sizeof x->nend + 2u);          /* (a 1.9.1 save: the area after its record erased) */
    proj_apply(&pj, 1);
    ok = bbx_ok(x);
    for (i = 0; i < NPART; i++)
        for (k = 0; k < NSTEP; k++)
            ok &= !(trk[i].step[k].flags & SF_END) && !trk[i].nend[k];
    check(ok && trk[2].step[0].flags == SF_ACCENT, "a 1.9.1 project (no ends): its extras read, no step has an end (the GATE's)");
}

int main(void)
{
    static const uint32_t L[5] = {3, 6, 9, 12, 20};
    uint32_t i;
    for (i = 0; i < 5u; i++)
        live_len(1, L[i], "track 2 RHODES (POLY)");
    live_len(0, 12, "track 1 808 BOOM (LEGATO)");
    free_take(1, "track 2 RHODES (POLY)");
    free_take(0, "track 1 808 BOOM (LEGATO)");
    play();
    project();
    printf("end: %d failed\n", fails);
    return fails;
}
