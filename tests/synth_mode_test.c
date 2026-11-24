/* SPDX-License-Identifier: GPL-3.0-only */
/* BONBLOOP 1.1 / 1.2: the SYNTH MODE and the WAVE engine on the real UI and audio (the ui_pages_test harness).
 *   ENV held 0.7 s: the mode on; the selected track plays WAVE, OSC 1 opens; again: off, the ENV page
 *   EDIT / ENV / LFO / FX tapped: their synth pages (again: the next of the group); SELECT walks those shown
 *   1.2: each oscillator's LFO; 1.3: no ENV of the voice, a whole ENV per oscillator (OSC 1's the track's), the voice
 *   sounding while one still releases; FX: the chain (every effect off at first), an effect's pages while it is in
 *   it; the delay's 1/2, FREE ms and FIX; REC + SAVE held: the synth's sound back to its start
 *   the mode keeps the screen: ARP, SEQ, SEL, GLO, SAVE tapped open nothing, HOME is OSC 1, ALGORITHM stays on the
 *   synth tracks, anything that took the screen gives it back
 *   the knobs edit the packed fields (WAVE, POS, LVL, REV ...), one field each; PRESETS: WAVE's sounds
 *   three oscillators sound, each table built; ENV / LFO > SHP read POS on WAVE
 *   screens: DIR/synth-*.ppm
 *   synth_mode_test [DIR] */
#define UI_PAGES_HARNESS_ONLY 1
#include "ui_pages_test.c"

static const char *pg_title(void) { return ui.home ? "HOME" : cur_page()->title; }
static void hold_env(void)                          /* ENV held 0.8 s, let go */
{
    press(B_ENV);
    frames(50);
    release(B_ENV);
}
static void turn(uint32_t role, int32_t s) { encs[panel.enc[role]] += s * panel.dir[role]; frame(); }
static int32_t peak_of(uint32_t blocks)            /* the mix's peak over some blocks */
{
    static int32_t o[CTL * 2];
    int32_t pk = 0;
    uint32_t b, i;
    for (b = 0; b < blocks; b++) {
        mix_block(o, CTL);
        for (i = 0; i < CTL * 2u; i++)
            if ((o[i] < 0 ? -o[i] : o[i]) > pk)
                pk = o[i] < 0 ? -o[i] : o[i];
    }
    return pk;
}
static void shot(const char *name) { ui.force = 1; frame(); ppm(name); }
static int field(uint32_t k) { return (int)wv_get(TSEL->p, k); }

int main(int argc, char **argv)
{
    uint32_t i, k;
    char b[120];
    outdir = argc > 1 ? argv[1] : "build/host";
    panel = PANEL_DEFAULT;
    layers_init();
    settings.palette = 4;
    palette_set(4);
    host_tracks_init();
    for (i = 0; i < NPART; i++) { set_engine_of(&trk[i], TRK_DEF[i][0]); apply_preset_to(&trk[i], TRK_DEF[i][1]); trk[i].engine = trk[i].eng_req; }
    TDRUM->p[P_E0] = DRUM_DEFAULT_KIT;
    sloop_splash();
    ppm("synth-boot");
    go_home();
    shot("synth-start");
    track_select(1);
    frames(2);
    check(!syn_mode && TSEL->eng_req != ENGI_WAVE, "start: SLOOP as it is, track 2 not on WAVE");

    /* on */
    press(B_ENV);
    frames(5);
    check(!syn_mode && !ui.home && cur_page()->fam == FAM_ENV, "ENV pressed: its page at once, as SLOOP (no mode yet)");
    frames(45);
    release(B_ENV);
    check(syn_mode == 1, "ENV held 0.8 s: SYNTH MODE on");
    check(song.sel == 1 && TSEL->eng_req == ENGI_WAVE, "the selected track (2) plays WAVE");
    check(TSEL->preset == WAVE_SINE && field(WV_W1) == WM_SIN && field(WV_W2) == WM_OFF && field(WV_W3) == WM_OFF &&
          !TSEL->p[P_DIST] && !TSEL->p[P_CHOR] && !TSEL->p[P_DLY] && !TSEL->p[P_REV] && !TSEL->p[P_ITYPE] && !TSEL->p[P_TFLT] &&
          !TSEL->p[P_LD_PIT] && !TSEL->p[P_LD_SHP] && !sx_get(1, SX_LD1),
          "1.4: it starts as SINE: OSC 1 a sine, OSC 2 and 3 off, no effect, no LFO");
    check(on_syn_page() && str_eq(pg_title(), "OSC 1"), "OSC 1 opens");
    shot("synth-splash");
    check(cur_btn() == B_EDIT, "OSC 1's light: EDIT");
    for (i = 0; i < WM_TAB + WT_BASIC - WM_SIN; i++)
        turn(EN_K1, 1);
    frames(60);
    check(field(WV_W1) == WM_TAB + WT_BASIC, "KNOB 1 on OSC 1: SIN -> TRI -> SAW -> SQR -> NOISE -> the BASIC table");
    check(TSEL->engine == ENGI_WAVE && wt_peek(TSEL) && wt_peek(TSEL)->sl[0].ready, "OSC 1's table built in the arena");
    shot("synth-osc1");

    /* the knobs: one field each */
    {
        int16_t w[8];
        int p1 = field(WV_P1), changed = 0;
        memcpy(w, &TSEL->p[P_E0], sizeof w);
        turn(EN_K2, 5);
        for (k = 0; k < 8u; k++)
            changed += w[k] != TSEL->p[P_E0 + k];
        check(field(WV_P1) > p1 && changed == 1 && field(WV_W1) == WM_TAB + WT_BASIC, "KNOB 2 on OSC 1: POS up, nothing else");
        turn(EN_K4, 1);
        check(field(WV_R1) == 1, "KNOB 4 on OSC 1: REV on");
        turn(EN_K4, -1);
        check(field(WV_R1) == 0, "and off");
    }

    /* three oscillators */
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "OSC 2"), "EDIT again: OSC 2");
    for (i = 0; i < 3u; i++)
        turn(EN_K1, 1);
    check(field(WV_W2) == WM_SAW, "KNOB 1 on OSC 2: WAVE OFF -> SIN -> TRI -> SAW");
    shot("synth-osc2-saw");
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "OSC 3"), "EDIT again: OSC 3");
    for (i = 0; i < WM_TAB + WT_VOCAL; i++)
        turn(EN_K1, 1);
    check(field(WV_W3) == WM_TAB + WT_VOCAL, "KNOB 1 on OSC 3: to the VOCAL table");
    frames(6);
    check(wt_peek(TSEL)->sl[2].ready && wt_peek(TSEL)->sl[2].want == WT_VOCAL + 1u, "OSC 3's table built in its own slot");
    shot("synth-osc3-vocal");
    tap(B_EDIT);
    {   /* 1.7: OSC MIX, the three oscillators' levels and the track's */
        int16_t *vp;
        const param_desc_t *d1 = page_desc(cur_page(), 0, &vp), *d3, *d4;
        int32_t l2 = field(WV_L2);
        d3 = page_desc(cur_page(), 2, &vp);
        d4 = page_desc(cur_page(), 3, &vp);
        check(on_syn_page() && str_eq(pg_title(), "OSC MIX") && d1 && str_eq(d1->label, "LVL1") && d3 &&
              str_eq(d3->label, "LVL3") && d4 == &TP[P_LEVEL] && vp == &TSEL->p[P_LEVEL],
              "1.7: EDIT again: OSC MIX (LVL1 LVL2 LVL3, the track's LEVEL)");
        turn(EN_K2, -3);
        check(field(WV_L2) == l2 - 3 && cur_page()->graph == GR_FX, "1.7: OSC MIX: KNOB 2 OSC 2's level; drawn as faders");
        ui.force = 1; frame(); shot("synth-osc-mix");
        turn(EN_K2, 3);
    }
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "TUNE"), "EDIT again: TUNE");
    turn(EN_K1, -12);
    check(field(WV_S2) <= -10, "KNOB 1 on TUNE: OSC 2 down (SEMI2)");
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "FILTER"), "EDIT again: FILTER (1.2: the voice's filter is EDIT's)");
    turn(EN_K3, 1);
    check(TSEL->p[P_FTYPE] == 1, "KNOB 3 on FILTER: TYPE BP");
    turn(EN_K3, -1);
    shot("synth-filter");
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "VOICE"), "EDIT again: VOICE");
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "OSC 1"), "EDIT again: back to OSC 1");
    TSEL->p[P_LD_SHP] = 50;
    TSEL->p[P_LRATE] = 70;
    fm1_in.notes |= 1u << 12;
    frame();
    {
        int32_t pk = peak_of(400);
        snprintf(b, sizeof b, "a key with three oscillators sounds (peak %d), bounded", pk);
        check(pk > 1000 && pk < 2000000, b);
    }
    shot("synth-osc1-playing");
    fm1_in.notes &= ~(1u << 12);
    frames(80);

    /* ENV, LFO, FX */
    tap(B_ENV);
    check(on_syn_page() && str_eq(pg_title(), "OSC1 ENV") && cur_btn() == B_ENV,
          "ENV tapped: OSC1 ENV (1.3: no ENV of the voice), ENV lit");
    tap(B_ENV); tap(B_ENV); tap(B_ENV);
    check(on_syn_page() && str_eq(pg_title(), "ENV DEST"), "ENV three more: OSC2 ENV, OSC3 ENV, ENV DEST");
    {
        int16_t *vp;
        const param_desc_t *d = page_desc(cur_page(), 2, &vp);
        check(d && str_eq(d->label, "POS") && vp == &TSEL->p[P_ED_SHP], "ENV DEST: SHP reads POS on WAVE");
    }
    tap(B_LFO);
    check(on_syn_page() && str_eq(pg_title(), "LFO") && cur_btn() == B_LFO, "LFO tapped: the synth LFO page, LFO lit");
    {   /* 1.3: an ENV per oscillator, each as SLOOP's; OSC 1's the track's */
        uint32_t part = (uint32_t)(TSEL - trk);
        int32_t a0, r0;
        tap(B_ENV);
        check(on_syn_page() && str_eq(pg_title(), "OSC1 ENV"), "ENV round its group: OSC1 ENV");
        {
            int16_t *vp;
            const param_desc_t *d = page_desc(cur_page(), 0, &vp);
            check(d == &TP[P_ATK] && vp == &TSEL->p[P_ATK], "OSC1 ENV: ATK is the track's ENV (P_ATK)");
        }
        check(sx_get(part, SX_A2) == TSEL->p[P_ATK] && sx_get(part, SX_S3) == TSEL->p[P_SUS] &&
              sx_get(part, SX_R2) == TSEL->p[P_REL], "the sound's start: OSC 2 and 3's ENVs as OSC 1's");
        tap(B_ENV);
        check(on_syn_page() && str_eq(pg_title(), "OSC2 ENV") && cur_page()->graph == GR_ADSR, "ENV again: OSC2 ENV, its ADSR");
        {
            int16_t *vp;
            const param_desc_t *d = page_desc(cur_page(), 3, &vp);
            check(d == &TP[P_REL] && vp && *vp == sx_get(part, SX_R2), "OSC2 ENV: REL shown as SLOOP's, its own value");
        }
        a0 = sx_get(part, SX_A2), r0 = TSEL->p[P_ATK];
        turn(EN_K1, 4);
        turn(EN_K3, -3);
        check(sx_get(part, SX_A2) > a0 && sx_get(part, SX_S2) < TSEL->p[P_SUS] && TSEL->p[P_ATK] == r0,
              "OSC2 ENV: KNOB 1 its ATK up, KNOB 3 its SUS down; OSC 1's (the track's) as it was");
        {   /* 1.3.1: the drawing follows OSC 2's own values (not the track's parameters: ui_draw.c graph_signature) */
            uint32_t g0 = graph_signature();
            sx_set(part, SX_A2, sx_get(part, SX_A2) + 1);
            check(graph_signature() != g0, "1.3.1: OSC2 ENV's drawing redraws when its own ATK changes");
            sx_set(part, SX_A2, sx_get(part, SX_A2) - 1);
        }
        shot("synth-osc2-env");
        {   /* the voice sounds while an oscillator still releases: OSC 2's REL long, OSC 1's short */
            uint32_t i, on;
            sx_set(part, SX_R2, 120);
            sx_set(part, SX_R3, 0);
            TSEL->p[P_REL] = 0;
            fm1_in.notes |= 1u << 12;
            frames(30);
            fm1_in.notes &= ~(1u << 12);
            frames(25);
            for (i = on = 0; i < NVOICE; i++)
                on |= TSEL->v[i].active;
            {
                int32_t pk = peak_of(20);
                snprintf(b, sizeof b, "released, OSC 1 gone: OSC 2's long REL still sounds (peak %d)", pk);
                check(on && pk > 50, b);
            }
            sx_set(part, SX_R2, 0);
            fm1_in.notes |= 1u << 12;
            frames(30);
            fm1_in.notes &= ~(1u << 12);
            frames(25);
            for (i = on = 0; i < NVOICE; i++)
                on |= TSEL->v[i].active;
            check(!on, "every REL short: the voice ends");
            sx_set(part, SX_R2, 40);                    /* (back to a usable release) */
            TSEL->p[P_REL] = 40;
        }
        tap(B_ENV);
        check(on_syn_page() && str_eq(pg_title(), "OSC3 ENV"), "ENV again: OSC3 ENV");
        tap(B_ENV);
        check(on_syn_page() && str_eq(pg_title(), "ENV DEST"), "ENV again: ENV DEST");
        /* 1.2: each oscillator's LFO */
        tap(B_LFO);
        check(on_syn_page() && str_eq(pg_title(), "LFO") && cur_btn() == B_LFO, "LFO tapped: the synth LFO page, LFO lit");
        tap(B_LFO); tap(B_LFO); tap(B_LFO);
        check(on_syn_page() && str_eq(pg_title(), "OSC1 LFO"), "LFO three more: OSC1 LFO");
        turn(EN_K1, 9);
        turn(EN_K2, 6);
        turn(EN_K3, 1);
        check(sx_get(part, SX_LR1) == 9 && sx_get(part, SX_LD1) == 6 && sx_get(part, SX_LT1) == 1 && TSEL->p[P_LD_PIT] == 0,
              "OSC1 LFO: RATE 9, DEPTH 6, DEST PITCH: its own, the voice's LFO untouched");
        shot("synth-osc1-lfo");
        fm1_in.notes |= 1u << 12;
        frame();
        {
            int32_t pk = peak_of(400);
            snprintf(b, sizeof b, "OSC 1 with its own ENV and LFO: a key sounds (peak %d), bounded", pk);
            check(pk > 500 && pk < 2000000, b);
        }
        fm1_in.notes &= ~(1u << 12);
        frames(80);
    }

    /* 1.2: FX, the chain: every effect off at first, an effect's pages while it is in the chain */
    {
        uint32_t part = (uint32_t)(TSEL - trk);
        check(!TSEL->p[P_DIST] && !TSEL->p[P_CHOR] && !TSEL->p[P_DLY] && !TSEL->p[P_REV] && !TSEL->p[P_ITYPE] &&
              !TSEL->p[P_TFLT] && !sx_chain_has(part, FXI_DRIVE),
              "the synth's sound: every effect off, the chain empty");
        tap(B_FX);
        check(on_syn_page() && str_eq(pg_title(), "FX CHAIN") && cur_btn() == B_FX, "FX tapped: FX CHAIN, FX lit");
        shot("synth-chain-empty");
        tap(B_FX);
        check(str_eq(pg_title(), "CHAIN 2"), "FX again: CHAIN 2");
        tap(B_FX);
        check(str_eq(pg_title(), "FX MIX") && !page_desc(cur_page(), 0, &(int16_t *){0}),
              "FX again: FX MIX (1.7: one knob per effect of the chain; none yet)");
        tap(B_FX);
        check(str_eq(pg_title(), "FX CHAIN"), "FX again: back to FX CHAIN (no effect: no page of one)");
        turn(EN_K1, FXI_DRIVE);
        turn(EN_K2, FXI_PHASER);
        turn(EN_K3, FXI_FILTER);
        turn(EN_K4, FXI_DELAY);
        check(sx_get(part, SX_C1) == FXI_DRIVE && TSEL->p[P_DIST] > 0, "SLOT1 DRIVE: in, at a usable drive");
        check(TSEL->p[P_ITYPE] == IT_PHASER && TSEL->p[P_IMIX] > 0, "SLOT2 PHASR: the INSERT as a PHASER");
        check(TSEL->p[P_TFLT] != 0 && TSEL->p[P_DLY] > 0, "SLOT3 FILTR (the DJ filter), SLOT4 DELAY: on");
        shot("synth-chain");
        tap(B_FX); tap(B_FX);
        {   /* 1.7: FX MIX follows the chain: DRIVE's MIX, PHASER's, FILTER's, DELAY's send */
            int16_t *v0, *v3;
            const param_desc_t *d0 = page_desc(cur_page(), 0, &v0), *d3 = page_desc(cur_page(), 3, &v3);
            check(str_eq(pg_title(), "FX MIX") && d0 && str_eq(d0->label, "DRIVE") && d3 && str_eq(d3->label, "DELAY") &&
                  v3 == &TSEL->p[P_DLY], "1.7: FX MIX: the chain's effects in its order, DELAY's knob its send");
            turn(EN_K1, -5);
            check(sx_get(part, SX_MXD) == 10, "1.7: FX MIX KNOB 1: DRIVE's MIX");
            turn(EN_K1, 5);
            turn(EN_K4, 6);
            check(TSEL->p[P_DLY] > 40, "1.7: FX MIX KNOB 4: DELAY's send");
            ui.force = 1; frame(); shot("synth-fx-mix");
        }
        tap(B_FX);
        check(str_eq(pg_title(), "DRIVE"), "FX: then DRIVE (FX MIX 2 hidden: four effects)");
        turn(EN_K1, 30);
        turn(EN_K2, 3);
        turn(EN_K4, 1);
        check(TSEL->p[P_DIST] > 60 && field(WV_DSTY) == 3 && field(WV_PUN) == 1, "DRIVE: KNOB 1 the drive, KNOB 2 style T, KNOB 4 PUNISH");
        shot("synth-drive");
        tap(B_FX);
        check(str_eq(pg_title(), "PHASER"), "FX again: PHASER");
        shot("synth-phaser");
        tap(B_FX);
        check(str_eq(pg_title(), "DJ FILT"), "FX again: DJ FILT (in the chain)");
        tap(B_FX);
        check(str_eq(pg_title(), "DELAY"), "FX again: DELAY (CHORUS hidden)");
        turn(EN_K2, -5);
        check(song.g[G_DTIME] == 9 && !strcmp(GP[G_DTIME].names[9], "1/2"),
              "DELAY: KNOB 2 five to the left: 1/8 1/4D 1/4T 1/4 1/2T 1/2 (1.4: musical order)");
        turn(EN_K2, 1);
        check(song.g[G_DTIME] == 10, "DELAY: TIME 1/2T");
        {   /* 1.4: KNOB 4 the delay's MODE: MS keeps the time in ms, KNOB 2 then the ms; SYNC back to the note value */
            uint32_t want = dly_samples(10) * 10u / 441u;
            int16_t *vp;
            turn(EN_K4, 1);
            snprintf(b, sizeof b, "DELAY: KNOB 4 MODE MS: %d ms, the 1/2T at %d BPM kept", song.g[G_DMS], song.g[G_BPM]);
            check(song.g[G_DTIME] == (int16_t)DLY_FREE && song.g[G_DMS] == (int16_t)want, b);
            check(page_desc(cur_page(), 1, &vp) == &GP[G_DMS] && vp == &song.g[G_DMS] &&
                  page_desc(cur_page(), 3, &vp) == &GP[G_DFIX] && vp && *vp == 1, "MODE MS: KNOB 2 is the time in ms, MODE shows MS");
            song.g[G_BPM] = (int16_t)(song.g[G_BPM] + 30);
            check(delay_samples() == want * 441u / 10u, "MODE MS: the tempo changes, the delay's time does not");
            song.g[G_BPM] = (int16_t)(song.g[G_BPM] - 30);
            turn(EN_K2, 3);
            check(song.g[G_DMS] > (int16_t)want && song.g[G_DTIME] == (int16_t)DLY_FREE, "MODE MS: KNOB 2 turns the ms");
            shot("synth-delay-ms");
            turn(EN_K4, -1);
            check(song.g[G_DTIME] == 10, "MODE SYNC: back to 1/2T");
        }
        shot("synth-delay");
        tap(B_FX);
        check(str_eq(pg_title(), "DELAY 2"), "FX again: DELAY 2");
        tap(B_FX);
        check(str_eq(pg_title(), "FX CHAIN"), "FX again: round to FX CHAIN (REVERB hidden)");
        fm1_in.notes |= 1u << 14;
        frame();
        {
            int32_t pk = peak_of(400);
            snprintf(b, sizeof b, "drive, phaser, transient and delay on: a key sounds (peak %d), bounded", pk);
            check(pk > 1000 && pk < 2000000, b);
        }
        fm1_in.notes &= ~(1u << 14);
        frames(80);
        turn(EN_K1, -FXI_DRIVE);
        check(!sx_get(part, SX_C1) && !TSEL->p[P_DIST], "SLOT1 back to --: DRIVE out, off");
        turn(EN_K4, -FXI_DELAY);
        check(!TSEL->p[P_DLY], "SLOT4 --: DELAY off");
        {
            uint32_t pg = 0;
            for (i = 0; i < NPAGES; i++)
                if (PAGES[i].fam == FAM_SYN && str_eq(PAGES[i].title, "DRIVE"))
                    pg = i;
            check(!page_shown(&PAGES[pg]), "DRIVE's page hidden again");
        }
        TSEL->p[P_CHOR] = 30;                           /* (an effect turned on elsewhere: the chain shows it) */
        frames(2);
        check(sx_chain_has(part, FXI_CHORUS), "CHORUS turned on outside the chain: it joins the chain");
        turn(EN_K2, -FXI_PHASER);
        frames(2);
        check(!TSEL->p[P_ITYPE] && sx_chain_has(part, FXI_CHORUS) && TSEL->p[P_CHOR] == 30,
              "SLOT2 --: the INSERT off; CHORUS took SLOT1, at its amount");
        {
            uint32_t n2 = 0;
            for (i = 1; i < FXI_N; i++)
                n2 += sx_chain_has(part, i) != 0;
            check(n2 == 2u, "the chain: CHORUS and FILTR");
        }
        {   /* 1.5: the EQ: in the chain its two pages, the curve; a band's GAIN changes the sound */
            int32_t p0, p1, p2;
            uint32_t sig;
            turn(EN_K2, FXI_EQ);
            check(sx_get(part, SX_C2) == FXI_EQ && sx_get(part, SX_EF1) == 24 && sx_get(part, SX_EG1) == 0,
                  "SLOT2 EQ: in, flat, LOW at 100 Hz");
            tap(B_FX); tap(B_FX); tap(B_FX); tap(B_FX);
            check(str_eq(pg_title(), "EQ 1") && cur_page()->graph == GR_EQ,
                  "FX four times: CHAIN 2, FX MIX, DJ FILT, EQ 1 (its curve)");
            shot("synth-eq-flat");
            fm1_in.notes |= 1u << 12;
            frames(4);
            p0 = peak_of(200);
            sig = graph_signature();
            turn(EN_K1, 127);
            turn(EN_K2, 15);
            snprintf(b, sizeof b, "EQ 1: KNOB 1 LOW's FREQ (%d: 16 kHz), KNOB 2 its GAIN (%d dB), the curve redrawn",
                     sx_get(part, SX_EF1), sx_get(part, SX_EG1));
            check(sx_get(part, SX_EF1) == 127 && sx_get(part, SX_EG1) == 15 && graph_signature() != sig, b);
            frames(4);
            p1 = peak_of(200);
            turn(EN_K2, -30);
            frames(4);
            p2 = peak_of(200);
            snprintf(b, sizeof b, "EQ LOW up to 16 kHz: peak %d flat, %d at +15 dB (the limiter), %d at -15 dB", p0, p1, p2);
            check(p1 >= p0 && p2 * 4 < p0, b);
            turn(EN_K2, 15);
            turn(EN_K3, 40 - sx_get(part, SX_EF2));
            turn(EN_K4, 9);
            shot("synth-eq");
            tap(B_FX);
            check(str_eq(pg_title(), "EQ 2"), "FX again: EQ 2 (MID 2, HIGH)");
            turn(EN_K4, -8);
            shot("synth-eq2");
            fm1_in.notes &= ~(1u << 12);
            frames(40);
            sx_set(part, SX_C2, 0);                     /* (out of the chain again: its pages hidden) */
            frames(2);
        }
    }
    tap(B_EDIT);
    {   /* SELECT walks every synth page shown, stopping at the ends */
        uint32_t n = 1;
        for (i = 0; i < 40u; i++) {
            uint32_t was = ui.page;
            turn(EN_SELECT, 1);
            n += ui.page != was;
        }
        snprintf(b, sizeof b, "SELECT walks every synth page shown (%u), last PRESET (1.7)", n);
        check(n == 23u && str_eq(pg_title(), "PRESET") && on_syn_page(), b);
        for (i = 0; i < 40u; i++)
            turn(EN_SELECT, -1);
        check(str_eq(pg_title(), "OSC 1"), "SELECT back to OSC 1");
    }
    {   /* 1.2: REC + SAVE held 2 s: the synth's sound back to its start, its notes kept */
        uint32_t part = (uint32_t)(TSEL - trk), rec0 = song.rec;
        step_t st0 = TSEL->step[0];
        TSEL->step[0].n = 1, TSEL->step[0].note[0] = 64, TSEL->step[0].vel = 100;
        st0 = TSEL->step[0];
        press(B_SAVE);
        frames(3);
        press(B_REC);
        frames(60);
        release(B_REC);
        release(B_SAVE);
        frames(3);
        check(sx_chain_has(part, FXI_CHORUS) && song.rec == rec0, "REC + SAVE let go after 1 s: nothing cleared, REC as it was");
        press(B_SAVE);
        frames(3);
        press(B_REC);
        frames(60);
        check(ui.hold_kind == 3u, "REC + SAVE held in the SYNTH MODE: the clear ring");
        shot("synth-clear-hold");
        frames(100);
        release(B_REC);
        release(B_SAVE);
        frames(3);
        check(sx_chain_has(part, FXI_CHORUS) && !TSEL->p[P_CHOR] && !TSEL->p[P_TFLT] &&   /* (1.6: the chain is the
                                                         * project's: kept; this track's amounts 0) */
              sx_get(part, SX_A2) == TSEL->p[P_ATK] && sx_get(part, SX_R3) == TSEL->p[P_REL] && !sx_get(part, SX_LD1) &&
              TSEL->eng_req == ENGI_WAVE && TSEL->preset == WAVE_SINE,
              "REC + SAVE held 2 s: the synth back to its first sound, its effects off (the project's chain kept), no LFO, the ENVs as the sound's");
        snprintf(b, sizeof b, "... its notes kept (%d), REC as it was (%u %u), OSC 1 (%s)", !memcmp(&TSEL->step[0], &st0, sizeof st0),
                 (unsigned)song.rec, (unsigned)rec0, pg_title());
        check(!memcmp(&TSEL->step[0], &st0, sizeof st0) && song.rec == rec0 && syn_mode && str_eq(pg_title(), "OSC 1"), b);
    }
    {   /* PRESETS: WAVE's sounds */
        uint32_t p0 = TSEL->preset;
        turn(EN_PRESET, 1);
        check(TSEL->eng_req == ENGI_WAVE && TSEL->preset == (p0 + 1u) % ENG_WAVE.npresets, "PRESETS: the next WAVE sound");
        for (i = 0; i < ENG_WAVE.npresets; i++)
            if (str_eq(ENG_WAVE.presets[TSEL->preset].name, "REV SWELL"))
                break;
            else
                turn(EN_PRESET, 1);
        check(str_eq(ENG_WAVE.presets[TSEL->preset].name, "REV SWELL") && field(WV_R1) == 1, "REV SWELL: OSC 1 backwards");
        frames(6);
        shot("synth-rev-swell");
    }

    /* the mode keeps the screen */
    {
        static const uint8_t OTHER[4] = {B_ARP, B_SEQ, B_SCL, B_GLO};
        static const char *const ON[4] = {"ARP", "SEQ", "SEL", "GLO"};
        tap(B_LFO);
        for (i = 0; i < 4u; i++) {
            tap(OTHER[i]);
            frames(2);
            snprintf(b, sizeof b, "%s tapped: still the synth (LFO page), the mode on", ON[i]);
            check(syn_mode && on_syn_page() && str_eq(pg_title(), "LFO"), b);
        }
        tap(B_SAVE);
        frames(2);
        check(syn_mode && on_syn_page() && str_eq(pg_title(), "PRESET") && cur_page()->graph == GR_USER &&
              cur_btn() == B_SAVE, "1.7: SAVE tapped: the synth's PRESET page (SLOT LOAD ERASE SAVE), SAVE lit, the mode on");
        tap(B_LFO);
        tap(B_HOME);
        frames(2);
        check(syn_mode && on_syn_page() && str_eq(pg_title(), "OSC 1"), "HOME tapped: OSC 1, the mode on");
        go_home();                                      /* (something else takes the screen: a load, the editor) */
        frames(2);
        check(syn_mode && on_syn_page(), "the TRACKS screen taken: back to the synth");
        for (i = 0; i < 6u; i++)
            turn(EN_ALGO, 1);
        check(song.sel == NPART - 1u && on_syn_page(), "ALGORITHM: the synth tracks only (not the drums)");
        for (i = 0; i < 6u; i++)
            turn(EN_ALGO, -1);
        check(song.sel == 0, "ALGORITHM back to track 1");
        shot("synth-track1-not-wave");
        turn(EN_PRESET, 1);
        check(TSEL->eng_req == ENGI_WAVE, "track 1: PRESETS gives it a WAVE sound");
        track_select(1);
        frames(2);
        press(B_SEQ);                                   /* held: its layer, as ever; let go: the synth again */
        frames(40);
        check(ui.layer == LY_STEP, "SEQ held: the steps layer");
        release(B_SEQ);
        frames(3);
        check(syn_mode && on_syn_page(), "let go: the synth again");
    }

    /* off */
    hold_env();
    check(!syn_mode && !ui.home && cur_page()->fam == FAM_ENV, "ENV held again: SYNTH MODE off, SLOOP's ENV page");
    check(TSEL->eng_req == ENGI_WAVE, "the sound stays");
    tap(B_EDIT);
    check(!on_syn_page() && cur_page()->fam == FAM_EDIT, "off: EDIT opens SLOOP's EDIT pages");
    {
        int16_t *vp;
        const param_desc_t *d = page_desc(cur_page(), 0, &vp);
        check(d && str_eq(d->label, "WAVE") && vp && *vp == field(WV_W1), "SLOOP's EDIT on WAVE: OSC 1's WAVE, not a word");
    }
    tap(B_SEQ);
    check(!on_syn_page(), "off: SEQ opens SLOOP's pages again");
    tap(B_HOME);
    frames(2);
    check(!on_syn_page() && !syn_mode, "off: HOME the TRACKS screen");

    /* from the drum track: track 1 */
    track_select(TRK_DRUM);
    frames(2);
    hold_env();
    check(syn_mode && song.sel == 0 && trk[0].eng_req == ENGI_WAVE && on_syn_page(), "drums selected: the mode takes track 1");
    hold_env();
    check(!syn_mode, "and off again");
    {   /* with the menu open, ENV held does nothing */
        ui.menu = 1;
        hold_env();
        check(!syn_mode, "menu open: ENV held does not start the mode");
        ui.menu = 0;
        ui.force = 1;
        frames(2);
    }
    {   /* 1.6: the FX chain is the project's: set from track 2, the same on track 1 and 3 */
        sx_set(1, SX_C1, FXI_DRIVE);
        check(sx_get(0, SX_C1) == FXI_DRIVE && sx_get(2, SX_C1) == FXI_DRIVE && sx_chain_has(2, FXI_DRIVE),
              "1.6: the FX chain is the project's (set on track 2, seen on tracks 1 and 3)");
        set_engine_of(&trk[0], ENGI_WAVE);
        apply_preset_to(&trk[0], WAVE_SINE);
        check(sx_get(0, SX_C1) == FXI_DRIVE, "1.6: a sound loaded on track 1 leaves the project's chain as it was");
        sx_set(1, SX_C1, 0);
    }
    {   /* 1.6: DRIVE's MIX on a block (fx.c track_fx_run): 15 the drive, 0 the dry back exactly, 7 between */
        int32_t blk[CTL], in[CTL], d15 = 0, d0 = 0, d7 = 0;
        uint32_t i;
        track_t *t2 = &trk[1];
        t2->p[P_DIST] = 127;
        for (i = 0; i < CTL; i++)
            in[i] = (int32_t)(sine_i((uint32_t)i << 27) / 2);
        sx_set(1, SX_MXD, 15);
        memcpy(blk, in, sizeof blk);
        track_fx_run(t2, blk, CTL, FXI_DRIVE);
        for (i = 0; i < CTL; i++)
            d15 += blk[i] != in[i];
        sx_set(1, SX_MXD, 0);
        memcpy(blk, in, sizeof blk);
        track_fx_run(t2, blk, CTL, FXI_DRIVE);
        for (i = 0; i < CTL; i++)
            d0 += blk[i] != in[i];
        sx_set(1, SX_MXD, 7);
        memcpy(blk, in, sizeof blk);
        track_fx_run(t2, blk, CTL, FXI_DRIVE);
        for (i = 0; i < CTL; i++)
            d7 += blk[i] != in[i];
        snprintf(b, sizeof b, "1.6: DRIVE's MIX: 100%% changes %d of %d samples, 0%% %d (the dry), 47%% %d", d15, CTL, d0, d7);
        check(d15 > CTL / 2 && !d0 && d7 > CTL / 2, b);
        sx_set(1, SX_MXD, 15);
        t2->p[P_DIST] = 0;
    }
    {   /* 1.6: SAVE > TOOLS COPY, TOOLS 2 PASTE: a track's pattern into another, the drums apart; undo brings it back */
        int same, kept;
        track_select(0);
        frames(2);
        steps_clear(&trk[0]);
        steps_clear(&trk[2]);
        trk[0].step[0].n = 1, trk[0].step[0].note[0] = 60, trk[0].step[0].time = ST_NOTE, trk[0].step[0].vel = 100;
        trk[0].step[5].n = 1, trk[0].step[5].note[0] = 67, trk[0].step[5].time = ST_NOTE, trk[0].step[5].vel = 90;
        trk[0].micro[5] = 9;
        trk[0].p[P_SLEN] = 32;
        pat_clip(0);
        track_select(2);
        frames(2);
        pat_clip(1);
        same = !memcmp(trk[2].step, trk[0].step, sizeof trk[0].step) && trk[2].micro[5] == 9 && trk[2].p[P_SLEN] == 32;
        check(same && str_eq(ui.msg, "PASTED TO TRACK 3"), "1.6: COPY on track 1, PASTE on track 3: its steps, nudges, LEN");
        track_select(TRK_DRUM);
        frames(2);
        pat_clip(1);
        check(str_eq(ui.msg, "DRUMS / SYNTH: NO"), "1.6: a synth pattern is not pasted on the drums");
        track_select(2);
        frames(2);
        press(B_EDIT);
        frames(10);
        fm1_in.buttons |= BT(B_OCTDN); edges_btn |= BT(B_OCTDN); frame();
        fm1_in.buttons &= ~BT(B_OCTDN); frame();
        release(B_EDIT);
        frames(4);
        kept = trk[2].step[0].n == 0 && trk[2].step[5].n == 0;
        check(kept, "1.6: EDIT + OCT- after a PASTE: the track as it was");
    }
    {   /* 1.7: an EQ GAIN (or a MIX) turned through 1..6 takes no effect in or out of the chain (1.5 .. 1.6 did) */
        int16_t *vp;
        track_t *t = &trk[0];
        int16_t flt = t->p[P_TFLT], dly = t->p[P_DLY], rev = t->p[P_REV];
        set_engine_of(t, ENGI_WAVE);
        t->engine = t->eng_req;
        wv_desc(t, VX0 + SX_EG2, &vp);
        *vp = 3;                                        /* (FXI_FILTER's number) */
        wv_commit(t, vp);
        wv_desc(t, VX0 + SX_MXD, &vp);
        *vp = 5;                                        /* (FXI_DELAY's) */
        wv_commit(t, vp);
        check(t->p[P_TFLT] == flt && t->p[P_DLY] == dly && t->p[P_REV] == rev && sx_get(0, SX_EG2) == 3,
              "1.7: EQ GAIN 3, DRIVE MIX 5: their values only, no effect in or out");
        sx_set(0, SX_EG2, 0), sx_set(0, SX_MXD, 15);
    }
    {   /* 1.7: FX > COMP's gain: full scale at THRS -30 dB, 10:1, held: -27 dB, + 13.5 of make-up; quiet: the make-up */
        static int32_t l[CTL], r[CTL];
        int32_t k, j, hi, lo;
        song.g[G_KTHR] = -30, song.g[G_KRAT] = 2, song.g[G_KATK] = 0, song.g[G_KREL] = 0;
        for (k = 0; k < 400; k++) {
            for (j = 0; j < CTL; j++)
                l[j] = r[j] = 32768;
            comp_process(l, r, CTL);
        }
        hi = l[CTL - 1];
        for (k = 0; k < 2000; k++) {
            for (j = 0; j < CTL; j++)
                l[j] = r[j] = 328;
            comp_process(l, r, CTL);
        }
        lo = l[CTL - 1];
        snprintf(b, sizeof b, "1.7: COMP THRS -30 10:1: full scale out at %d (~6900), -40 dB out at %d (~1550)", hi, lo);
        check(hi > 6200 && hi < 7600 && lo > 1400 && lo < 1700, b);
        song.g[G_KTHR] = 0;
        for (j = 0; j < CTL; j++)
            l[j] = r[j] = 12345;
        comp_process(l, r, CTL);
        check(l[0] == 12345 && r[CTL - 1] == 12345 && kmp.g == 4096, "1.7: COMP THRS 0 dB: off, the mix bit for bit");
        song.g[G_KATK] = GP[G_KATK].def, song.g[G_KREL] = GP[G_KREL].def, song.g[G_KRAT] = GP[G_KRAT].def;
    }
    {   /* 1.7: FX > EQ, the master's: flat nothing; LOW -15 dB under a 60 Hz sine: it drops */
        static int32_t l[CTL], r[CTL];
        int32_t k, j, pk = 0;
        uint32_t ph = 0;
        for (j = 0; j < CTL; j++)
            l[j] = r[j] = 777;
        eq_master(l, r, CTL);
        check(l[0] == 777 && r[CTL - 1] == 777, "1.7: the master's EQ flat: the mix bit for bit");
        sx_set(NPART, SX_EF1, 24), sx_set(NPART, SX_EG1, -15);
        for (k = 0; k < 300; k++) {
            for (j = 0; j < CTL; j++, ph += 5843626u)       /* (60 Hz) */
                l[j] = r[j] = osc_sine(ph);
            eq_master(l, r, CTL);
            for (j = 0; k > 200 && j < CTL; j++)
                pk = l[j] > pk ? l[j] : pk;
        }
        snprintf(b, sizeof b, "1.7: the master's EQ, LOW -15 dB: a 60 Hz sine of 32767 out at %d", pk);
        check(pk > 3000 && pk < 9000, b);
        sx_set(NPART, SX_EG1, 0);
    }
    {   /* 1.7: FX > MIXER on the drum track: DST and MOD its own, DLY and REV the drums' sends (GLO > DRUMS) */
        const page_t *mx = 0;
        int16_t *vp;
        for (i = 0; i < NPAGES; i++)
            if (PAGES[i].fam == FAM_FX && str_eq(PAGES[i].title, "MIXER"))
                mx = &PAGES[i];
        track_select(TRK_DRUM);
        frames(2);
        check(mx && page_for_drum(mx) && page_desc(mx, 2, &vp) == &GP[G_DRCLIP] && vp == &song.g[G_DRCLIP] &&
              page_desc(mx, 3, &vp) == &GP[G_DRREV] && vp == &song.g[G_DRREV] && page_desc(mx, 1, &vp) &&
              vp == &TDRUM->p[P_CHOR], "1.7: the drums' MIXER: DST, MOD theirs, CLIP (no DLY), REV the GLO > DRUMS send");
        for (i = 0; i < NPAGES; i++)
            if (PAGES[i].fam == FAM_FX && str_eq(PAGES[i].title, "DIST"))
                mx = &PAGES[i];
        check(page_desc(mx, 0, &vp) == &GP[G_DRSTY] && page_desc(mx, 2, &vp) == &GP[G_DRPUN] && !page_desc(mx, 3, &vp),
              "1.7: FX > DIST on the drums: their DRIVE's STYLE, TONE, PUNSH");
        {   /* the drums' DST (the DRIVE, in parallel) and CLIP each change the kick, bounded; 0 and 0: bit for bit */
            static int32_t l[CTL], r[CTL], l0[CTL];
            int32_t j, same = 1, chg = 0, clip = 0;
            uint32_t ph = 0;
            for (j = 0; j < CTL; j++, ph += 40000000u)
                l0[j] = osc_sine(ph) >> 1;
            memcpy(l, l0, sizeof l), memcpy(r, l0, sizeof r);
            drum_fx(l, r, CTL);
            same = !memcmp(l, l0, sizeof l);
            TDRUM->p[P_DIST] = 100, song.g[G_DRSTY] = 3;
            memcpy(l, l0, sizeof l), memcpy(r, l0, sizeof r);
            drum_fx(l, r, CTL);
            for (j = 0; j < CTL; j++)
                chg |= l[j] != l0[j];
            TDRUM->p[P_DIST] = 0;
            drum_fx(l, r, CTL);
            song.g[G_DRCLIP] = 127;
            for (j = 0; j < CTL; j++)
                l[j] = r[j] = 60000;
            drum_fx(l, r, CTL);
            clip = l[CTL - 1];
            song.g[G_DRCLIP] = 0, song.g[G_DRSTY] = 0;
            snprintf(b, sizeof b, "1.7: drums' DST 0 CLIP 0 dry; DST (T) changes them; CLIP 127 rounds 60000 to %d", clip);
            check(same && chg && clip > 5000 && clip < 12000, b);
        }
        track_select(0);
        frames(2);
    }
    {   /* 1.7: the parallel DIST on SLOOP's engines; FX > MOD a flanger, a phaser: each its own sound, bounded */
        track_t *t = &trk[2];
        int32_t p0, p1, pc, pf, pp;
        set_engine_of(t, 0);
        t->engine = t->eng_req;
        apply_preset_to(t, 0);
        t->p[P_DIST] = 0, t->p[P_CHOR] = 0;
        track_select(2);
        frames(4);
        fm1_in.notes |= 1u << 12;
        frames(6);
        p0 = peak_of(100);
        song.g[G_XTYPE] = 2, song.g[G_XA] = 120;       /* FOLD, driven */
        t->p[P_DIST] = 127;
        frames(6);
        p1 = peak_of(100);
        t->p[P_DIST] = 0;
        t->p[P_CHOR] = 127;
        frames(6);
        pc = peak_of(100);
        song.g[G_MTYPE] = 1;
        frames(6);
        pf = peak_of(100);
        song.g[G_MTYPE] = 2;
        frames(6);
        pp = peak_of(100);
        fm1_in.notes &= ~(1u << 12);
        frames(60);
        snprintf(b, sizeof b, "1.7: DST 127 FOLD: peak %d (dry %d); MOD CHOR %d, FLANG %d, PHASR %d", p1, p0, pc, pf, pp);
        check(p1 != p0 && pc != pf && pf != pp && pf > 1000 && pp > 1000 && pf < 2000000 && pp < 2000000, b);
        song.g[G_MTYPE] = 0, song.g[G_XTYPE] = 0, song.g[G_XA] = GP[G_XA].def;
        t->p[P_CHOR] = 0;
        track_select(0);
        frames(2);
    }
    printf(fails ? "synth mode: %d FAILED\n" : "synth mode: PASS\n", fails);
    return fails != 0;
}
