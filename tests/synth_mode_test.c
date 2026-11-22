/* SPDX-License-Identifier: GPL-3.0-only */
/* BONBLOOP 1.1: the SYNTH MODE and the WAVE engine on the real UI and audio (the ui_pages_test harness).
 *   ENV held 0.7 s: the mode on; the selected track plays WAVE, OSC 1 opens; again: off, the ENV page
 *   EDIT / ENV / LFO / FX tapped: their synth pages (again: the next of the group); SELECT walks all 18
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
    tap(B_FX);
    check(on_syn_page() && str_eq(pg_title(), "FILTER") && cur_btn() == B_FX, "FX tapped: FILTER, FX lit");
    turn(EN_K3, 1);
    check(TSEL->p[P_FTYPE] == 1, "KNOB 3 on FILTER: TYPE BP");
    turn(EN_K3, -1);
    shot("synth-filter");
    tap(B_FX);
    check(on_syn_page() && str_eq(pg_title(), "DRIVE"), "FX again: DRIVE");
    turn(EN_K1, 30);
    turn(EN_K2, 3);
    check(TSEL->p[P_DIST] > 0 && field(WV_DSTY) == 3, "DRIVE: KNOB 1 the drive, KNOB 2 style T");
    shot("synth-drive");
    tap(B_FX);
    check(on_syn_page() && str_eq(pg_title(), "TRANSNT"), "FX again: TRANSNT");
    turn(EN_K1, 6);
    check(field(WV_TATK) > 0, "TRANSNT: KNOB 1 ATTCK up");
    shot("synth-transient");
    tap(B_FX);
    check(on_syn_page() && str_eq(pg_title(), "PHASER") && TSEL->p[P_ITYPE] == IT_PHASER, "FX again: PHASER (the INSERT, PHASR)");
    turn(EN_K1, 40);
    check(TSEL->p[P_IMIX] > 0, "PHASER: KNOB 1 its MIX");
    shot("synth-phaser");
    fm1_in.notes |= 1u << 14;
    frame();
    {
        int32_t pk = peak_of(400);
        snprintf(b, sizeof b, "drive, transient and phaser on: a key sounds (peak %d), bounded", pk);
        check(pk > 1000 && pk < 2000000, b);
    }
    fm1_in.notes &= ~(1u << 14);
    frames(80);
    tap(B_FX); tap(B_FX);
    check(on_syn_page() && str_eq(pg_title(), "SENDS"), "FX twice more: SENDS (chorus, delay, reverb)");
    tap(B_FX); tap(B_FX); tap(B_FX);
    check(on_syn_page() && str_eq(pg_title(), "FILTER"), "FX round its group: back to FILTER");
    tap(B_EDIT);
    {   /* SELECT walks every synth page, stopping at the ends */
        uint32_t n = 1;
        for (i = 0; i < 30u; i++) {
            uint32_t was = ui.page;
            turn(EN_SELECT, 1);
            n += ui.page != was;
        }
        snprintf(b, sizeof b, "SELECT walks every synth page (%u), last REV/CHO", n);
        check(n == 18u && str_eq(pg_title(), "REV/CHO") && on_syn_page(), b);
        for (i = 0; i < 30u; i++)
            turn(EN_SELECT, -1);
        check(str_eq(pg_title(), "OSC 1"), "SELECT back to OSC 1");
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
