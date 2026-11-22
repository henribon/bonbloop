/* SPDX-License-Identifier: GPL-3.0-only */
/* BONBLOOP: the SYNTH MODE and the WAVE engine on the real UI and audio (the ui_pages_test harness).
 *   ENV held 0.7 s: the mode on; the selected track plays WAVE, the wavetable page opens; again: off, the ENV page
 *   EDIT / ENV / LFO / FX tapped: their synth pages (again: the next of the group); SELECT walks every synth page
 *   PRESETS on a synth page: WAVE's sounds only; KNOB 2 moves POS; ENV / LFO > SHP read POS on WAVE
 *   HOME: the TRACKS screen, the mode stays; the drums selected: the mode takes track 1
 *   the table: built in the arena within ~50 ms of the switch, every TABL; a note sounds, bounded
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

int main(int argc, char **argv)
{
    uint32_t i, tab;
    char b[96];
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
    ui.force = 1;
    frame();
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
    check(on_syn_page() && str_eq(pg_title(), "WAVE"), "the wavetable page opens");
    ui.force = 1;
    frame();
    ppm("synth-splash");
    check(cur_btn() == B_EDIT, "the WAVE page's light: EDIT");
    frames(60);
    check(TSEL->engine == ENGI_WAVE && wt_peek(TSEL) && wt_peek(TSEL)->ready, "its table built in the arena");
    ui.force = 1;
    frame();
    ppm("synth-wave");

    /* a note: it sounds, bounded; POS moved by LFO > SHP shows in the graph */
    TSEL->p[P_LD_SHP] = 50;
    TSEL->p[P_LRATE] = 70;
    fm1_in.notes |= 1u << 12;
    frame();
    {
        int32_t pk = peak_of(400);
        snprintf(b, sizeof b, "a key on the WAVE track sounds (peak %d), bounded", pk);
        check(pk > 1000 && pk < 2000000, b);
    }
    frames(3);
    ui.force = 1;
    frame();
    ppm("synth-wave-playing");
    fm1_in.notes &= ~(1u << 12);
    frames(80);

    /* the buttons: their synth pages */
    tap(B_ENV);
    check(on_syn_page() && str_eq(pg_title(), "ENV") && cur_btn() == B_ENV, "ENV tapped: the synth ENV page, ENV lit");
    ui.force = 1; frame(); ppm("synth-env");
    tap(B_ENV);
    check(on_syn_page() && str_eq(pg_title(), "ENV DEST"), "ENV again: ENV DEST");
    {
        int16_t *vp;
        const param_desc_t *d = page_desc(cur_page(), 2, &vp);
        check(d && str_eq(d->label, "POS") && vp == &TSEL->p[P_ED_SHP], "ENV DEST: SHP reads POS on WAVE");
    }
    tap(B_ENV);
    check(on_syn_page() && str_eq(pg_title(), "ENV"), "ENV a third time: back to ENV (its group)");
    tap(B_LFO);
    check(on_syn_page() && str_eq(pg_title(), "LFO") && cur_btn() == B_LFO, "LFO tapped: the synth LFO page, LFO lit");
    ui.force = 1; frame(); ppm("synth-lfo");
    tap(B_LFO); tap(B_LFO);
    check(on_syn_page() && str_eq(pg_title(), "LFO DEST"), "LFO twice more: LFO DEST");
    tap(B_FX);
    check(on_syn_page() && str_eq(pg_title(), "FX") && cur_btn() == B_FX, "FX tapped: the synth FX page, FX lit");
    ui.force = 1; frame(); ppm("synth-fx");
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "WAVE"), "EDIT tapped: WAVE");
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "OSC"), "EDIT again: OSC");
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "FILTER"), "EDIT again: FILTER");
    ui.force = 1; frame(); ppm("synth-filter");
    tap(B_EDIT);
    check(on_syn_page() && str_eq(pg_title(), "WAVE"), "EDIT again: back to WAVE");
    {   /* SELECT walks every synth page, stopping at the ends */
        uint32_t n = 1;
        for (i = 0; i < 20u; i++) {
            uint32_t was = ui.page;
            turn(EN_SELECT, 1);
            n += ui.page != was;
        }
        snprintf(b, sizeof b, "SELECT walks every synth page (%u), last REV/CHO", n);
        check(n == 13u && str_eq(pg_title(), "REV/CHO") && on_syn_page(), b);
        for (i = 0; i < 20u; i++)
            turn(EN_SELECT, -1);
        check(str_eq(pg_title(), "WAVE"), "SELECT back to WAVE");
    }
    {   /* PRESETS: WAVE's sounds */
        uint32_t p0 = TSEL->preset;
        turn(EN_PRESET, 1);
        check(TSEL->eng_req == ENGI_WAVE && TSEL->preset == (p0 + 1u) % ENG_WAVE.npresets, "PRESETS: the next WAVE sound");
        turn(EN_PRESET, -1);
        turn(EN_PRESET, -1);
        check(TSEL->eng_req == ENGI_WAVE && TSEL->preset == (p0 + ENG_WAVE.npresets - 1u) % ENG_WAVE.npresets,
              "PRESETS back past the first: the last WAVE sound");
    }
    {   /* KNOB 2: POS, KNOB 1: TABL (every table builds) */
        int16_t pos = TSEL->p[P_E1];
        turn(EN_K2, 5);
        check(TSEL->p[P_E1] > pos, "KNOB 2 on WAVE: POS up");
        for (tab = 0; tab < WT_NTAB; tab++) {
            TSEL->p[P_E0] = (int16_t)tab;
            frames(4);
            snprintf(b, sizeof b, "TABL %s: built within 4 frames", N_WTAB[tab]);
            check(wt_peek(TSEL) && wt_peek(TSEL)->ready && wt_peek(TSEL)->want == tab + 1u, b);
            if (tab == WT_VOCAL) {
                ui.force = 1; frame(); ppm("synth-wave-vocal");
            }
        }
    }

    /* HOME: TRACKS, the mode stays */
    tap(B_HOME);
    check(syn_mode && !on_syn_page(), "HOME: the TRACKS screen, the mode stays");
    ui.force = 1; frame(); ppm("synth-tracks");
    tap(B_LFO);
    check(on_syn_page() && str_eq(pg_title(), "LFO"), "then LFO: the synth LFO page again");

    /* off */
    hold_env();
    check(!syn_mode && !ui.home && cur_page()->fam == FAM_ENV, "ENV held again: SYNTH MODE off, SLOOP's ENV page");
    check(TSEL->eng_req == ENGI_WAVE, "the sound stays");
    tap(B_EDIT);
    check(!on_syn_page() && cur_page()->fam == FAM_EDIT, "off: EDIT opens SLOOP's EDIT pages");
    tap(B_ENV);
    check(!on_syn_page() && cur_page()->fam == FAM_ENV, "off: ENV opens SLOOP's ENV page");

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
