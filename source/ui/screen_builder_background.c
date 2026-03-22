#include "screen_builder.h"
#include "ui_core.h"
#include "../data/srd_backgrounds.h"
#include "../data/srd_races.h"
#include "../utils/dnd_rules.h"
#include <stdio.h>
#include <string.h>

// ---- Zustand ---------------------------------------------------------------
static UiList   s_list;
static UiButton s_btn_next;
static UiButton s_btn_back;

// ASI-Modus: 0 = Hintergrund-Auswahl, 1 = ASI-Verteilung
static int s_asi_mode = 0;
static int s_asi_cursor = 0;  // 0-5 Attribut-Cursor im ASI-Modus

static int asi_total(void) {
    int sum = 0;
    for (int i = 0; i < ABILITY_COUNT; i++)
        sum += g_builder.bg_asi[i];
    return sum;
}

// ---- Initialisierung -------------------------------------------------------

void step_background_init(void) {
    s_asi_mode  = 0;
    s_asi_cursor = 0;

    ui_list_init(&s_list, 4, 28, 312, 166, 24);
    for (int i = 0; i < SRD_BACKGROUND_COUNT; i++) {
        char row[128];
        const SrdBackground* bg = &SRD_BACKGROUNDS[i];
        char skills[64] = "";
        for (int j = 0; j < bg->skill_prof_count; j++) {
            if (j > 0) strncat(skills, ", ", sizeof(skills) - strlen(skills) - 1);
            strncat(skills, SKILL_NAMES[bg->skill_profs[j]],
                    sizeof(skills) - strlen(skills) - 1);
        }
        snprintf(row, sizeof(row), "%-14s %s", bg->name, skills);
        ui_list_set_item(&s_list, i, row, COLOR_TEXT);
    }

    if (g_builder.bg_idx >= 0)
        s_list.selected = g_builder.bg_idx;
    else
        s_list.selected = 0;

    s_btn_back = (UiButton){ 4,   202, 100, 26, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_next = (UiButton){ 210, 202, 106, 26, "Weiter >",  COLOR_ACCENT,     COLOR_TEXT, 0 };
}

// ---- Top-Screen: Hintergrund-Details oder ASI-Vorschau --------------------

void step_background_draw_top(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    if (!s_asi_mode)
        ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Schritt 4/6: Hintergrund");
    else
        ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Schritt 4/6: Hintergrunds-Boni");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    if (!s_asi_mode) {
        // --- Hintergrund-Vorschau (unverändert) ---
        int sel = s_list.selected;
        if (sel < 0 || sel >= SRD_BACKGROUND_COUNT) {
            ui_draw_text(10, 60, 0.5f, 0.42f, COLOR_TEXT_DIM, "Waehle einen Hintergrund.");
            return;
        }
        const SrdBackground* bg = &SRD_BACKGROUNDS[sel];
        ui_draw_textf(10, 34, 0.5f, 0.60f, COLOR_TEXT, "%s", bg->name);
        ui_draw_textf(10, 55, 0.5f, 0.38f, COLOR_TEXT_DIM, "%s", bg->description);
        ui_draw_separator(0, 70, SCREEN_TOP_W, COLOR_TEXT_DIM);
        ui_draw_text(10, 76, 0.5f, 0.40f, COLOR_TEXT, "Fertigkeits-Uebungen:");
        char skills_str[128] = "";
        for (int j = 0; j < bg->skill_prof_count; j++) {
            if (j > 0) strncat(skills_str, ", ", sizeof(skills_str) - strlen(skills_str) - 1);
            strncat(skills_str, SKILL_NAMES[bg->skill_profs[j]],
                    sizeof(skills_str) - strlen(skills_str) - 1);
        }
        ui_draw_textf(10, 90, 0.5f, 0.42f, COLOR_GOLD, "  %s", skills_str);
        int y = 108;
        if (bg->tool_profs && bg->tool_profs[0]) {
            ui_draw_textf(10, y, 0.5f, 0.38f, COLOR_TEXT_DIM, "Werkzeuge: %s", bg->tool_profs); y += 14;
        }
        if (bg->languages && bg->languages[0]) {
            ui_draw_textf(10, y, 0.5f, 0.38f, COLOR_TEXT_DIM, "Sprachen: %s", bg->languages); y += 14;
        }
        ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM); y += 4;
        ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_ACCENT2, "Merkmal: %s", bg->feature_name); y += 16;
        const char* fd = bg->feature_desc;
        int cut = 59;
        while (cut > 0 && fd[cut] && fd[cut] != ' ') cut--;
        if (!fd[cut]) cut = 59;
        char line1[64];
        int flen = (int)strlen(fd);
        if (flen <= 60) {
            ui_draw_textf(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, "%s", fd);
        } else {
            snprintf(line1, sizeof(line1), "%.*s", cut, fd);
            ui_draw_textf(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, "%s", line1); y += 11;
            ui_draw_textf(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, "%s", fd + cut + 1);
        }
    } else {
        // --- ASI-Vorschau ---
        ui_draw_text(10, 34, 0.5f, 0.42f, COLOR_TEXT_DIM,
            "D&D 2024: Hintergrund gibt +2/+1 auf Attribute deiner Wahl.");
        ui_draw_text(10, 50, 0.5f, 0.38f, COLOR_TEXT_DIM,
            "Verteile: +2 auf ein Attribut und +1 auf ein anderes");
        ui_draw_text(10, 62, 0.5f, 0.38f, COLOR_TEXT_DIM,
            "(oder +1/+1/+1 auf drei verschiedene).");
        ui_draw_separator(0, 78, SCREEN_TOP_W, COLOR_TEXT_DIM);

        // Alle 6 Attribute mit aktuellem Basiswert, Rassen-Bonus, HG-Bonus, Gesamt
        ui_draw_text(10,  84, 0.5f, 0.36f, COLOR_TEXT_DIM, "Attribut");
        ui_draw_text(100, 84, 0.5f, 0.36f, COLOR_TEXT_DIM, "Basis");
        ui_draw_text(145, 84, 0.5f, 0.36f, COLOR_TEXT_DIM, "Rasse");
        ui_draw_text(195, 84, 0.5f, 0.36f, COLOR_TEXT_DIM, "HG");
        ui_draw_text(240, 84, 0.5f, 0.36f, COLOR_TEXT_DIM, "Ges.");
        ui_draw_text(285, 84, 0.5f, 0.36f, COLOR_TEXT_DIM, "Mod");

        for (int i = 0; i < ABILITY_COUNT; i++) {
            float y = 96 + i * 22;
            int base = g_builder.abilities[i];
            int bg_bonus = g_builder.bg_asi[i];

            // Rassenboni ohne bg_asi berechnen
            int race_bonus = 0;
            if (g_builder.race_idx >= 0) {
                const SrdRace* r = &SRD_RACES[g_builder.race_idx];
                race_bonus += r->asi[i];
                if (g_builder.subrace_idx >= 0 && g_builder.subrace_idx < r->subrace_count)
                    race_bonus += r->subraces[g_builder.subrace_idx].asi[i];
            }
            int total = base + race_bonus + bg_bonus;
            int mod   = dnd_modifier(total);

            u32 col = (i == s_asi_cursor) ? COLOR_GOLD : COLOR_TEXT;
            if (i == s_asi_cursor)
                ui_draw_rect(0, y - 2, SCREEN_TOP_W, 20, COLOR_PANEL2);

            ui_draw_textf(10,  y, 0.5f, 0.40f, col, "%s", ABILITY_SHORT[i]);
            ui_draw_textf(100, y, 0.5f, 0.40f, col, "%2d", base);
            if (race_bonus != 0)
                ui_draw_textf(145, y, 0.5f, 0.40f, COLOR_ACCENT2, "%+d", race_bonus);
            else
                ui_draw_textf(145, y, 0.5f, 0.40f, COLOR_TEXT_DIM, "--");
            if (bg_bonus > 0)
                ui_draw_textf(195, y, 0.5f, 0.40f, COLOR_HP_GREEN, "+%d", bg_bonus);
            else
                ui_draw_textf(195, y, 0.5f, 0.40f, COLOR_TEXT_DIM, "--");
            ui_draw_textf(240, y, 0.5f, 0.44f, col, "%2d", total);
            ui_draw_textf(285, y, 0.5f, 0.40f,
                          mod >= 0 ? COLOR_HP_GREEN : COLOR_HP_RED, "%+d", mod);
        }

        // Budget-Anzeige
        int remaining = 3 - asi_total();
        u32 bud_col = remaining == 0 ? COLOR_HP_GREEN : COLOR_GOLD;
        ui_draw_textf(10, 232, 0.5f, 0.38f, bud_col, "Verbleibende Punkte: %d", remaining);
    }
}

// ---- Bottom-Screen ---------------------------------------------------------

void step_background_draw_bottom(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    if (!s_asi_mode) {
        ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_TEXT_DIM, "Hintergrund waehlen");
        ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);
        ui_list_draw(&s_list);
        ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);
        s_btn_next.color_bg = (s_list.selected >= 0) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
        ui_button_draw(&s_btn_back);
        ui_button_draw(&s_btn_next);
        ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM,
                     "[DPad] Nav  [A/Tip] Auswaehlen  [B] Zurueck");
    } else {
        // ASI-Modus: +/- Buttons pro Attribut
        ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_GOLD, "Hintergrunds-Boni verteilen (3 Punkte)");
        ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

        // Sechs Attribut-Zeilen mit +/- Anzeige
        for (int i = 0; i < ABILITY_COUNT; i++) {
            float y = 30 + i * 22;
            u32 col = (i == s_asi_cursor) ? COLOR_GOLD : COLOR_TEXT;
            if (i == s_asi_cursor)
                ui_draw_rect(0, y - 1, SCREEN_BTM_W, 22, COLOR_PANEL2);

            ui_draw_textf(8,   y + 3, 0.5f, 0.42f, col, "%-3s", ABILITY_SHORT[i]);
            ui_draw_textf(56,  y + 3, 0.5f, 0.42f, col, "+%d", g_builder.bg_asi[i]);
            if (i == s_asi_cursor) {
                ui_draw_text(120, y + 3, 0.5f, 0.42f, COLOR_TEXT_DIM, "[ L -    R + ]");
            }
        }

        ui_draw_separator(0, 164, SCREEN_BTM_W, COLOR_TEXT_DIM);

        // "Ueberspringen" link / info
        int remaining = 3 - asi_total();
        u32 bud_col = remaining == 0 ? COLOR_HP_GREEN : COLOR_GOLD;
        ui_draw_textf(8, 167, 0.5f, 0.38f, bud_col, "Punkte: %d/3 vergeben", asi_total());

        // Buttons
        s_btn_back.label = "< Zurueck";
        s_btn_next.color_bg = (remaining == 0) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
        s_btn_next.label = "Weiter >";
        s_btn_back = (UiButton){ 4,   182, 100, 22, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_next = (UiButton){ 214, 182, 102, 22, "Weiter >",  remaining == 0 ? COLOR_ACCENT : COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        ui_button_draw(&s_btn_back);
        ui_button_draw(&s_btn_next);

        ui_draw_text(4, 208, 0.5f, 0.32f, COLOR_TEXT_DIM,
                     "[DPad] Attribut  [L/R] +/-  [B] Zurueck");
        ui_draw_text(4, 220, 0.5f, 0.32f, COLOR_TEXT_DIM,
                     "[START] Ueberspringen (kein Hintergrunds-Bonus)");
    }
}

// ---- Update ----------------------------------------------------------------

static void asi_change(int delta) {
    int cur = g_builder.bg_asi[s_asi_cursor];
    int new_val = cur + delta;
    if (new_val < 0) return;
    if (new_val > 2) return;                      // max +2 auf ein Attribut
    if (delta > 0 && asi_total() >= 3) return;    // Budget erschoepft
    g_builder.bg_asi[s_asi_cursor] = new_val;
}

void step_background_update(u32 keys_down, u32 keys_held,
                            touchPosition* touch, int touch_down) {
    (void)keys_held;

    if (!s_asi_mode) {
        // ---- Modus 1: Hintergrund-Auswahl ----
        if (keys_down & KEY_B) { builder_prev_step(); return; }

        if (keys_down & KEY_DOWN) { s_list.selected++; ui_list_scroll(&s_list, 1); }
        if (keys_down & KEY_UP)   { s_list.selected--; ui_list_scroll(&s_list, -1); }
        if (s_list.selected < 0)                       s_list.selected = 0;
        if (s_list.selected >= SRD_BACKGROUND_COUNT)   s_list.selected = SRD_BACKGROUND_COUNT - 1;

        if (keys_down & KEY_A && s_list.selected >= 0) {
            g_builder.bg_idx = s_list.selected;
            // Wechsel zu ASI-Modus
            memset(g_builder.bg_asi, 0, sizeof(g_builder.bg_asi));
            s_asi_mode  = 1;
            s_asi_cursor = 0;
            return;
        }

        if (!touch_down) return;

        int old_sel = s_list.selected;
        ui_list_handle_touch(&s_list, touch, touch_down);

        if (ui_button_touched(&s_btn_next, touch) && s_list.selected >= 0) {
            g_builder.bg_idx = s_list.selected;
            memset(g_builder.bg_asi, 0, sizeof(g_builder.bg_asi));
            s_asi_mode  = 1;
            s_asi_cursor = 0;
            return;
        }
        if (ui_button_touched(&s_btn_back, touch)) { builder_prev_step(); return; }

        if (ui_point_in_rect((float)touch->px, (float)touch->py,
                             s_list.x, s_list.y, s_list.w, s_list.h)
            && s_list.selected == old_sel && s_list.selected >= 0) {
            g_builder.bg_idx = s_list.selected;
            memset(g_builder.bg_asi, 0, sizeof(g_builder.bg_asi));
            s_asi_mode  = 1;
            s_asi_cursor = 0;
        }

    } else {
        // ---- Modus 2: ASI-Verteilung ----

        // B = zurück zur Hintergrund-Auswahl
        if (keys_down & KEY_B) {
            s_asi_mode = 0;
            return;
        }

        // START = überspringen (keine Hintergrunds-Boni)
        if (keys_down & KEY_START) {
            memset(g_builder.bg_asi, 0, sizeof(g_builder.bg_asi));
            builder_next_step();
            return;
        }

        // Cursor navigieren
        if (keys_down & KEY_DOWN) s_asi_cursor = (s_asi_cursor + 1) % ABILITY_COUNT;
        if (keys_down & KEY_UP)   s_asi_cursor = (s_asi_cursor - 1 + ABILITY_COUNT) % ABILITY_COUNT;

        // ASI +/-
        if (keys_down & KEY_R || keys_down & KEY_RIGHT) asi_change(+1);
        if (keys_down & KEY_L || keys_down & KEY_LEFT)  asi_change(-1);

        // A = weiter (wenn alles vergeben)
        if (keys_down & KEY_A && asi_total() == 3) {
            builder_next_step();
            return;
        }

        if (!touch_down) return;

        // Attribut-Zeile antippen
        for (int i = 0; i < ABILITY_COUNT; i++) {
            float y = 30 + i * 22;
            if (ui_point_in_rect((float)touch->px, (float)touch->py,
                                 0, y - 1, SCREEN_BTM_W, 22)) {
                if (s_asi_cursor == i) {
                    if (touch->px > SCREEN_BTM_W / 2)
                        asi_change(+1);
                    else
                        asi_change(-1);
                } else {
                    s_asi_cursor = i;
                }
                return;
            }
        }

        // "Weiter"-Button: nur wenn 3 Punkte vergeben
        if (ui_point_in_rect((float)touch->px, (float)touch->py, 214, 182, 102, 22)
            && asi_total() == 3) {
            builder_next_step();
            return;
        }
        // "Zurück"-Button
        if (ui_point_in_rect((float)touch->px, (float)touch->py, 4, 182, 100, 22)) {
            s_asi_mode = 0;
            return;
        }
    }
}
