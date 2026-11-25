/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Felucca UI drawing: status bar (top), columns + gauges, graphs, focus readout,
 * footer (steps + engine / preset / page). */
static void draw_menu(void);
static uint32_t str_hash(uint32_t h, const char *s);

/* --------------------------------------------------------- drawing --- */
UI_SMALL static int is_eng_name(const char *s)                 /* one of the ENGINES[]->name strings */
{
    uint32_t e;
    for (e = 0; e < NENGINES; e++)
        if (s == ENGINES[e]->name)
            return 1;
    return 0;
}

/* at most 5 characters, and no wider than maxw */
UI_SMALL static void fit(char *d, const char *src, const felucca_font_t *f, int32_t maxw)
{
    str_cpy(d, src, is_eng_name(src) ? 8 : 6);           /* engine names are kept whole */
    while (d[0] && text_w(f, d) > maxw)
        d[str_len(d) - 1u] = 0;
}

UI_SMALL static int32_t batt_level(void)                         /* ADC ch3 thresholds */
{
    return song.batt_raw >= 591 ? 3 : song.batt_raw >= 561 ? 2 : song.batt_raw >= 531 ? 1 : 0;
}
/* charging (a USB host powers us; there is no charger status line): bars fill 1, 2, 3
 * every 0.6 s, so the header is redrawn twice a second at most; else the level */
UI_SMALL static int32_t batt_shown(void)
{
    if (usb.config && !usb.suspended)
        return 1 + (int32_t)((fm1_ms / 600u) % 3u);
    return batt_level();
}

/* top bar: transport, BPM, octave | USB, battery, CPU; messages replace it */
UI_SMALL static void draw_head(void)
{
    char b[16];
    uint32_t i;
    int32_t x;
    uint32_t rec = (song.rec >> song.sel) & 1u ? 2u : song.rec != 0u;   /* 2 the selected track armed, 1 another */
    uint32_t sig = (uint32_t)song.playing * 3u + rec * 5u + (uint32_t)(song.octave + 8) * 11u + song.sel * 13131u +
                   (ui.msg_t ? str_hash(7u, ui.msg) : 0u) + (uint32_t)song.g[G_BPM] * 101u + (ui.bpm_t != 0) * 31u +
                   (uint32_t)batt_shown() * 7777u + (usb.config && !usb.suspended) * 99991u + mlu.on * 7919u +
                   syn_mode * 524287u;
    if (!ui.force && sig == ui.head_sig)
        return;
    ui.head_sig = sig;
    cv_begin(240, H_HEAD, C_BLACK);
    if (ui.msg_t) {
        cv_text(4, 1, &FONT_S, ui.msg, C_HI);
        cv_blit(0, Y_HEAD);
        return;
    }
    if (song.playing) {                               /* > play, square stop */
        for (i = 0; i < 5u; i++)
            cv_rect(4 + (int32_t)i * 2, 4 + (int32_t)i, 2, 10 - 2 * (int32_t)i, C_WHITE);
    } else {
        cv_rect(4, 5, 8, 8, C_HI);
    }
    if (rec)                                          /* recording armed: white = this track, gray = another */
        te_disc(21, 9, 4, rec == 2u ? TE_RED : C_GRAY);
    fmt_int(b, song.g[G_BPM]);
    x = 32;
    if (FELUCCA_ICONS) {                              /* metronome, then the BPM */
        cv_icon(x, 2, ICON_TEMPO, C_GRAY);
        x += 14;
    }
    x = cv_text(x, 1, &FONT_S, b, ui.bpm_t ? C_WHITE : C_HI);   /* white while SELECT turns it */
    if (syn_mode) {                                   /* BONBLOOP: the SYNTH MODE, then the octave alone, small */
        cv_rect(x + 8, 1, 44, 18, TE_COL[song.sel & 3u]);
        cv_text(x + 10, 1, &FONT_S, "SYNTH", C_BLACK);
        if (mlu.on)
            cv_text(x + 56, 1, &FONT_S, "LRN", C_AMB);
        else if (song.octave) {
            str_cpy(b, song.octave > 0 ? "+" : "", 4);
            fmt_int(b + str_len(b), song.octave);
            cv_text(x + 58, 1, &FONT_S, b, C_HI);
        }
    } else if (mlu.on) {                              /* MIDI LEARN on (2.6, GLO + F#3): in the octave's place */
        cv_text(x + 12, 1, &FONT_S, "LEARN", C_AMB);
    } else if (song.octave) {
        str_cpy(b, song.octave > 0 ? "+" : "", 4);
        fmt_int(b + str_len(b), song.octave);
        cv_text(x + 12, 1, &FONT_S, "OCT", C_GRAY);
        cv_text(x + 40, 1, &FONT_S, b, C_HI);
    }
    if (FELUCCA_ICONS) {                              /* the selected track: tape + number */
        cv_icon(156, 2, ICON_TAPE, C_GRAY);
        b[0] = (char)('1' + song.sel);
        b[1] = 0;
        cv_rect(168, 2, 12, 16, TE_COL[song.sel & 3u]);
        cv_text(170, 1, &FONT_S, b, C_BLACK);
    } else {
        b[0] = 'T';
        b[1] = (char)('1' + song.sel);
        b[2] = 0;
        cv_text(158, 1, &FONT_S, b, C_HI);
    }
    {   /* battery, 3 bars; USB when a host is there */
        int32_t lvl = batt_shown(), k;
        int32_t bx = 236 - 19;                          /* right edge (the CPU figure is in the console) */
        cv_rect(bx, 4, 17, 1, C_GRAY);
        cv_rect(bx, 12, 17, 1, C_GRAY);
        cv_rect(bx, 4, 1, 9, C_GRAY);
        cv_rect(bx + 16, 4, 1, 9, C_GRAY);
        cv_rect(bx + 17, 6, 2, 5, C_GRAY);
        for (k = 0; k < lvl; k++)
            cv_rect(bx + 2 + k * 5, 6, 3, 5, lvl == 1 && batt_level() <= 1 ? C_WHITE : C_HI);
        if (usb.config && !usb.suspended)
            cv_text(bx - 28, 1, &FONT_S, "USB", C_DIM);
    }
    cv_blit(0, Y_HEAD);
}
/* full redraw: the strips (head, columns, graph, foot) cover the rest, so only
 * the space between them is cleared, then the rules are drawn */
UI_SMALL static void draw_frame(void)
{
    uint32_t i;
    lcd_fill(0, H_HEAD, 240, Y_LABEL - H_HEAD, C_BLACK);
    lcd_fill(0, Y_SEP_END, 240, Y_GRAPH - Y_SEP_END, C_BLACK);
    lcd_fill(0, Y_GRAPH + H_GRAPH, 240, Y_FOOT - Y_GRAPH - H_GRAPH, C_BLACK);
    for (i = 0; i < 4u; i++)                            /* left inset of each column */
        lcd_fill(i * 60u, Y_LABEL, 4, Y_SEP_END - Y_LABEL, C_BLACK);
    lcd_fill(0, H_HEAD, 240, 1, syn_mode ? TE_COL[song.sel & 3u] : C_LINE);   /* (BONBLOOP: the SYNTH MODE's colour) */
    lcd_fill(0, Y_FOOT - 2, 240, 1, C_LINE);
    for (i = 1; i < 4u; i++)
        lcd_fill(i * 60u - 1u, H_HEAD + 4, 1, Y_SEP_END - H_HEAD - 4, C_LINE);
}

/* one column: [icon] LABEL / value unit / gauge, redrawn only when it changed.
 * ratio: 0..1000 for the gauge, -1 = no gauge. icon: ICON_* (icons.c), ICON_AUTO = by label */
#define LABEL_X (FELUCCA_ICONS ? ICON_CELL + ICON_GAP : 0)
UI_SMALL static void draw_column(uint32_t c, const char *label, const char *val, const char *unit, uint16_t vc,
                        int32_t ratio, uint32_t icon)
{
    char l[8], v[8], u[8], key[32];
    int32_t x, gw = 52, fx;
    if (icon == ICON_AUTO)
        icon = icon_for_label(label);
    fit(l, label, &FONT_T, 55 - LABEL_X);                 /* (1.7: the small font, the column's whole width) */
    fit(v, val, &FONT_S, is_eng_name(val) ? 56 : 40);   /* engine names whole */
    fit(u, unit, &FONT_T, 54 - text_w(&FONT_S, v) - 2);
    str_cpy(key, l, 8);                                 /* cache key: texts + colour + gauge */
    str_cpy(key + str_len(key), "|", 2);
    str_cpy(key + str_len(key), v, 8);
    str_cpy(key + str_len(key), "|", 2);
    str_cpy(key + str_len(key), u, 8);
    {
        uint32_t n = str_len(key);
        key[n] = (char)('A' + (vc == C_WHITE) + (vc == C_DIM) * 2);
        key[n + 1] = (char)(' ' + (ratio < 0 ? 0 : 1 + ratio / 20));
        key[n + 2] = (char)(icon == ICON_NONE ? '~' : '!' + icon % 90u);   /* same label, other icon */
        key[n + 3] = 0;
    }
    if (c < 4u) {                                       /* (the big values: their own fit) */
        str_cpy(ui.big_l[c], l, 8);
        fit(ui.big_v[c], val, &FONT_L, 112 - (unit[0] ? 4 : 0));
        str_cpy(ui.big_u[c], unit, 8);
        ui.big_c[c] = vc;
    }
    if (c == ui.hot_col) {
        str_cpy(ui.focus_l, l, 8);
        str_cpy(ui.focus_v, v, 8);
        str_cpy(ui.focus_u, u, 8);
    }
    if (!ui.force && str_eq(key, ui.col[c]))
        return;
    str_cpy(ui.col[c], key, sizeof ui.col[c]);
    cv_begin(55, Y_SEP_END - Y_LABEL, C_BLACK);         /* x 4..58: the rule at 59 stays */
    if (FELUCCA_ICONS && icon != ICON_NONE && l[0])
        cv_icon(0, 1, icon, TE_COL[c & 3u]);            /* icon rows 1..10 = the label's cap height */
    cv_text(l[0] ? LABEL_X : 0, 2, &FONT_T, l, C_GRAY);   /* (1.7: names small, the value large: on the icon's line) */
    x = cv_text(0, Y_VALUE - Y_LABEL, &FONT_S, v, vc);
    cv_text(x + 2, Y_VALUE - Y_LABEL + 4, &FONT_T, u, C_DIM);   /* (the unit small, on the value's baseline) */
    if (ratio >= 0) {                                   /* gauge: track, fill, 1 px end line */
        int32_t gy = Y_GAUGE - Y_LABEL;
        fx = ratio * gw / 1000;
        cv_rect(0, gy + 1, gw, 1, C_LINE);
        cv_rect(0, gy, fx, 3, vc == C_DIM ? C_DIM : ARC_OF(c));
        cv_rect(fx, gy - 1, 1, 5, vc == C_DIM ? C_HI : vc);
    }
    cv_blit(c * 60u + 4u, Y_LABEL);
}

/* Matches voice.c: attack is linear, decay and release are exponential
 * (env += (target - env) * k each tick, ~99 % after the set time). Time
 * axis is the parameter value (the times themselves are exponential). */
/* BONBLOOP 1.5: the EQ's curve (pages EQ 1, EQ 2), as a DAW's: the gain against frequency, 30 Hz .. 16 kHz on the
 * filter's scale, +-15 dB (3 px a dB), lines at 0 dB and at 100 Hz, 1 kHz, 10 kHz; each band its shape (the filters'
 * response at +12 dB, computed from eng_wave.c eq_run's: EQ_BELL by the distance in the scale's steps / 2, EQ_SHELF a
 * LOW shelf from -64 to +64 steps) scaled by its GAIN, the shapes added in dB; a dot per band at its FREQ and GAIN */
static const int8_t EQ_BELL[33] = {127, 126, 124, 120, 115, 109, 103, 96, 89, 81, 74, 67, 61, 54, 48, 43, 37, 33, 28, 24,
                                   21, 18, 15, 13, 11, 9, 7, 6, 5, 4, 3, 3, 2};
static const int8_t EQ_SHELF[65] = {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
                                    127, 126, 126, 126, 125, 124, 122, 120, 117, 114, 108, 102, 94, 85, 74, 64, 53, 42, 33,
                                    25, 19, 13, 10, 7, 5, 3, 2, 1, 1, 1};   /* (the rest 0) */
UI_SMALL static void graph_eq(const track_t *t, uint16_t c)
{
    uint32_t part = (uint32_t)(t - trk) % NTRK, b;    /* (1.9: FX > EQ, the track's; the drums' synx[NPART]) */
    uint32_t mine = cur_page()->id[0] == VX0 + SX_EF3 ? 1u : 0u;
    int32_t x, py = 0, f[4], g[4];
    static const uint8_t GRID_F[3] = {24, 71, 118};    /* 100 Hz, 1 kHz, 10 kHz */
    for (b = 0; b < 4u; b++)
        f[b] = sx_get(part, SX_EF1 + 2u * b), g[b] = sx_get(part, SX_EG1 + 2u * b);
    for (b = 0; b < 3u; b++)
        cv_line(6 + GRID_F[b] * 226 / 127, 10, 6 + GRID_F[b] * 226 / 127, 100, TE_G2);
    cv_line(6, 55, 232, 55, TE_G2);
    for (x = 6; x <= 232; x++) {
        int32_t fi = (x - 6) * 127 / 226, db = 0, y;
        for (b = 0; b < 4u; b++) {
            int32_t d = fi - f[b];
            if (b == 1u || b == 2u)
                db += g[b] * EQ_BELL[clamp((d < 0 ? -d : d) / 2, 0, 32)];
            else
                db += g[b] * EQ_SHELF[clamp(((b ? -d : d) + 64) / 2, 0, 64)];
        }
        y = clamp(55 - db * 3 / 127, 10, 100);
        if (x > 6)
            cv_line(x - 1, py, x, y, c);
        py = y;
    }
    for (b = 0; b < 4u; b++) {                        /* (this page's: its FREQ knob's colour, KNOB 1 or 3; the others
                                                       * grey) */
        int32_t r = (b >> 1) == mine ? 3 : 2;
        cv_rect(6 + f[b] * 226 / 127 - r, clamp(55 - g[b] * 3, 10, 100) - r, 2 * r + 1, 2 * r + 1,
                (b >> 1) == mine ? TE_COL[(b & 1u) * 2u] : TE_G3);
    }
}
UI_SMALL static void graph_adsr(const track_t *t, uint16_t c)
{
    uint32_t id = cur_page()->id[0], o = id == VX0 + SX_A2 ? 1u : id == VX0 + SX_A3 ? 2u : 0u;   /* (BONBLOOP 1.3: a
                                                         * synth oscillator's: OSC 2, 3; 0: the track's ENV, OSC 1's) */
    int32_t a = 4 + wv_envp(t, o, 0) * 50 / 127, d = 6 + wv_envp(t, o, 1) * 50 / 127, r = 6 + wv_envp(t, o, 3) * 60 / 127;
    int32_t top = 8, bot = 90, sus = wv_envp(t, o, 2) * 1000 / 127;     /* 0..1000 */
    int32_t x0 = 6, x1 = x0 + a, x3 = 232 - r, i, px, py;
    int32_t e = 32768;                                                  /* exp(-4.6 u), Q15 */
#define EGY(lvl) (bot - (lvl) * (bot - top) / 1000)
    cv_line(x0, bot, x1, top, c);                                       /* attack: linear */
    px = x1;
    py = top;
    for (i = 1; i <= d; i++) {                                          /* decay: exponential to SUS */
        int32_t lvl;
        e = (e * (32768 - 150733 / d)) >> 15;           /* k^d = exp(-4.6) */
        lvl = sus + ((1000 - sus) * e >> 15);
        cv_line(px, py, x1 + i, EGY(lvl), c);
        px = x1 + i;
        py = EGY(lvl);
    }
    cv_line(px, py, x3, EGY(sus), c);                                    /* sustain */
    px = x3;
    py = EGY(sus);
    e = 32768;
    for (i = 1; i <= r; i++) {                                          /* release: exponential to 0 */
        e = (e * (32768 - 150733 / r)) >> 15;
        cv_line(px, py, x3 + i, EGY(sus * e >> 15), c);
        px = x3 + i;
        py = EGY(sus * e >> 15);
    }
    cv_line(0, bot + 1, 239, bot + 1, C_LINE);
#undef EGY
}

UI_SMALL static void graph_lfo(const track_t *t, uint16_t c)
{
    int32_t x, py = 50;
    uint32_t ph = (uint32_t)t->p[P_LPHASE] << 25;
    for (x = 0; x < 240; x++) {                      /* (lfo_wave only reads the track) */
        int32_t y = 50 - lfo_wave((track_t *)t, ph + (uint32_t)x * (0xFFFFFFFFu / 120u)) * 38 / 32768;
        if (t->p[P_LWAVE] == 4)
            y = 50 - ((int32_t)((x / 20 * 2654435761u) >> 16) - 32768) * 38 / 32768;
        if (x)
            cv_line(x - 1, py, x, y, c);
        py = y;
    }
    cv_line(0, 50, 239, 50, C_LINE);
}

/* BONBLOOP: an oscillator of the synth (eng_wave.c; the arena read only, never claimed here). The OSC 1..3 pages show
 * their own, the others OSC 1. A table: its eight frames one behind the other, dim, the position playing in front in
 * the track's colour (between two frames, as the voice reads them; modulation included while a voice sounds, else
 * POS); SIN TRI SAW SQR NOISE: a cycle; "SYNTH MODE" as the mode starts */
UI_SMALL static const wt_part_t *wt_peek(const track_t *t)
{
    uint32_t p = (uint32_t)(t - trk);
    return p < NPART && t->engine == ENGI_WAVE && eng_arena_own[p] == ENGI_WAVE + 1u ? &eng_arena[p].wave : 0;
}
UI_SMALL static uint32_t wt_osc_shown(void)
{
    uint32_t o = ui.page - page_first(FAM_SYN);
    return on_syn_page() && o < WT_NOSC ? o : 0u;
}
UI_SMALL static int32_t wt_pos_shown(const track_t *t, const wt_part_t *w, uint32_t o)
{
    uint32_t i;
    for (i = 0; i < NVOICE; i++)
        if (t->v[i].active)
            return w->pos[o];
    return wv_get(t->p, WV_P1 + 3u * o) << 8;
}
UI_SMALL static void graph_wave(const track_t *t, uint16_t c)
{
    const wt_part_t *w = wt_peek(t);
    uint32_t o = wt_osc_shown(), mode = wave_track(t) ? (uint32_t)wv_get(t->p, WV_W1 + 3u * o) : 0u;
    int32_t f, x, py = 0, fp, fr, ox, oy;
    const char *m = 0;
    if (syn_splash) {
        cv_text((240 - text_w(&FONT_L, "SYNTH MODE")) / 2, 14, &FONT_L, "SYNTH MODE", c);
        cv_text((240 - text_w(&FONT_S, "3 OSC  ENV  LFO  FX")) / 2, 54, &FONT_S, "3 OSC  ENV  LFO  FX", C_HI);
        cv_text((240 - text_w(&FONT_S, "HOLD ENV: BACK TO SLOOP")) / 2, 76, &FONT_S, "HOLD ENV: BACK TO SLOOP", C_DIM);
        return;
    }
    if (!w || !wave_track(t))
        m = "PRESETS: A WAVE SOUND";                    /* another engine */
    else if (mode == WM_OFF)
        m = "OSC OFF";
    else if (mode >= WM_TAB && !w->sl[o].gen && !w->sl[o].ready)
        m = "BUILDING";
    if (m) {
        cv_text((240 - text_w(&FONT_S, m)) / 2, 40, &FONT_S, m, C_DIM);
        return;
    }
    fp = wt_pos_shown(t, w, o);
    if (mode < WM_TAB) {                                /* an analog shape: two cycles, as the oscillator draws it */
        uint32_t pw = 0x80000000u - (uint32_t)fp * 57950u;
        for (x = 0; x < 228; x++) {
            uint32_t ph = (uint32_t)x * (0xFFFFFFFFu / 114u);
            int32_t v = mode == WM_SIN ? sine_i(ph) : mode == WM_TRI ? osc_tri(ph) : mode == WM_SAW ? (int32_t)(ph >> 16) - 32768
                      : mode == WM_SQR ? (ph < pw ? 30000 : -30000) : (int32_t)((x * 2654435761u) >> 16) - 32768;
            int32_t y = 50 - v * 36 / 32768;
            if (x)
                cv_line(5 + x, py, 6 + x, y, c);
            py = y;
        }
        cv_line(0, 50, 239, 50, C_LINE);
        return;
    }
    for (f = (int32_t)WT_FR - 1; f >= 0; f--) {         /* the frames, back to front */
        ox = 6 + f * 10;
        oy = 80 - f * 7;
        for (x = 0; x < 150; x += 3) {
            int32_t y = oy - w->w[o][f][x * (int32_t)WT_LEN / 150] * 16 / WT_PEAK;
            if (x)
                cv_line(ox + x - 3, py, ox + x, y, C_DIM);
            py = y;
        }
    }
    fp = fp * 7 / 127;                                  /* the position: frame << 8 | between it and the next */
    fr = fp & 255;
    f = (fp >> 8) & 7;
    ox = 6 + fp * 10 / 256;
    oy = 80 - fp * 7 / 256;
    for (x = 0; x < 150; x++) {
        int32_t i = x * (int32_t)WT_LEN / 150, a = w->w[o][f][i], y;
        if (fr)
            a += ((w->w[o][(f + 1) & 7][i] - a) * fr) >> 8;
        y = oy - a * 18 / WT_PEAK;
        if (x)
            cv_line(ox + x - 1, py, ox + x, y, c);
        py = y;
    }
}

UI_SMALL static void graph_steps(const track_t *t, uint16_t c)
{
    uint32_t i, len = (uint32_t)t->p[P_SLEN];
    for (i = 0; i < NSTEP; i++) {                    /* 4 rows of 16 thin bars */
        int32_t x = 6 + (int32_t)(i % 16u) * 14 + (int32_t)(i % 16u) / 4 * 4, y = 6 + (int32_t)(i / 16u) * 24;
        const step_t *st = &t->step[i];
        if (i >= len)
            continue;
        if (step_on(st))
            cv_rect(x, y, 1, 14, c);
        else if (st->time == ST_TIE)
            cv_rect(x, y + 6, 1, 8, C_DIM);
        else
            cv_rect(x, y + 13, 1, 1, C_DIM);
        if (st->flags & SF_ACCENT)
            cv_rect(x - 1, y - 2, 3, 1, c);
        if ((song.playing && i == t->seq_idx) || i == ui.cursor)
            cv_rect(x - 1, y + 16, 3, 3, C_WHITE);
    }
}
/* STEP page: the cursor's 16-step bank as a little piano roll */
/* (2.7.2) SEQ > NOTES: the pattern as a piano roll (cv_oy 0, the whole strip). Each note a bar as long as its
 * step and TIEs, its FORCE as its fill (GHOST dark, SOFT hollow, NORM / HARD full, ACC a white cap), a SLIDE's line to
 * the next; the note under edit outlined in white, its name over it; the C rows named on the left, the beats ruled;
 * the playhead a line. The rows follow the note under edit when the pattern spans more than 18 semitones */
UI_SMALL static void graph_roll(const track_t *t, uint16_t c)
{
    uint32_t len = trk_len(t), nv = NV_ZOOM[clamp(1 + (int8_t)ui.zoom, 0, 3)], v0, i, j;
    int32_t lo = 127, hi = 0, x0 = 22, w = 240 - 22 - 2, top = 14, bot = H_GRAPH - 4 - (ui.sheet ? 18 : 0), rh, sel_note = last_note, cx;
    const step_t *cs = &t->step[ui.cursor];
    char b[12];
    if (nv > len)
        nv = len;
    v0 = ui.cursor / nv * nv;
    for (i = 0; i < len; i++)
        for (j = 0; j < t->step[i].n; j++) {
            if (t->step[i].note[j] < lo) lo = t->step[i].note[j];
            if (t->step[i].note[j] > hi) hi = t->step[i].note[j];
        }
    if (step_on(cs))
        sel_note = cs->note[ui.note_k % cs->n];
    if (lo > hi)
        lo = hi = sel_note;
    if (hi - lo < 11) {                                 /* (an octave at least: rows of a sane height) */
        lo = clamp((lo + hi) / 2 - 6, 0, 115);
        hi = lo + 11;
    }
    if (hi - lo > 17) {                                 /* (a wide pattern: 18 rows around the note under edit) */
        lo = clamp(sel_note - 8, lo, hi - 17);
        hi = lo + 17;
    }
    lo = clamp(lo - 1, 0, 127), hi = clamp(hi + 1, 0, 127);
    rh = (bot - top) / (hi - lo + 1);
    fmt_int(b, (int32_t)v0 + 1);
    str_cpy(b + str_len(b), "-", 2);
    fmt_int(b + str_len(b), (int32_t)(v0 + nv > len ? len : v0 + nv));
    cv_text(2, 0, &FONT_S, b, C_GRAY);
    str_cpy(b, "ZOOM ", 12);
    fmt_int(b + 5, (int32_t)nv);
    cv_text(238 - text_w(&FONT_S, b), 0, &FONT_S, b, C_DIM);
    for (i = (uint32_t)lo; i <= (uint32_t)hi; i++) {    /* rows: the black keys' bands, the C's ruled and named */
        int32_t y = bot - ((int32_t)i - lo + 1) * rh;
        uint32_t pc = i % 12u;
        if ((0x54Au >> pc) & 1u)
            cv_rect(x0, y, w, rh, TE_G1);
        if (!pc) {
            note_name(b, i);
            cv_text(0, y + rh / 2 - 6, &FONT_S, b, C_DIM);
            cv_rect(x0, y + rh - 1, w, 1, C_GRAY);
        }
    }
    for (i = 0; i <= nv; i++)                           /* the beats (4 steps), a tick on each step */
        if (i % 4u == 0u)
            cv_rect(x0 + (int32_t)i * w / (int32_t)nv, top, 1, bot - top, i % 16u ? C_DIM : C_GRAY);
        else if (nv <= 32u)
            cv_rect(x0 + (int32_t)i * w / (int32_t)nv, bot + 1, 1, 2, C_DIM);
    for (i = 0; i < len; i++) {                         /* the notes (one starting before the view: its part in it) */
        const step_t *st = &t->step[i];
        uint32_t n = note_len(t, i), f;
        int32_t xa, xb, y;
        if (!step_on(st) || i + n <= v0 || i >= v0 + nv)
            continue;
        xa = x0 + ((int32_t)i - (int32_t)v0) * w / (int32_t)nv + 1;
        f = i + n - 1u;                                 /* (BONBLOOP 1.9.2) to its own end: 1/8 of a step (core.h SF_END) */
        f = (t->step[f].flags & SF_END) && !(n == 1u && st->rat) ? f * 8u + t->nend[f] + 1u : (i + n) * 8u;
        xb = x0 + ((int32_t)f - (int32_t)v0 * 8) * w / ((int32_t)nv * 8) - 1;
        xb = xb <= xa ? xa + 1 : xb;
        xa = xa < x0 ? x0 : xa;
        xb = xb > x0 + w ? x0 + w : xb;
        for (j = 0; j < st->n; j++) {
            if (st->note[j] < lo || st->note[j] > hi)
                continue;
            y = bot - ((int32_t)st->note[j] - lo + 1) * rh + 1;
            f = note_force(st, j);
            cv_rect(xa, y, xb - xa, rh - 1, f ? c : TE_DIM[song.sel & 3u]);
            if (f == 1u && rh > 4)                      /* SOFT: hollow */
                cv_rect(xa + 1, y + 1, xb - xa - 2, rh - 3, C_BLACK);
            if (f == 4u)                                /* ACC: a white cap */
                cv_rect(xa, y, xb - xa, 2, C_WHITE);
            if ((st->flags & SF_SLIDE) && j == 0u)
                cv_rect(xb, y + rh / 2 - 1, 4, 2, c);
            if (i == ui.cursor && j == ui.note_k % st->n) {   /* the note under edit */
                cv_rect(xa - 2, y - 2, xb - xa + 4, 1, C_WHITE), cv_rect(xa - 2, y + rh - 1, xb - xa + 4, 1, C_WHITE);
                cv_rect(xa - 2, y - 2, 1, rh + 2, C_WHITE), cv_rect(xb + 1, y - 2, 1, rh + 2, C_WHITE);
                note_name(b, st->note[j]);
                cv_text(xa, y - 15 < top ? y + rh + 1 : y - 15, &FONT_S, b, C_WHITE);
            }
        }
    }
    cx = x0 + ((int32_t)ui.cursor - (int32_t)v0) * w / (int32_t)nv;
    if (!step_on(cs) && sel_note >= lo && sel_note <= hi) {   /* an empty step: a frame where a note would go */
        int32_t y = bot - (sel_note - lo + 1) * rh + 1, xw = w / (int32_t)nv - 2;
        cv_rect(cx + 1, y - 1, xw, 1, C_WHITE), cv_rect(cx + 1, y + rh - 1, xw, 1, C_WHITE);
        cv_rect(cx + 1, y - 1, 1, rh, C_WHITE), cv_rect(cx + xw, y - 1, 1, rh, C_WHITE);
    }
    if (song.playing && t->seq_idx >= v0 && t->seq_idx < v0 + nv) {   /* the playhead */
        int32_t x = x0 + ((int32_t)t->seq_idx - (int32_t)v0) * w / (int32_t)nv;
        cv_rect(x, top, 1, bot - top, C_HI);
        cv_rect(x - 2, top, 5, 2, C_HI);
    }
    if (ui.sheet) {                                     /* OCT+ held: the note's sheet */
        char nu[8];
        const char *val[6] = {cs->flags & SF_SLIDE ? "ON" : "OFF", nu, FILL_N[step_fill(t, ui.cursor) % 3u], "", "", ""};
        fmt_int(nu, t->micro[ui.cursor]);
        if (t->micro[ui.cursor] > 0) { nu[0] = '+'; fmt_int(nu + 1, t->micro[ui.cursor]); }
        sheet_bar(H_GRAPH - 16, SHEET_N, val, ui.sheet - 1u, TE_DIM[song.sel & 3u]);
    }
}
UI_SMALL static void graph_scale(const track_t *t, uint16_t c)
{
    static const uint8_t BLACK[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
    uint32_t i, mask = scale_mask(t);
    for (i = 0; i < 12u; i++) {
        uint32_t deg = (i + 12u - (uint32_t)t->p[P_ROOT]) % 12u;
        int32_t x = 6 + (int32_t)i * 19;
        uint16_t col = (mask >> deg) & 1u ? (deg == 0u ? C_WHITE : c) : C_DIM;
        cv_rect(x, BLACK[i] ? 10 : 40, 16, 1, col);
        cv_rect(x + 7, BLACK[i] ? 10 : 40, 1, 40, col);
    }
}

/* FX > MIXER, the synth's OSC MIX and FX MIX (BONBLOOP 1.7): a mixer, a channel strip under each knob's column: the
 * fader's slot and its ticks, the level up from the bottom in the knob's dim colour, the cap in its colour (white while
 * it turns), the value under it, the name at the foot. An empty knob: no strip */
UI_SMALL static void graph_fx(const track_t *t, uint16_t c)
{
    uint32_t i, k;
    (void)t, (void)c;
    for (i = 0; i < 4u; i++) {
        int16_t *vp;
        const param_desc_t *d = page_desc(cur_page(), i, &vp);
        int32_t x = (int32_t)i * 60 + 30, h, w;
        uint16_t col = ui.big_c[i] == C_WHITE ? C_WHITE : TE_COL[i];
        if (!d || !vp || d->max == d->min)
            continue;
        h = (clamp(*vp, d->min, d->max) - d->min) * 72 / (d->max - d->min);
        for (k = 0; k <= 4u; k++)
            cv_rect(x - 14, 8 + (int32_t)k * 18, 6, 1, TE_G2);
        cv_rect(x - 2, 8, 4, 72, C_LINE);
        cv_rect(x - 2, 80 - h, 4, h, ARC_OF(i));
        cv_rect(x - 11, 76 - h, 22, 8, col);          /* the cap, a line across its middle */
        cv_rect(x - 9, 80 - h, 18, 1, C_BLACK);
        w = text_w(&FONT_S, ui.big_v[i]);
        cv_text(x - w / 2, 88, &FONT_S, ui.big_v[i], col == C_WHITE ? C_WHITE : C_HI);
        w = text_w(&FONT_T, ui.big_l[i]);
        cv_text(x - w / 2, 108, &FONT_T, ui.big_l[i], C_GRAY);
    }
}
/* SLICER page: the pattern's 16 steps, a 'x' step a full bar; a '.' step: GATE a bar as high as
 * it stays open (DEPTH), STUT hatched (it repeats the last 'x'); the step playing underlined.
 * Grey when the SLICER is OFF. */
UI_SMALL static void graph_slicer(const track_t *t, uint16_t c)
{
    uint32_t i, pat = sl_pattern(t), mode = (uint32_t)t->p[P_SLCR], cur = sl[t - trk].idx;
    int32_t open = 70 - t->p[P_SLDEPTH] * 70 / 127;      /* px a closed GATE step keeps */
    uint16_t col = mode == SL_OFF ? C_DIM : c;
    for (i = 0; i < 16u; i++) {
        int32_t x = 4 + (int32_t)i * 14 + (int32_t)(i / 4u) * 2, y;
        if ((pat >> i) & 1u) {
            cv_rect(x, 10, 11, 70, col);
        } else if (mode == SL_STUT) {
            for (y = 10; y < 80; y += 4)
                cv_rect(x, y, 11, 1, col);
        } else {
            cv_rect(x, 79, 11, 1, C_LINE);
            if (open)
                cv_rect(x, 80 - open, 11, open, C_DIM);
        }
        if (mode != SL_OFF && i == cur)
            cv_rect(x, 85, 11, 3, C_WHITE);
    }
}
UI_SMALL static uint32_t steps_hash(const track_t *t)
{
    uint32_t h = 2166136261u, i;
    for (i = 0; i < NSTEP; i++) {
        const step_t *st = &t->step[i];
        h = (h ^ (st->note[0] + st->n * 128u + st->time * 1024u + st->flags * 4096u + st->note[1] * 65536u +
                  (uint32_t)t->nend[i] * 16777216u)) * 16777619u;   /* (BONBLOOP 1.9.2: + where its notes end) */
    }
    return h;
}

UI_SMALL static uint32_t str_hash(uint32_t h, const char *s)
{
    while (*s)
        h = (h ^ (uint8_t)*s++) * 16777619u;
    return h;
}

/* a page drawn with its values large (graph_big) */
UI_SMALL static int big_page(const page_t *pg)            /* (2.6: the INSERT pages too: their middle stayed empty) */
{
    return (pg->graph == GR_NONE || pg->graph == GR_INS) && pg->scope != SC_SONG && pg->scope != SC_DRUM;
}
UI_SMALL static uint32_t comp_meter(uint32_t h);    /* (BONBLOOP 1.7: below, with graph_comp) */
UI_SMALL static uint32_t clip_meter(uint32_t h);    /* (1.9.2: below, with graph_clip) */
UI_SMALL static uint32_t graph_signature(void)
{
    const page_t *pg = cur_page();
    const track_t *t = TSEL;
    uint32_t h = 2166136261u, i;
    if (ui.hot_t && settings.zoom)
        h = str_hash(str_hash(str_hash(h ^ 0x5555u, ui.focus_v), ui.focus_l), ui.focus_u);
    if (!ui.home && (big_page(pg) || pg->graph == GR_COMP || pg->graph == GR_FX))   /* the big values: as the columns
                                                         * show them */
        for (i = 0; i < 4u; i++)
            h = str_hash(str_hash(str_hash(h ^ ui.big_c[i] * 31u, ui.big_v[i]), ui.big_l[i]), ui.big_u[i]);
    if (!ui.home && pg->graph == GR_COMP)            /* (BONBLOOP 1.7) the COMP page's meter, falling */
        h = comp_meter(h);
    if (!ui.home && pg->graph == GR_CLIP)            /* (1.9.2) FX > CLIP's: its values, the meter */
        for (i = 0, h = clip_meter(h); i < 4u; i++)
            h = str_hash(h ^ ui.big_c[i] * 31u, ui.big_v[i]);
    if (ui.home)
        return h ^ (ui.frame / 2u);                  /* scope: redraw every other frame */
    h ^= (uint32_t)pg->graph * 131u + TSEL->eng_req + song.sel * 7777u;
    for (i = 0; i < P_COUNT; i++)
        h = (h ^ (uint32_t)t->p[i]) * 16777619u;
    for (i = 0; song.sel < NPART && i < NELEM(synx[0].w); i++)   /* (BONBLOOP 1.3) the synth's extras: OSC 2 and 3's
                                                         * ENV drawn; 1.5 the EQ's curve */
        h = (h ^ synx[song.sel].w[i]) * 16777619u;
    h = (((h ^ synx[NPART].w[4]) * 16777619u) ^ synx[NPART].w[5]) * 16777619u;   /* (1.7) FX > EQ's, the master's */
    h ^= ui.page * 2654435761u;
    h ^= (uint32_t)TSEL->preset * 7u + (uint32_t)song.g[G_SLOT] * 13u + TSEL->user * 257u + up_gen * 7919u + ui.uslot * 104729u;
    {   /* (2.7) the favourites: the PRESETS list and its FAV column */
        uint32_t q;
        for (q = 0; q < FAV_N; q++) h = h * 31u + fav[q];
    }
    if (pg->graph == GR_WAVE) {                      /* BONBLOOP: the table as it is built, the position playing */
        const wt_part_t *w = wt_peek(t);
        uint32_t o = wt_osc_shown();
        h ^= (syn_splash != 0u) * 104729u + o * 524287u;
        if (w)
            h ^= (uint32_t)w->sl[o].gen * 7919u + (uint32_t)(wt_pos_shown(t, w, o) >> 9) * 31u + 1u;
    }
    if (pg->graph == GR_SLCR && t->p[P_SLCR])        /* the SLICER's step playing */
        h ^= (sl[song.sel].idx + 1u) * 2654435761u;
    if (pg->graph == GR_SLOTS)                       /* (a checksum over each slot) */
        for (i = 0; i < 4u; i++)
            h ^= (uint32_t)project_used(i) << (20u + i);
    if (pg->graph == GR_STEPS || pg->graph == GR_ROLL) {
        uint32_t ph = song.playing ? t->seq_idx : 0xFFFFu;
        if (pg->graph == GR_ROLL)                    /* (2.7.2 NOTES: the note under edit, the zoom, the sheet) */
            h ^= ui.note_k * 104729u + ui.zoom * 7u + ui.sheet * 977u + (uint32_t)t->micro[ui.cursor] * 31337u +
                 step_fill(t, ui.cursor) * 3u + last_note * 65537u;
        h ^= steps_hash(t) + ph * 31u + ui.cursor * 7919u;
    }
    return h;
}
/* preset browser: the list by kind (ui.c BANK), the current one in white */
UI_SMALL static void graph_browse(void)
{
    uint32_t total, cur = preset_pos(&total), e, k;
    int32_t row;
    if (!total)
        return;
    for (row = -3; row <= 3; row++) {
        int32_t y = 4 + (row + 3) * 17;
        char tag[5], nm[13];
        int sel = row == 0;
        uint32_t n = (cur + total * 4u + (uint32_t)row) % total;
        e = preset_at(n, &k);
        if (e == NENGINES) {                             /* user preset: "U07" and its name */
            up_slot_label(tag, k);
            up_name(k, nm);
        } else {                                         /* its kind: BASS, KEYS, PAD... */
            str_cpy(tag, preset_kind(n), sizeof tag);
            str_cpy(nm, ENGINES[e]->presets[k].name, sizeof nm);
        }
        if (sel)
            cv_rect(4, y + 6, 3, 3, C_WHITE);
        cv_text(14, y, &FONT_S, tag, sel ? C_GRAY : C_DIM);
        cv_text(54, y, &FONT_S, nm, sel ? C_WHITE : C_GRAY);
    }
}

/* user preset slots around the selected one: "U07  NAME" / EMPTY, the selected one in white */
UI_SMALL static void graph_user(void)
{
    int32_t row, first = clamp((int32_t)ui.uslot - 3, 0, UP_SLOTS - 7);
    for (row = 0; row < 7; row++) {
        uint32_t k = (uint32_t)(first + row);
        int32_t y = 4 + row * 17;
        char tag[4], nm[13];
        int sel = k == ui.uslot, used = up_used(k);
        up_slot_label(tag, k);
        if (used)
            up_name(k, nm);
        else
            str_cpy(nm, "EMPTY", sizeof nm);
        if (sel)
            cv_rect(4, y + 6, 3, 3, C_WHITE);
        cv_text(14, y, &FONT_S, tag, sel ? C_WHITE : C_GRAY);
        cv_text(54, y, &FONT_S, nm, used ? (sel ? C_WHITE : C_HI) : C_DIM);
    }
}

/* the sections A..D (the four slots): used / empty, the selected one in white */
UI_SMALL static void graph_slots(void)
{
    uint32_t i;
    for (i = 0; i < 4u; i++) {
        int32_t y = 8 + (int32_t)i * 26;
        char b[4];
        int sel = (int32_t)i + 1 == song.g[G_SLOT];
        b[0] = (char)('A' + i);
        b[1] = 0;
        if (sel)
            cv_rect(4, y + 6, 3, 3, C_WHITE);
        cv_text(14, y, &FONT_S, b, sel ? C_WHITE : C_GRAY);
        cv_text(40, y, &FONT_S, project_used(i) ? "USED" : "EMPTY", project_used(i) ? (sel ? C_WHITE : C_HI) : C_DIM);
    }
}

/* TRACKS page: four channel strips (mixer style), one under each column: number +
 * REC / ARM / MUTE, the sound's short name, then a level fader with the output meter
 * (note activity), the pattern over its LEN (time running down; bar width = notes in
 * the step) and the play head. The selected track is drawn bright. Every part is
 * its own small canvas with its own signature: while the transport runs only the
 * head markers and the meters move. (No ZOOM focus here.) */
#define TS_HEAD_Y (Y_GRAPH + 2)
#define TS_NAME_Y (Y_GRAPH + 20)
#define TS_Y (Y_GRAPH + 38)
#define TS_H 84                                      /* body: rows Y_GRAPH + 38 .. + 121 */
static struct {
    uint32_t head[NTRK], name[NTRK], fader[NTRK], ov[NTRK], mark[NTRK];
    uint8_t meter[NTRK];
} ts;

UI_SMALL static int32_t meter_px(int32_t a)                   /* |sample| (Q15) -> px: 6 dB = TS_H / 10 */
{
    int32_t lg = 0, v;
    if (a < 64)
        return 0;
    while ((a >> lg) > 1)
        lg++;
    v = lg * 8 + (((a << 3) >> lg) & 7);             /* 8 log2(a): 48 (-54 dB) .. 128 (+6 dB) */
    return clamp((v - 48) * (TS_H - 2) / 80, 0, TS_H - 2);
}

UI_SMALL static uint32_t trk_level(uint32_t c)                /* LEVEL 0..127 (the drum track: GLO > DRUMS LEVEL) */
{
    return (uint32_t)(c == TRK_DRUM ? song.g[G_DRLVL] : trk[c].p[P_LEVEL]) & 127u;
}

UI_SMALL static void trk_short_name(uint32_t c, char *b)      /* the track's sound, b holds 13 */
{
    const track_t *t = &trk[c];
    const engine_t *e = ENGINES[t->eng_req % NENGINES];
    if (c == TRK_DRUM)
        str_cpy(b, "DRUM", 13);
    else if (user_of(t) < UP_SLOTS)
        up_name(user_of(t), b);
    else if (e->npresets)
        str_cpy(b, e->presets[t->preset % e->npresets].name, 13);
    else
        str_cpy(b, e->name, 13);
}

UI_SMALL static void draw_tracks(void)
{
    uint32_t c;
    if (ui.force) {
        lcd_fill(0, Y_GRAPH, 240, H_GRAPH, C_BLACK);
        for (c = 0; c < NTRK; c++)
            ts.meter[c] = 0;
    }
    for (c = 0; c < NTRK; c++) {
        track_t *t = &trk[c];
        uint32_t x0 = c * 60u + 4u, sel = c == song.sel, lvl = trk_level(c), mute = !lvl || t->p[P_MUTE];
        uint32_t arm = (song.rec >> c) & 1u, st = arm ? (song.playing ? 1u : 2u) : mute ? 3u : 0u, sig;
        uint32_t len = t->p[P_SLEN] > 0 ? (uint32_t)t->p[P_SLEN] : 1u, row = 0xFFFFu;
        int32_t pk, m;
        char b[16];
        if (c == TRK_DRUM) {
            pk = drums.peak;
            drums.peak = 0;
        } else {
            pk = t->peak;
            t->peak = 0;
        }
        /* head: number (white = selected), REC / ARM / MUTE */
        sig = 1u + sel + st * 2u;
        if (ui.force || sig != ts.head[c]) {
            ts.head[c] = sig;
            b[0] = (char)('1' + c);
            b[1] = 0;
            cv_begin(54, 16, C_BLACK);
            cv_text(0, 0, &FONT_S, b, sel ? C_WHITE : C_GRAY);
            if (sel)
                cv_rect(0, 15, 8, 1, C_WHITE);
            if (st == 1u || st == 2u) {
                cv_rect(14, 5, 6, 6, st == 1u ? C_WHITE : C_AMB);
                cv_text(24, 0, &FONT_S, st == 1u ? "REC" : "ARM", st == 1u ? C_WHITE : C_AMB);
            } else if (st) {
                cv_text(14, 0, &FONT_S, "MUTE", C_DIM);
            }
            cv_blit(x0, TS_HEAD_Y);
        }
        /* the sound: preset name (user preset, DRUM), cut to the column */
        trk_short_name(c, b);
        while (b[0] && text_w(&FONT_S, b) > 54)
            b[str_len(b) - 1u] = 0;
        sig = str_hash(2u + sel, b);
        if (ui.force || sig != ts.name[c]) {
            ts.name[c] = sig;
            cv_begin(54, 16, C_BLACK);
            cv_text(0, 0, &FONT_S, b, sel ? C_HI : C_DIM);
            cv_blit(x0, TS_NAME_Y);
        }
        /* fader (LEVEL) + meter of the output */
        m = mute ? 0 : meter_px(pk);
        if (m < ts.meter[c] - 3)
            m = ts.meter[c] - 3;                     /* falls ~20 dB/s */
        ts.meter[c] = (uint8_t)(m < 0 ? 0 : m);
        sig = 1u + lvl + ts.meter[c] * 128u + sel * 65536u + (sel && ui.hot_t && ui.hot_col == 1u) * 131072u +
              (t->p[P_MUTE] != 0) * 262144u;
        if (ui.force || sig != ts.fader[c]) {
            int32_t fy = (int32_t)(TS_H - 2u) - (int32_t)lvl * (TS_H - 4) / 127;
            ts.fader[c] = sig;
            cv_begin(12, TS_H, C_BLACK);
            cv_rect(3, 0, 1, TS_H, C_LINE);
            if (lvl) {
                cv_rect(2, fy, 3, TS_H - fy, C_DIM);
                cv_rect(0, fy - 1, 7, 2, (sig & 131072u) ? C_WHITE : sel ? C_HI : C_GRAY);
            } else {
                cv_rect(0, TS_H - 2, 7, 2, C_DIM);
            }
            if (ts.meter[c])
                cv_rect(9, TS_H - ts.meter[c], 3, ts.meter[c], C_AMB);
            cv_blit(x0, TS_Y);
        }
        /* the pattern: one band per step over LEN, a note step as wide as its notes */
        sig = steps_hash(t) * 31u + len * 7u + sel;
        if (ui.force || sig != ts.ov[c]) {
            uint32_t i;
            ts.ov[c] = sig;
            cv_begin(34, TS_H, C_BLACK);
            cv_rect(17, 0, 1, TS_H, C_LINE);
            for (i = 0; i < len; i++) {
                const step_t *s = &t->step[i];
                int32_t r0 = (int32_t)(i * TS_H / len), r1 = (int32_t)((i + 1u) * TS_H / len), h = r1 - r0;
                if (h > 2)
                    h--;                             /* a gap between steps when there is room */
                if (h < 1)
                    h = 1;
                if (step_on(s)) {
                    int32_t w = 2 + 6 * (int32_t)s->n;
                    cv_rect(17 - w / 2, r0, w, h, (s->flags & SF_ACCENT) && sel ? C_WHITE : sel ? C_HI : C_GRAY);
                } else if (s->time == ST_TIE) {
                    cv_rect(16, r0, 3, r1 - r0, C_DIM);
                }
            }
            cv_blit(x0 + 14u, TS_Y);
        }
        /* play head: a small arrow right of the pattern (white while recording) */
        if (song.playing)
            row = ((t->seq_idx % len) * 2u + 1u) * TS_H / (2u * len);
        sig = 1u + row + (st == 1u) * 65536u;
        if (ui.force || sig != ts.mark[c]) {
            int32_t i;
            ts.mark[c] = sig;
            cv_begin(5, TS_H, C_BLACK);
            if (row != 0xFFFFu)
                for (i = -2; i <= 2; i++) {
                    int32_t w = 5 - 2 * (i < 0 ? -i : i);
                    cv_rect(5 - w, (int32_t)row + i, w, 1, st == 1u ? C_WHITE : C_AMB);
                }
            cv_blit(x0 + 49u, TS_Y);
        }
    }
}

/* oscilloscope of the output, triggered on a rising zero crossing */
#include "scratch.h"                                /* sloop-lab: the snapshot in the main loop's shared scratch */
_Static_assert(sizeof main_scratch.snap == SCOPE_N * sizeof(int16_t), "the scratch holds the snapshot");
UI_SMALL static void graph_scope(uint16_t c)
{
    int16_t *snap = main_scratch.snap;
    uint32_t w = scope_w, i, trig = 0;
    int32_t py = 50, x, peak = 1500;
    for (i = 0; i < SCOPE_N; i++) {
        snap[i] = scope_buf[(w + i) & (SCOPE_N - 1u)];
        if (snap[i] > peak)
            peak = snap[i];
        else if (-snap[i] > peak)
            peak = -snap[i];
    }
    for (i = 1; i < SCOPE_N - 240u; i++)
        if (snap[i - 1] < 0 && snap[i] >= 0) {
            trig = i;
            break;
        }
    cv_line(0, 50, 239, 50, C_LINE);
    for (x = 0; x < 240; x++) {
        int32_t y = 50 - snap[trig + (uint32_t)x] * 44 / peak;   /* auto-scaled */
        if (x)
            cv_line(x - 1, py, x, y, c);
        py = y;
    }
}

/* (BONBLOOP 1.7) FX > COMP as a DAW's compressor: RATIO, ATTACK, RELEASE down the left, each in its knob's colour; on
 * the right the THRESH meter, 0 dB at the top to -48 dB (2 px a dB): the gain reduction from the top, the master's
 * left and right as they come in (brighter from -3 dB), the threshold a line and a pointer, its value beside it (1.7:
 * the palette's tones, knobs on the left). The meter falls 0.5 dB a frame (comp_meter, each frame: graph_signature) */
UI_SMALL static int32_t knob_ratio(uint32_t c);      /* (1.7: below, with graph_big) */
#define KM_Y 20                                       /* the meter's 0 dB */
static uint8_t km_px[3];                              /* left, right (up from -48 dB), the reduction (down from 0 dB) */
UI_SMALL static uint32_t comp_meter(uint32_t h)
{
    uint32_t i;
    int32_t v[3];
    v[0] = clamp(96 + ((log2q16(kmp.ml) * 12) >> 16), 0, 96);   /* (octaves Q16 x 12.04: 2 px a dB) */
    v[1] = clamp(96 + ((log2q16(kmp.mr) * 12) >> 16), 0, 96);
    v[2] = clamp((kmp.gr * 12) >> 16, 0, 96);
    kmp.ml = kmp.mr = 0, kmp.gr = 0;
    for (i = 0; i < 3u; i++) {
        km_px[i] = (uint8_t)(v[i] >= km_px[i] ? v[i] : km_px[i] - 1);
        h = (h ^ km_px[i]) * 16777619u;
    }
    return h;
}
UI_SMALL static void graph_comp(void)
{
    static const char *const L[3] = {"RATIO", "ATTACK", "RELEASE"};
    int32_t i, x, thr = song.g[G_KTHR], yt = KM_Y + clamp(-thr, 0, 48) * 2, y;
    for (i = 1; i < 4; i++) {                         /* KNOB 2 .. 4: a knob each (1.7), its name and value beside */
        int32_t y0 = 2 + (i - 1) * 41;
        int hot = ui.big_c[i] == C_WHITE;
        te_dial(20, y0 + 19, 15, knob_ratio((uint32_t)i), hot ? C_WHITE : KNOB_LIT, C_LINE);
        cv_text(44, y0 + 6, &FONT_T, L[i - 1], C_GRAY);
        x = cv_text(44, y0 + 17, &FONT_S, ui.big_v[i], hot ? C_WHITE : C_HI);
        cv_text(x + 2, y0 + 21, &FONT_T, ui.big_u[i], C_DIM);
    }
    cv_text(136, 5, &FONT_T, "THRESH", C_GRAY);
    for (i = 0; i <= 4; i++)                          /* 0, -12, -24, -36, -48 dB */
        cv_rect(132, KM_Y + i * 24, 6, 1, TE_G2);
    cv_rect(142, KM_Y, 8, 96, C_LINE);                /* the reduction, from the top */
    cv_rect(142, KM_Y, 8, km_px[2], C_HI);
    for (i = 0; i < 2; i++) {                         /* left, right: green, yellow from -12 dB, red from -3 */
        int32_t bx = 156 + i * 10, top = KM_Y + 96 - km_px[i];
        cv_rect(bx, KM_Y, 8, 96, C_LINE);
        for (y = top; y < KM_Y + 96; y++)
            cv_rect(bx, y, 8, 1, y < KM_Y + 6 ? RGB(150, 240, 190) : RGB(30, 204, 112));   /* (1.7: green, lighter
                                                         * over -3 dB) */
    }
    cv_rect(140, yt, 37, 1, thr ? C_HI : C_DIM);      /* the threshold, its pointer */
    for (i = 0; i < 7; i++)
        cv_rect(178 + i, yt - i, 1, 2 * i + 1, thr ? C_HI : C_DIM);
    y = clamp(yt - 8, KM_Y - 2, 84);
    if (thr) {
        x = cv_text(188, y, &FONT_S, ui.big_v[0], ui.big_c[0] == C_WHITE ? C_WHITE : C_HI);
        cv_text(x + 2, y + 4, &FONT_T, "DB", C_GRAY);
    } else {
        cv_text(188, y, &FONT_S, "OFF", C_DIM);
    }
    if (thr && km_px[2] >= 2u) {                      /* the reduction now, in dB */
        char b[8];
        b[0] = '-';
        fmt_int(b + 1, km_px[2] / 2u);
        x = cv_text(188, 102, &FONT_S, b, C_AMB);
        cv_text(x + 2, 106, &FONT_T, "DB", C_GRAY);
    }
}

/* (BONBLOOP 1.9.2) FX > CLIP as a soft clipper's screen: its transfer curve (in -> out, 0 .. +6 dBFS both ways: the
 * clipper's own code on a ramp), its ceiling (violet), and the selected track's level in and out (green, as the COMP's,
 * while it clips) */
static uint8_t clm_px[2];
UI_SMALL static uint32_t clip_meter(uint32_t h)
{
    uint32_t i;
    for (i = 0; i < 2u; i++) {
        int32_t v = clip_pk[i] ? clamp(96 + ((log2q16(clip_pk[i]) * 12) >> 16), 0, 96) : 0;
        clip_pk[i] = 0;
        clm_px[i] = (uint8_t)(v >= clm_px[i] ? v : clm_px[i] - 1);
        h = (h ^ clm_px[i]) * 16777619u;
    }
    return h;
}
UI_SMALL static void graph_clip(const track_t *t)
{
    static const char *const L[2] = {"IN", "OUT"};
    const page_t *pg = cur_page();
    int16_t *vp;
    int32_t v[4], i, x, y, px = 8, py = 116, c, top = 116, a = 1036;   /* (a: the input, -30 dBFS to +6 across) */
    for (i = 0; i < 4; i++)
        v[i] = page_desc(pg, (uint32_t)i, &vp) && vp ? *vp : 0;
    c = is_drum(t) ? v[0] : v[0] * 2;
    for (i = 0; i <= 6; i++) {                        /* the grid: 6 dB a line, -30 .. +6 dBFS both ways; 0 dB brighter */
        cv_rect(8 + i * 112 / 6, 4, 1, 113, i == 5 ? C_DIM : C_LINE);
        cv_rect(8, 4 + i * 112 / 6, 113, 1, i == 1 ? C_DIM : C_LINE);
    }
    for (x = 0; x <= 112; x++) {                      /* the curve: the clipper's own code on the input, in dB */
        int32_t s = a;
        if (c)
            clip_block(&s, 1u, c, (uint32_t)clamp(v[2], 0, 2), v[1], v[3]);
        s = s < 0 ? -s : s;
        y = s ? clamp(23 - ((log2q16((uint32_t)s) * 1199) >> 22), 4, 116) : 116;   /* (6.02 dB an octave, 3.11 px a dB) */
        top = y < top ? y : top;
        if (x)
            cv_line(px, py, 8 + x, y, c ? KNOB_LIT : C_DIM);
        px = 8 + x, py = y;
        a = (a * 4250) >> 12;                         /* (+0.32 dB a pixel) */
    }
    if (c) {                                          /* its ceiling: the curve's top */
        for (x = 8; x < 121; x += 4)
            cv_rect(x, top, 2, 1, C_ON);
        cv_text(10, top < 20 ? top + 3 : top - 13, &FONT_T, "CEIL", C_ON);
    }
    cv_text(104, 104, &FONT_T, "IN", C_DIM);
    for (i = 0; i <= 4; i++)                          /* 0, -12, -24, -36, -48 dB */
        cv_rect(138, KM_Y + i * 24, 6, 1, TE_G2);
    for (i = 0; i < 2; i++) {
        int32_t bx = 150 + i * 22;
        cv_text(bx + (i ? -3 : 0), 5, &FONT_T, L[i], C_GRAY);
        cv_rect(bx, KM_Y, 10, 96, C_LINE);
        for (y = KM_Y + 96 - clm_px[i]; y < KM_Y + 96; y++)
            cv_rect(bx, y, 10, 1, y < KM_Y + 6 ? RGB(150, 240, 190) : RGB(30, 204, 112));
    }
}

/* (BONBLOOP 1.7) a knob's dial for value c of the page: where its value sits in its range (0..1000; -1: no value) */
UI_SMALL static int32_t knob_ratio(uint32_t c)
{
    int16_t *vp;
    const param_desc_t *d = page_desc(cur_page(), c, &vp);
    if (!d || !vp || d->max <= d->min)
        return -1;
    return (clamp(*vp, d->min, d->max) - d->min) * 1000 / (d->max - d->min);
}
/* a page without a graph of its own (EDIT, VOICE, the DEST pages, GLOBAL, DELAY, DIST, MOD ...; SLOOP 2.4: its values
 * large): BONBLOOP 1.7, a knob under each column, its value and name below; one value alone (FILTER), one big knob in
 * the middle. The one turned in white. The columns (draw_columns, drawn first) fill ui.big_* */
/* (BONBLOOP 1.8) an effect's page (FX > DIST MOD DELAY REVERB FILTER CLIP; the synth's DRIVE PHASER FILTER CHORUS
 * DELAY REVERB): the value windows of before, 2 x 2 (one value: in the middle), the name small over the value large;
 * no knobs there (the user's call: the windows read better for an effect). Every other value page keeps its knobs */
UI_SMALL static int fx_windows(const page_t *pg)
{
    return !ui.home && pg->graph == GR_NONE && (pg->fam == FAM_FX || (pg->fam == FAM_SYN && syn_fx_of(pg)));
}
UI_SMALL static void graph_windows(uint32_t n)
{
    uint32_t c;
    if (n > 1u) {
        cv_rect(119, 6, 1, 112, C_LINE);
        cv_rect(6, 62, 228, 1, C_LINE);
    }
    for (c = 0; c < 4u; c++) {
        int32_t x0 = c & 1u ? 126 : 6, y0 = c & 2u ? 64 : 2, x;
        if (!ui.big_l[c][0])
            continue;
        if (n == 1u) {                                  /* one value: in the middle */
            x0 = (240 - text_w(&FONT_L, ui.big_v[c]) - (ui.big_u[c][0] ? text_w(&FONT_T, ui.big_u[c]) + 3 : 0)) / 2;
            y0 = 30;
        }
        cv_text(x0, y0 + 6, &FONT_T, ui.big_l[c], C_GRAY);
        x = cv_text(x0, y0 + 20, &FONT_L, ui.big_v[c], ui.big_c[c] == C_DIM ? C_DIM : ui.big_c[c] == C_WHITE ? C_WHITE : C_HI);
        if (ui.big_u[c][0])
            cv_text(x + 3, y0 + 36, &FONT_T, ui.big_u[c], C_DIM);   /* (on the value's baseline) */
    }
}
/* (BONBLOOP 1.8) knobs only for sound: a page of ENV, LFO, EDIT or the synth (not an effect's) whose every value is a
 * continuous sound parameter (a level, a time, a rate, a cutoff, a depth, semitones, an amount with its unit). A page
 * with anything to choose or set (a type, a slot, a mode, an engine, BPM, MIDI, the arp, the scale, the pattern,
 * GLOBAL ...): the screen in four windows, as before the knobs (the user's call) */
UI_SMALL static int knob_value(const page_t *pg, uint32_t c)
{
    int16_t *vp;
    const param_desc_t *d = page_desc(pg, c, &vp);
    if (!d || !vp || d->max <= d->min || d->names)
        return 0;
    switch (d->fmt) {
    case F_PCT: case F_BIPCT: case F_TIME: case F_LFOHZ: case F_CUTOFF: case F_DB: case F_SEMI:
        return 1;
    case F_INT:
        return d->unit != 0;                          /* (cents, dB, ms: an amount; a bare number is a setting) */
    default:
        return 0;
    }
}
UI_SMALL static int knob_page(const page_t *pg)
{
    uint32_t c, k = 0;
    if ((pg->fam != FAM_ENV && pg->fam != FAM_LFO && pg->fam != FAM_EDIT && pg->fam != FAM_SYN) || fx_windows(pg))
        return 0;
    for (c = 0; c < 4u; c++)
        if (ui.big_l[c][0]) {
            if (!knob_value(pg, c))
                return 0;
            k++;
        }
    return k != 0u;
}
UI_SMALL static void graph_big(void)
{
    uint32_t c, n = 0;
    for (c = 0; c < 4u; c++)
        n += ui.big_l[c][0] != 0;
    if (!knob_page(cur_page())) {
        graph_windows(n);
        return;
    }
    for (c = 0; c < 4u; c++) {
        int32_t cx = n == 1u ? 120 : 30 + 60 * (int32_t)c, r = n == 1u ? 32 : 21, cy = n == 1u ? 46 : 38, w, x;
        int hot = ui.big_c[c] == C_WHITE;
        char v[12];
        if (!ui.big_l[c][0])
            continue;
        te_dial(cx, cy, r, knob_ratio(c), hot ? C_WHITE : KNOB_LIT, C_LINE);
        fit(v, ui.big_v[c], &FONT_S, n == 1u ? 180 : 56);
        w = text_w(&FONT_S, v) + (ui.big_u[c][0] && n == 1u ? text_w(&FONT_T, ui.big_u[c]) + 2 : 0);
        x = cv_text(cx - w / 2, cy + r + 7, &FONT_S, v, hot ? C_WHITE : C_HI);
        if (ui.big_u[c][0] && n == 1u)
            cv_text(x + 2, cy + r + 11, &FONT_T, ui.big_u[c], C_DIM);
        w = text_w(&FONT_T, ui.big_l[c]);
        cv_text(cx - w / 2, cy + r + 27, &FONT_T, ui.big_l[c], C_GRAY);   /* (the name small, under) */
    }
}

UI_SMALL static void draw_graph(void)
{
    const page_t *pg = cur_page();
    const track_t *t = TSEL;
    uint16_t c = TE_COL[song.sel & 3u];               /* LIVE: the curves in the track's colour */
    uint32_t sig, top, drum_note = !ui.home && is_drum(t) && !page_for_drum(pg);
    if (!ui.home && pg->graph == GR_TRK) {
        draw_tracks();
        ui.graph_top = 1;                            /* the next graph draws its top rows again */
        return;
    }
    sig = graph_signature();
    if (!ui.force && sig == ui.graph_sig)
        return;
    ui.graph_sig = sig;
    cv_begin(240, H_GRAPH, C_BLACK);
    cv_oy = G_OY;
    if (ui.home) {
        graph_scope(c);
    } else if (drum_note) {                          /* a page the drum track has no use for */
        static const char *const L[2] = {"DRUM TRACK", "SEQ  TRACKS  GLO DRUMS"};
        cv_text((240 - text_w(&FONT_S, L[0])) / 2, 26, &FONT_S, L[0], C_HI);
        cv_text((240 - text_w(&FONT_S, L[1])) / 2, 52, &FONT_S, L[1], C_DIM);
    } else {
        switch (pg->graph) {
        case GR_ADSR:
            graph_adsr(t, c);
            break;
        case GR_EQ:                                  /* BONBLOOP 1.5 */
            graph_eq(t, c);
            break;
        case GR_LFO:
            graph_lfo(t, c);
            break;
        case GR_WAVE:                                /* BONBLOOP */
            graph_wave(t, c);
            break;
        case GR_STEPS:
            graph_steps(t, c);
            break;
        case GR_ROLL:
            cv_oy = 0;
            graph_roll(t, c);
            break;
        case GR_SCALE:
            graph_scale(t, c);
            break;
        case GR_FX:                                  /* (1.7: the faders, from the top) */
            cv_oy = 0;
            graph_fx(t, c);
            break;
        case GR_SLCR:
            graph_slicer(t, c);
            break;
        case GR_COMP:                                /* (BONBLOOP 1.7) */
            cv_oy = 0;
            graph_comp();
            break;
        case GR_CLIP:                                /* (1.9.2) */
            cv_oy = 0;
            graph_clip(t);
            break;
        case GR_BROWSE:
            cv_oy = 0;
            graph_browse();
            break;
        case GR_SLOTS:
            cv_oy = 0;
            graph_slots();
            break;
        case GR_USER:
            cv_oy = 0;
            graph_user();
            break;
        default:
            if (big_page(pg)) {
                cv_oy = 0;
                graph_big();
            }
            break;
        }
    }
    top = !ui.home && !drum_note && (pg->graph == GR_BROWSE || pg->graph == GR_SLOTS || pg->graph == GR_USER ||
                                     pg->graph == GR_ROLL || pg->graph == GR_COMP || pg->graph == GR_FX ||
                                     pg->graph == GR_CLIP || big_page(pg));   /* these draw from the top */
    cv_oy = 0;
    if (ui.hot_t && settings.zoom &&
        !(top && (big_page(pg) || pg->graph == GR_ROLL || pg->graph == GR_COMP || pg->graph == GR_FX ||
                  pg->graph == GR_CLIP) && !drum_note)) {   /* (the big values show it already) */                 /* focus (menu ZOOM): the touched value, large and white */
        int32_t x;
        top = 1;
        cv_rect(0, 0, 150, 50, C_BLACK);
        cv_text(4, 0, &FONT_S, ui.focus_l, C_GRAY);
        x = cv_text(4, 16, &FONT_L, ui.focus_v, C_WHITE);
        cv_text(x + 4, 30, &FONT_S, ui.focus_u, C_DIM);
    }
    /* graphs keep out of the top G_OY rows: skip them unless something is (or was) there */
    cv_blit_from(0, Y_GRAPH, top || ui.graph_top || ui.force ? 0u : G_OY);
    ui.graph_top = (uint8_t)top;
}

/* the page shown: its place in its family and how many (*n), as SELECT walks them (2.7: the screens of their own, SONG
 * and DRUMS, are not counted: SEQ is STEP 1/2, PATTERN 2/2; the page shown always counts) */
UI_SMALL static uint32_t page_pos(uint32_t *n)
{
    const page_t *pg = cur_page();
    uint32_t i, k = 0;
    *n = 0;
    for (i = 0; i < NPAGES; i++)
        if (PAGES[i].fam == pg->fam && (i == ui.page || (page_shown(&PAGES[i]) && PAGES[i].scope != SC_SONG &&
                                                         PAGES[i].scope != SC_DRUM))) {
            (*n)++;
            if (i == ui.page)
                k = *n;
        }
    return k;
}

UI_SMALL static void draw_foot(void)
{
    char s[48], pn[16], en[10], ti[20];
    const track_t *t = TSEL;
    uint32_t sig;
    const page_t *pg = cur_page();
    const engine_t *e = ENGINES[TSEL->eng_req % NENGINES];
    const char *ename = is_drum(t) ? "DRUM" : e->name;
    int32_t x;
    pn[0] = 0;
    if (is_drum(t))
        str_cpy(pn, DRUM_KIT_NAMES[drum_kit()], sizeof pn);
    else if (user_of(t) < UP_SLOTS)
        up_name(user_of(t), pn);                       /* a user preset */
    else if (e->npresets)
        str_cpy(pn, e->presets[TSEL->preset % e->npresets].name, sizeof pn);
    if (ui.home) {
        str_cpy(ti, "HOME", sizeof ti);
    } else {                                           /* page title + number in its family: "ENV DEST 2/2" */
        uint32_t n, k = page_pos(&n);
        const char *pt = pg->scope == SC_ENGINE ? e->page_title[pg->id[0] != P_E0] : 0;   /* EDIT: the engine's */
        str_cpy(ti, pt ? pt : pg->title, 10);
        if (n > 1) {
            str_cpy(ti + str_len(ti), " ", 4);
            fmt_int(ti + str_len(ti), (int32_t)k);
            str_cpy(ti + str_len(ti), "/", 4);
            fmt_int(ti + str_len(ti), (int32_t)n);
        }
    }
    str_cpy(s, ename, sizeof s);
    s[str_len(s) + 1u] = 0;
    s[str_len(s)] = (char)('1' + song.sel);
    str_cpy(s + str_len(s), pn, 16);
    str_cpy(s + str_len(s), ti, sizeof ti);
    {   /* step markers: the playhead only when it is in the shown bank, the cursor only in SEQ */
        uint32_t ph = song.playing && t->seq_idx / 16u == ui.bank ? t->seq_idx : 0xFFu;
        sig = str_hash(0x9E3779B9u, s) + ph * 97u + (song.seq_mode ? ui.cursor : 0xFFu) * 3001u + steps_hash(t) +
              ui.bank * 7u + (uint32_t)t->p[P_SLEN] * 13u;
    }
    if (!ui.force && sig == ui.foot_sig)
        return;
    ui.foot_sig = sig;
    cv_begin(240, H_FOOT, C_BLACK);
    {
        uint32_t i;
        for (i = 0; i < 16u; i++) {                   /* row 1: the cursor's bank as 16 thin bars */
            uint32_t si = ui.bank * 16u + i;
            int32_t sx = 6 + (int32_t)i * 14 + (int32_t)(i / 4u) * 4;
            const step_t *st = &t->step[si];
            if (si >= (uint32_t)t->p[P_SLEN])
                continue;
            if (step_on(st))
                cv_rect(sx, 2, 2, 9, TE_COL[song.sel & 3u]);
            else
                cv_rect(sx, 10, 1, 1, C_DIM);
            if ((song.playing && si == t->seq_idx) || (song.seq_mode && si == ui.cursor))
                cv_rect(sx - 1, 13, 3, 3, C_WHITE);
        }
    }
    x = 4;
    if (FELUCCA_ICONS) {                              /* row 2: engine icon + name, preset, page */
        cv_icon(x, 21, engine_icon(ename), C_GRAY);
        x += 14;
    }
    fit(en, ename, &FONT_S, 90 - (x - 4));
    x = cv_text(x, 20, &FONT_S, en, C_HI);
    {
        char pf[16];
        int32_t room = 236 - text_w(&FONT_S, ti) - 10 - (x + 10);
        str_cpy(pf, pn, sizeof pf);
        while (pf[0] && text_w(&FONT_S, pf) > room)
            pf[str_len(pf) - 1u] = 0;
        cv_text(x + 10, 20, &FONT_S, pf, C_AMB);
    }
    cv_text(236 - text_w(&FONT_S, ti), 20, &FONT_S, ti, C_GRAY);
    cv_blit(0, Y_FOOT);
}
UI_SMALL static void draw_columns(void)
{
    uint32_t c;
    char val[12];
    const char *unit;
    if (ui.home) {
        for (c = 0; c < 4u; c++) {
            int16_t *vp;
            const param_desc_t *d = home_param(c, &vp);
            param_format(d, *vp, val, &unit);
            draw_column(c, d->label, val, unit, VAL(c), RATIO(d, *vp), param_icon(d, *vp));
        }
        return;
    }
    if (is_drum(TSEL) && !page_for_drum(cur_page())) {   /* "DRUM TRACK" (the graph says so) */
        for (c = 0; c < 4u; c++)
            draw_column(c, "", "", "", C_HI, -1, ICON_AUTO);
        return;
    }
    if (cur_page()->scope == SC_TRK) {                 /* TRACK LEVEL LEN PAN of the selected track */
        const track_t *t = TSEL;
        uint32_t lvl = trk_level(song.sel);
        fmt_int(val, (int32_t)song.sel + 1);
        draw_column(0, "TRACK", val, "/4", VAL(0u), (int32_t)song.sel * 1000 / (NTRK - 1), ICON_AUTO);
        if (!lvl || t->p[P_MUTE]) {                    /* (MUTE: a turn of KNOB 2 unmutes, tracks_edit) */
            str_cpy(val, "MUTE", 12);
            unit = "";
        } else if (is_drum(t)) {
            fmt_int(val, (int32_t)lvl);
            unit = "";
        } else {
            param_format(&TP[P_LEVEL], (int32_t)lvl, val, &unit);
        }
        draw_column(1, "LEVEL", val, unit, lvl && !t->p[P_MUTE] ? VAL(1u) : C_DIM, (int32_t)lvl * 1000 / 127, ICON_AUTO);
        param_format(&TP[P_SLEN], t->p[P_SLEN], val, &unit);
        draw_column(2, "LEN", val, unit, VAL(2u), RATIO(&TP[P_SLEN], t->p[P_SLEN]), ICON_AUTO);
        param_format(&TP[P_PAN], t->p[P_PAN], val, &unit);
        draw_column(3, "PAN", val, unit, VAL(3u), RATIO(&TP[P_PAN], t->p[P_PAN]), param_icon(&TP[P_PAN], t->p[P_PAN]));
        return;
    }
    if (cur_page()->graph == GR_BROWSE) {
        uint32_t total, cur = preset_pos(&total);
        char u[8];
        fmt_int(val, (int32_t)cur + 1);
        str_cpy(u, "/", 8);
        fmt_int(u + 1, (int32_t)total);
        draw_column(0, "No.", val, u, VAL(0u), -1, ICON_NONE);
        draw_column(1, "ENG", ENGINES[TSEL->eng_req]->name, "", VAL(1u), -1, engine_icon(ENGINES[TSEL->eng_req]->name));
        draw_column(2, "", "", "", C_HI, -1, ICON_AUTO);
        draw_column(3, "FAV", fav_has(fav_key()) ? "ON" : "--", "", VAL(3u), -1, ICON_NONE);   /* (2.7) KNOB 4 */
        return;
    }
    if (cur_page()->graph == GR_USER) {                  /* SLOT, then three GO buttons */
        int used = up_used(ui.uslot);
        up_slot_label(val, ui.uslot);
        draw_column(0, "SLOT", val, "", VAL(0u), (int32_t)ui.uslot * 1000 / (int32_t)(UP_SLOTS - 1u), ICON_AUTO);
        draw_column(1, "LOAD", "--", "", used ? C_HI : C_DIM, -1, ICON_AUTO);
        draw_column(2, "ERASE", "--", "", used ? C_HI : C_DIM, -1, ICON_AUTO);
        draw_column(3, "SAVE", "--", "", C_HI, -1, ICON_AUTO);
        return;
    }
    if (cur_page()->scope == SC_STEP) {                 /* (2.7.2) NOTES: NOTE PITCH LEN FORCE of the note under edit */
        const step_t *st = &TSEL->step[ui.cursor];
        char sn[8], sl[8], u[8], ln[8];
        int on = step_on(st);
        uint32_t k = on ? ui.note_k % st->n : 0u, ms;
        fmt_int(sn, (int32_t)ui.cursor + 1);
        str_cpy(sl, "/", 8);
        fmt_int(sl + 1, TSEL->p[P_SLEN]);
        draw_column(0, "NOTE", sn, sl, VAL(0u), -1, ICON_AUTO);
        u[0] = 0;
        if (on) {
            note_name(val, st->note[k]);
            if (st->n > 1u) {                             /* a chord: which of its notes */
                fmt_int(u, (int32_t)k + 1);
                str_cpy(u + str_len(u), "/", 2);
                fmt_int(u + str_len(u), st->n);
            }
        } else {
            str_cpy(val, "--", 12);
        }
        draw_column(1, "PITCH", val, u, on ? VAL(1u) : C_DIM, -1, ICON_AUTO);
        ms = on ? note_ms(TSEL, ui.cursor) : 0u;        /* (BONBLOOP 1.9.2: in ms, not steps; from 10 s on, in s) */
        if (on)
            fmt_int(ln, (int32_t)(ms < 10000u ? ms : ms / 1000u));
        else
            str_cpy(ln, "--", 8);
        draw_column(2, "LEN", ln, !on ? "" : ms < 10000u ? "MS" : "S", on ? VAL(2u) : C_DIM, -1, ICON_AUTO);
        draw_column(3, "FORCE", on ? FORCE_N[note_force(st, k)] : "--", "", on ? VAL(3u) : C_DIM, -1, ICON_AUTO);
        return;
    }
    for (c = 0; c < 4u; c++) {
        int16_t *vp;
        const param_desc_t *d = page_desc(cur_page(), c, &vp);
        if (cur_page()->graph == GR_SLOTS && c == 1u) {   /* 2.6: ERASE, lit while the slot holds a project */
            draw_column(1, "ERASE", "--", "", project_used((uint32_t)song.g[G_SLOT] - 1u) ? C_HI : C_DIM, -1, ICON_AUTO);
            continue;
        }
        if (!d || !d->label || d->label[0] == '-') {
            draw_column(c, "", "", "", C_HI, -1, ICON_AUTO);
            continue;
        }
        if (cur_page()->id[c] == G_INFO && cur_page()->scope == SC_GLOBAL && !usb.config) {   /* not connected: the
                                                       * USB status instead of the CPU (2.4: MIDI is MIDI OUT, ROUT is IN) */
            str_cpy(val, !usb.up ? "OFF" : usb.config ? "MIDI" : usb.setups ? "ENUM" : usb.sof_seen ? "BUS" : "WAIT", 12);
            unit = "USB";
            draw_column(c, "USB", val, unit, C_HI, -1, ICON_AUTO);
            continue;
        }
        if (cur_page()->id[c] == G_INFO && cur_page()->scope == SC_GLOBAL) {
            fmt_int(val, (int32_t)(song.cpu_q8 * 100u / 256u));
            unit = "%";
        } else {
            param_format(d, *vp, val, &unit);
        }
        draw_column(c, d->label, val, unit, VAL(c), d->fmt == F_ENUM && d->max < 2 ? -1 : RATIO(d, *vp),
                    param_icon(d, *vp));
    }
}


/* the UI's timers, once a frame whatever the screen (a message, the BPM highlight, an armed GO, the
 * white value of the knob just turned) */
UI_SMALL static void ui_timers(void)
{
    if (ui.msg_t)
        ui.msg_t--;
    if (ui.bpm_t)
        ui.bpm_t--;
    if (ui.arm_t && !--ui.arm_t)
        ui.arm = 0;
    if (ui.hot_t)
        ui.hot_t--;
    if (syn_splash)                                  /* (BONBLOOP: the SYNTH MODE's title over the wavetable) */
        syn_splash--;
}

static void vis_draw(void);                     /* ui_vis.c */
static int vis_shown(void);
static uint8_t vis_on, vis_shown_last, vis_name_t;   /* the visualiser (ui_vis.c): on, drawn last frame, its name's time */
UI_SMALL static void ui_draw(void)
{
    ui.frame++;
    pads_tick();
    if (rec_go) {                                       /* the take started: say so */
        rec_go = 0;
        ui_message("RECORDING");
    }
    if (ft_bars) {                                      /* a free take closed: the loop it made */
        char m[24];
        uint32_t n = ft_bars;
        ft_bars = 0;
        if (n == 0xFFu) {
            ui_message("TAKE DROPPED");
        } else {
            str_cpy(m, n == 1u ? "LOOP 1 BAR " : n == 2u ? "LOOP 2 BARS " : "LOOP 4 BARS ", sizeof m);
            fmt_int(m + str_len(m), song.g[G_BPM]);
            str_cpy(m + str_len(m), " BPM", sizeof m - str_len(m));
            ui_message(m);
        }
    }
    if (er_flash) {                                     /* EDIT + key took something out */
        er_flash = 0;
        if (!ui.msg_t)
            ui_message("ERASED");
    }
    if (!ui.menu && (ui.layer != LY_PLAY || ui.hold_kind)) {   /* a layer held / a hold to confirm */
        if (ui.hold_kind)
            hold_screen_draw();
        else
            layer_screen_draw();
        ui_timers();
        ui.force = 0;
        return;
    }
    if (layer_shown) {                                  /* back to the page */
        layer_shown = 0;
        lcd_fill(0, 0, 240, 240, C_BLACK);
        ui.force = 1;
    }
    if ((rec_wait || ft_on) && !ui.menu && !on_song_page()) {   /* armed / free take */
        rec_screen_draw();
        ui_timers();
        ui.force = 0;
        return;
    }
    if (rec_shown) {                                    /* back to the page */
        rec_shown = 0;
        lcd_fill(0, 0, 240, 240, C_BLACK);
        ui.force = 1;
    }
#if FELUCCA_ARRANGER
    if (!ui.menu && on_song_page()) {
        song_screen_draw();
        ui_timers();
        ui.force = 0;
        return;
    }
#endif
    if (ui.menu) {
        draw_menu();
        ui_timers();
        ui.force = 0;
        return;
    }
    if (vis_shown()) {                                  /* the visualiser over the TRACKS screen */
        vis_draw();
        ui_timers();
        ui.force = 0;
        return;
    }
    if (vis_shown_last) {                               /* back from it */
        vis_shown_last = 0;
        vis_on = 0;
        lcd_fill(0, 0, 240, 240, C_BLACK);
        ui.force = 1;
    }
    if (!ui.home && cur_page()->scope == SC_TRK) {
        studio_tracks_draw();
        ui_timers();
        ui.force = 0;
        return;
    }
    if (on_drum_page()) {
        drum_screen_draw();
        ui_timers();
        ui.force = 0;
        return;
    }
    cursor_fix();
    if (ui.force)
        draw_frame();
    felucca_dbg.stage = 3;
    draw_head();
    felucca_dbg.stage = 4;
    draw_columns();
    felucca_dbg.stage = 5;
    draw_graph();
    ui_timers();
    felucca_dbg.stage = 6;
    draw_foot();
    ui.force = 0;
}
