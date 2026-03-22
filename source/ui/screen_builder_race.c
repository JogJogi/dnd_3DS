#include "screen_builder.h"
#include "ui_core.h"
#include "../data/srd_races.h"
#include "../utils/dnd_rules.h"
#include <stdio.h>
#include <string.h>

// ---- Zustand ---------------------------------------------------------------
static UiList  s_list;
static UiButton s_btn_next;
static UiButton s_btn_back;

// Unterrassen-Auswahl (wenn Rasse Unterrassen hat)
static int      s_show_subrace = 0;
static UiList   s_subrace_list;

// ---- Hilfsfunktionen -------------------------------------------------------

// Zeile fuer die Rassen-Liste aufbauen: "Aarakocra    +2 DEX +1 WIS"
static void build_race_row(int idx, char* buf, int buflen) {
    const SrdRace* r = &SRD_RACES[idx];
    char asi[48];
    srd_race_asi_summary(r, NULL, asi, sizeof(asi));
    snprintf(buf, buflen, "%-16s %s", r->name, asi);
}

// ---- Initialisierung -------------------------------------------------------

void step_race_init(void) {
    s_show_subrace = 0;

    // Listen-Widget fuer Rassen (4 = x, 28 = y, 312 = w, 166 = h, 24 = item_h)
    ui_list_init(&s_list, 4, 28, 312, 166, 24);
    for (int i = 0; i < SRD_RACE_COUNT; i++) {
        char row[128];
        build_race_row(i, row, sizeof(row));
        ui_list_set_item(&s_list, i, row, COLOR_TEXT);
    }

    // Vorherige Auswahl wiederherstellen
    if (g_builder.race_idx >= 0)
        s_list.selected = g_builder.race_idx;
    else
        s_list.selected = 0;

    s_btn_back = (UiButton){ 4,   202, 100, 26, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_next = (UiButton){ 210, 202, 106, 26, "Weiter >",  COLOR_ACCENT,     COLOR_TEXT, 0 };
}

// ---- Unterrassen-Init ------------------------------------------------------
static void init_subrace_list(int race_idx) {
    const SrdRace* r = &SRD_RACES[race_idx];
    ui_list_init(&s_subrace_list, 4, 50, 312, 140, 24);
    for (int i = 0; i < r->subrace_count; i++) {
        char row[128];
        char asi[48];
        // Unterrassen-ASI auf Basis-ASI addieren fuer Anzeige
        srd_race_asi_summary(r, &r->subraces[i], asi, sizeof(asi));
        snprintf(row, sizeof(row), "%-16s %s", r->subraces[i].name, asi);
        ui_list_set_item(&s_subrace_list, i, row, COLOR_TEXT);
    }
    s_subrace_list.selected = (g_builder.subrace_idx >= 0)
                              ? g_builder.subrace_idx : 0;
}

// ---- Top-Screen: Rassen-Details --------------------------------------------

void step_race_draw_top(void) {
    // Header
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD,
                  "Schritt 1/6: Rasse waehlen");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    int sel = s_show_subrace ? g_builder.race_idx : s_list.selected;
    if (sel < 0 || sel >= SRD_RACE_COUNT) {
        ui_draw_text(10, 60, 0.5f, 0.42f, COLOR_TEXT_DIM, "Waehle eine Rasse aus der Liste.");
        return;
    }

    const SrdRace* r = &SRD_RACES[sel];
    const Subrace* sub = NULL;
    if (s_show_subrace && g_builder.subrace_idx >= 0)
        sub = &r->subraces[g_builder.subrace_idx];
    else if (s_show_subrace && r->subrace_count > 0)
        sub = &r->subraces[s_subrace_list.selected];

    // Name gross
    ui_draw_textf(10, 34, 0.5f, 0.60f, COLOR_TEXT, "%s",
                  sub ? sub->name : r->name);

    // Groesse und Geschwindigkeit
    char extra[64] = "";
    if (r->extra_speed)
        snprintf(extra, sizeof(extra), ", %s", r->extra_speed);
    ui_draw_textf(10, 57, 0.5f, 0.40f, COLOR_TEXT_DIM,
                  "%s  |  %d Fuss Geh.%s", r->size, r->speed, extra);

    // ASI
    char asi_buf[64];
    srd_race_asi_summary(r, sub, asi_buf, sizeof(asi_buf));
    ui_draw_textf(10, 74, 0.5f, 0.40f, COLOR_GOLD, "Attribute: %s", asi_buf);

    // Sprachen
    ui_draw_textf(10, 90, 0.5f, 0.38f, COLOR_TEXT_DIM,
                  "Sprachen: %s", r->languages);

    ui_draw_separator(0, 106, SCREEN_TOP_W, COLOR_TEXT_DIM);

    // Fähigkeiten
    ui_draw_text(10, 110, 0.5f, 0.40f, COLOR_TEXT, "Faehigkeiten:");
    int y = 126;

    // Basis-Traits der Rasse
    for (int i = 0; i < r->trait_count && y < 230; i++) {
        ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_GOLD, "* %s", r->traits[i].name);
        y += 14;
        // Beschreibung in zwei Zeilen aufteilen wenn zu lang (>55 Zeichen)
        const char* desc = r->traits[i].description;
        int dlen = (int)strlen(desc);
        if (dlen <= 55) {
            ui_draw_textf(18, y, 0.5f, 0.35f, COLOR_TEXT_DIM, "%s", desc);
            y += 13;
        } else {
            // Zeile 1 (erste 55 Zeichen bis letztem Leerzeichen)
            char line[64];
            int cut = 54;
            while (cut > 0 && desc[cut] != ' ') cut--;
            if (cut == 0) cut = 54;
            snprintf(line, sizeof(line), "%.*s", cut, desc);
            ui_draw_textf(18, y, 0.5f, 0.35f, COLOR_TEXT_DIM, "%s", line);
            y += 12;
            ui_draw_textf(18, y, 0.5f, 0.35f, COLOR_TEXT_DIM, "%s", desc + cut + 1);
            y += 13;
        }
    }

    // Unterrassen-Traits (falls ausgewaehlt)
    if (sub) {
        for (int i = 0; i < sub->trait_count && y < 230; i++) {
            ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_ACCENT2,
                          "* %s", sub->traits[i].name);
            y += 14;
            ui_draw_textf(18, y, 0.5f, 0.35f, COLOR_TEXT_DIM,
                          "%s", sub->traits[i].description);
            y += 13;
        }
    }

    // Beschreibung der Rasse ganz unten
    if (y < 220) {
        ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM);
        y += 4;
        ui_draw_textf(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, "%s", r->description);
    }
}

// ---- Bottom-Screen ---------------------------------------------------------

void step_race_draw_bottom(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);

    if (s_show_subrace) {
        // Unterrassen-Auswahl
        ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_TEXT, "Unterrasse waehlen");
        ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

        // Rassen-Name als Kontext
        ui_draw_textf(8, 30, 0.5f, 0.38f, COLOR_TEXT_DIM,
                      "Rasse: %s", SRD_RACES[g_builder.race_idx].name);
        ui_list_draw(&s_subrace_list);
    } else {
        ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_TEXT_DIM, "Rasse waehlen");
        ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);
        ui_list_draw(&s_list);
    }

    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);

    // "Weiter" aktiv faerben wenn Rasse gewaehlt
    int race_ok = s_show_subrace
                  ? 1
                  : (s_list.selected >= 0);
    s_btn_next.color_bg = race_ok ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_back);
    ui_button_draw(&s_btn_next);

    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM,
                 "[DPad] Nav  [A] Waehlen  [B] Zurueck");
}

// ---- Update ----------------------------------------------------------------

void step_race_update(u32 keys_down, u32 keys_held,
                      touchPosition* touch, int touch_down) {
    (void)keys_held;

    if (s_show_subrace) {
        // Unterrassen-Navigation
        if (keys_down & KEY_B) {
            s_show_subrace = 0;
            return;
        }
        if (keys_down & KEY_DOWN) {
            s_subrace_list.selected++;
            ui_list_scroll(&s_subrace_list, 1);
        }
        if (keys_down & KEY_UP) {
            s_subrace_list.selected--;
            ui_list_scroll(&s_subrace_list, -1);
        }
        const SrdRace* r = &SRD_RACES[g_builder.race_idx];
        if (s_subrace_list.selected < 0) s_subrace_list.selected = 0;
        if (s_subrace_list.selected >= r->subrace_count)
            s_subrace_list.selected = r->subrace_count - 1;

        if (keys_down & KEY_A) {
            g_builder.subrace_idx = s_subrace_list.selected;
            builder_next_step();
            return;
        }

        if (touch_down) {
            ui_list_handle_touch(&s_subrace_list, touch, touch_down);
            if (ui_button_touched(&s_btn_next, touch)) {
                g_builder.subrace_idx = s_subrace_list.selected;
                builder_next_step();
                return;
            }
            if (ui_button_touched(&s_btn_back, touch)) {
                s_show_subrace = 0;
                return;
            }
        }
        return;
    }

    // Normale Rassen-Navigation
    if (keys_down & KEY_B) {
        builder_prev_step();
        return;
    }
    if (keys_down & KEY_DOWN) {
        s_list.selected++;
        ui_list_scroll(&s_list, 1);
    }
    if (keys_down & KEY_UP) {
        s_list.selected--;
        ui_list_scroll(&s_list, -1);
    }
    if (s_list.selected < 0) s_list.selected = 0;
    if (s_list.selected >= SRD_RACE_COUNT)
        s_list.selected = SRD_RACE_COUNT - 1;

    if (keys_down & KEY_A && s_list.selected >= 0) {
        g_builder.race_idx = s_list.selected;
        const SrdRace* r = &SRD_RACES[g_builder.race_idx];
        if (r->subrace_count > 0) {
            // Unterrassen-Auswahl oeffnen
            g_builder.subrace_idx = 0;
            init_subrace_list(g_builder.race_idx);
            s_show_subrace = 1;
        } else {
            g_builder.subrace_idx = -1;
            builder_next_step();
        }
        return;
    }

    if (!touch_down) return;

    int old_sel = s_list.selected;
    ui_list_handle_touch(&s_list, touch, touch_down);
    if (s_list.selected != old_sel) {
        // Auswahl geaendert – nur Vorschau aktualisieren, nicht weiternavigieren
    }

    if (ui_button_touched(&s_btn_next, touch) && s_list.selected >= 0) {
        g_builder.race_idx = s_list.selected;
        const SrdRace* r = &SRD_RACES[g_builder.race_idx];
        if (r->subrace_count > 0) {
            g_builder.subrace_idx = 0;
            init_subrace_list(g_builder.race_idx);
            s_show_subrace = 1;
        } else {
            g_builder.subrace_idx = -1;
            builder_next_step();
        }
        return;
    }

    if (ui_button_touched(&s_btn_back, touch)) {
        builder_prev_step();
        return;
    }

    // Tap auf Liste + A-Equivalent: direkt weiter wenn bereits gewaehlt
    if (ui_point_in_rect((float)touch->px, (float)touch->py,
                         s_list.x, s_list.y, s_list.w, s_list.h)
        && s_list.selected >= 0) {
        g_builder.race_idx = s_list.selected;
        const SrdRace* r = &SRD_RACES[g_builder.race_idx];
        if (r->subrace_count > 0) {
            g_builder.subrace_idx = 0;
            init_subrace_list(g_builder.race_idx);
            s_show_subrace = 1;
        } else {
            g_builder.subrace_idx = -1;
            builder_next_step();
        }
    }
}
