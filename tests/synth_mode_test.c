/* SPDX-License-Identifier: GPL-3.0-only */
/* BONBLOOP 1.1 / 1.2: the SYNTH MODE and the WAVE engine on the real UI and audio (the ui_pages_test harness).
 *   ENV held 0.7 s: the mode on; the selected track plays WAVE, OSC 1 opens; again: off, the ENV page
 *   EDIT / ENV / LFO / FX tapped: their synth pages (again: the next of the group); SELECT walks those shown
 *   1.2: each oscillator's ENV and LFO; FX: the chain (every effect off at first), an effect's pages while it is in
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
    check(on_syn_page() && str_eq(pg_title(), "OSC 1"), "OSC 1 opens");
    shot("synth-splash");
    check(cur_btn() == B_EDIT, "OSC 1's light: EDIT");
    frames(60);
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
    check(on_syn_page() && str_eq(pg_title(), "ENV") && cur_btn() == B_ENV, "ENV tapped: the synth ENV page, ENV lit");
    tap(B_ENV);
    check(on_syn_page() && str_eq(pg_title(), "ENV DEST"), "ENV again: ENV DEST");
    {
        int16_t *vp;
        const param_desc_t *d = page_desc(cur_page(), 2, &vp);
        check(d && str_eq(d->label, "POS") && vp == &TSEL->p[P_ED_SHP], "ENV DEST: SHP reads POS on WAVE");
    }
    tap(B_LFO);
    check(on_syn_page() && str_eq(pg_title(), "LFO") && cur_btn() == B_LFO, "LFO tapped: the synth LFO page, LFO lit");
    {   /* 1.2: each oscillator's ENV */
        uint32_t part = (uint32_t)(TSEL - trk);
        tap(B_ENV); tap(B_ENV); tap(B_ENV);
        check(on_syn_page() && str_eq(pg_title(), "OSC1 ENV"), "ENV, ENV DEST, then OSC1 ENV");
        turn(EN_K1, 1);
        turn(EN_K2, 3);
        turn(EN_K3, 5);
        turn(EN_K4, -10);
        check(sx_get(part, SX_EON1) == 1 && sx_get(part, SX_EA1) == 3 && sx_get(part, SX_ED1) == 5 &&
              sx_get(part, SX_ES1) == 5 && TSEL->p[P_ATK] == ENG_WAVE.presets[TSEL->preset].env[0],
              "OSC1 ENV: ON, ATK 3, DEC 5, SUS 5: its own, the voice's ENV as it was");
        shot("synth-osc1-env");
        tap(B_ENV);
        check(on_syn_page() && str_eq(pg_title(), "OSC2 ENV"), "ENV again: OSC2 ENV");
        tap(B_ENV); tap(B_ENV);
        check(on_syn_page() && str_eq(pg_title(), "ENV"), "ENV round its group: ENV");
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
              !TSEL->p[P_TFLT] && !field(WV_TATK) && !field(WV_TSUS) && !sx_chain_has(part, FXI_DRIVE),
              "the synth's sound: every effect off, the chain empty");
        tap(B_FX);
        check(on_syn_page() && str_eq(pg_title(), "FX CHAIN") && cur_btn() == B_FX, "FX tapped: FX CHAIN, FX lit");
        shot("synth-chain-empty");
        tap(B_FX);
        check(str_eq(pg_title(), "CHAIN 2"), "FX again: CHAIN 2");
        tap(B_FX);
        check(str_eq(pg_title(), "FX CHAIN"), "FX again: back to FX CHAIN (no effect: no page of one)");
        turn(EN_K1, FXI_DRIVE);
        turn(EN_K2, FXI_PHASER);
        turn(EN_K3, FXI_TRANS);
        turn(EN_K4, FXI_DELAY);
        check(sx_get(part, SX_C1) == FXI_DRIVE && TSEL->p[P_DIST] > 0, "SLOT1 DRIVE: in, at a usable drive");
        check(TSEL->p[P_ITYPE] == IT_PHASER && TSEL->p[P_IMIX] > 0, "SLOT2 PHASR: the INSERT as a PHASER");
        check(field(WV_TATK) > 0 && TSEL->p[P_DLY] > 0, "SLOT3 TRANS, SLOT4 DELAY: on");
        shot("synth-chain");
        tap(B_FX); tap(B_FX);
        check(str_eq(pg_title(), "DRIVE"), "FX twice: CHAIN 2, then DRIVE (in the chain)");
        turn(EN_K1, 30);
        turn(EN_K2, 3);
        turn(EN_K4, 1);
        check(TSEL->p[P_DIST] > 60 && field(WV_DSTY) == 3 && field(WV_PUN) == 1, "DRIVE: KNOB 1 the drive, KNOB 2 style T, KNOB 4 PUNISH");
        shot("synth-drive");
        tap(B_FX);
        check(str_eq(pg_title(), "PHASER"), "FX again: PHASER");
        shot("synth-phaser");
        tap(B_FX);
        check(str_eq(pg_title(), "TRANSNT"), "FX again: TRANSNT (DJ FILT is not in the chain: hidden)");
        shot("synth-transient");
        tap(B_FX);
        check(str_eq(pg_title(), "DELAY"), "FX again: DELAY (CHORUS hidden)");
        turn(EN_K2, 9 - song.g[G_DTIME]);
        check(song.g[G_DTIME] == 9 && !strcmp(GP[G_DTIME].names[9], "1/2"), "DELAY: KNOB 2 TIME 1/2");
        turn(EN_K2, 1);
        check(song.g[G_DTIME] == 10, "DELAY: TIME 1/2T");
        shot("synth-delay");
        tap(B_FX);
        check(str_eq(pg_title(), "DELAY 2"), "FX again: DELAY 2");
        {
            uint32_t want = dly_samples(10) * 10u / 441u;
            turn(EN_K3, 1);
            snprintf(b, sizeof b, "DELAY 2: KNOB 3 FIX: FREE at %d ms (1/2T at %d BPM)", song.g[G_DMS], song.g[G_BPM]);
            check(song.g[G_DTIME] == (int16_t)DLY_FREE && song.g[G_DMS] == (int16_t)want && !song.g[G_DFIX], b);
            song.g[G_BPM] = (int16_t)(song.g[G_BPM] + 30);
            check(delay_samples() == want * 441u / 10u, "FIXED: the tempo changes, the delay's time does not");
            song.g[G_BPM] = (int16_t)(song.g[G_BPM] - 30);
        }
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
            check(n2 == 2u, "the chain: CHORUS and TRANS");
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
        snprintf(b, sizeof b, "SELECT walks every synth page shown (%u), last CHORUS", n);
        check(n == 21u && str_eq(pg_title(), "CHORUS") && on_syn_page(), b);
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
        check(!sx_chain_has(part, FXI_CHORUS) && !sx_chain_has(part, FXI_TRANS) && !TSEL->p[P_CHOR] && !field(WV_TATK) &&
              !sx_get(part, SX_EON1) && !sx_get(part, SX_LD1) && TSEL->eng_req == ENGI_WAVE && TSEL->preset == 0u,
              "REC + SAVE held 2 s: the synth back to its first sound, every effect off, no ENV / LFO of the oscillators");
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
        static const uint8_t OTHER[5] = {B_ARP, B_SEQ, B_SCL, B_GLO, B_SAVE};
        static const char *const ON[5] = {"ARP", "SEQ", "SEL", "GLO", "SAVE"};
        tap(B_LFO);
        for (i = 0; i < 5u; i++) {
            tap(OTHER[i]);
            frames(2);
            snprintf(b, sizeof b, "%s tapped: still the synth (LFO page), the mode on", ON[i]);
            check(syn_mode && on_syn_page() && str_eq(pg_title(), "LFO"), b);
        }
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
    printf(fails ? "synth mode: %d FAILED\n" : "synth mode: PASS\n", fails);
    return fails != 0;
}
