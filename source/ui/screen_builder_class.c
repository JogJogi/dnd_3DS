#include "screen_builder.h"
#include "ui_core.h"
#include "../data/srd_classes.h"
#include "../utils/dnd_rules.h"
#include <stdio.h>
#include <string.h>

// ---- Zustand: Klassen-Auswahl ----------------------------------------------
static UiList   s_class_list;
static UiButton s_btn_next;
static UiButton s_btn_back;

// ---- Zustand: Skill-Auswahl (BUILDER_STEP_SKILLS) --------------------------
static UiList   s_skill_list;
static UiButton s_btn_skills_next;
static UiButton s_btn_skills_back;

// ---- Skill-Listen-Text aufbauen --------------------------------------------
static void refresh_skill_list(void) {
    if (g_builder.class_idx < 0) return;
    const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];

    if (cls->any_skill) {
        // Alle 18 Skills
        ui_list_init(&s_skill_list, 4, 28, 312, 162, 22);
        for (int i = 0; i < SKILL_COUNT; i++) {
            char row[128];
            int chosen = g_builder.chosen_skills[i];
            snprintf(row, sizeof(row), "[%s] %s",
                     chosen ? "X" : " ", SKILL_NAMES[i]);
            u32 col = chosen ? COLOR_GOLD : COLOR_TEXT;
            ui_list_set_item(&s_skill_list, i, row, col);
        }
    } else {
        // Nur erlaubte Skills
        ui_list_init(&s_skill_list, 4, 28, 312, 162, 22);
        for (int i = 0; i < cls->skill_option_count; i++) {
            int si = cls->skill_options[i];
            char row[128];
            int chosen = g_builder.chosen_skills[si];
            snprintf(row, sizeof(row), "[%s] %s",
                     chosen ? "X" : " ", SKILL_NAMES[si]);
            u32 col = chosen ? COLOR_GOLD : COLOR_TEXT;
            ui_list_set_item(&s_skill_list, i, row, col);
        }
    }
}

// ---- Skill umschalten ------------------------------------------------------
static void toggle_skill(int list_idx) {
    if (g_builder.class_idx < 0) return;
    const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];

    // Welcher echte Skill-Index?
    int skill_idx;
    if (cls->any_skill)
        skill_idx = list_idx;
    else {
        if (list_idx >= cls->skill_option_count) return;
        skill_idx = cls->skill_options[list_idx];
    }

    // Aktuelle Anzahl gewaehlter Skills
    int count = 0;
    for (int i = 0; i < SKILL_COUNT; i++)
        count += g_builder.chosen_skills[i];

    if (g_builder.chosen_skills[skill_idx]) {
        // Abwaehlen immer erlaubt
        g_builder.chosen_skills[skill_idx] = 0;
    } else if (count < cls->num_skills) {
        // Auswaehlen nur wenn noch Budget
        g_builder.chosen_skills[skill_idx] = 1;
    }
    // Wenn Budget erschoepft: nichts tun (visuelles Feedback fehlt – TODO)

    refresh_skill_list();
}

// ============================================================================
// KLASSEN-SCHRITT
// ============================================================================

void step_class_init(void) {
    ui_list_init(&s_class_list, 4, 28, 312, 166, 22);
    for (int i = 0; i < SRD_CLASS_COUNT; i++) {
        char row[128];
        snprintf(row, sizeof(row), "%-14s W%d  %s",
                 SRD_CLASSES[i].name,
                 SRD_CLASSES[i].hit_die,
                 ABILITY_SHORT[SRD_CLASSES[i].primary_ability]);
        ui_list_set_item(&s_class_list, i, row, COLOR_TEXT);
    }

    if (g_builder.class_idx >= 0)
        s_class_list.selected = g_builder.class_idx;
    else
        s_class_list.selected = 0;

    s_btn_back = (UiButton){ 4,   202, 100, 26, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_next = (UiButton){ 210, 202, 106, 26, "Weiter >",  COLOR_ACCENT,     COLOR_TEXT, 0 };
}

void step_class_draw_top(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Schritt 2/6: Klasse waehlen");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    int sel = s_class_list.selected;
    if (sel < 0 || sel >= SRD_CLASS_COUNT) {
        ui_draw_text(10, 60, 0.5f, 0.42f, COLOR_TEXT_DIM, "Waehle eine Klasse.");
        return;
    }

    const SrdClass* cls = &SRD_CLASSES[sel];

    // Name gross + Trefferwuerfel
    ui_draw_textf(10, 34, 0.5f, 0.60f, COLOR_TEXT, "%s", cls->name);
    ui_draw_textf(250, 42, 0.5f, 0.42f, COLOR_GOLD, "W%d", cls->hit_die);

    // Primärattribut und Rettungswuerfe
    ui_draw_textf(10, 58, 0.5f, 0.40f, COLOR_TEXT_DIM,
                  "Primaer: %s  |  RW: %s + %s",
                  ABILITY_SHORT[cls->primary_ability],
                  ABILITY_SHORT[cls->save_prof[0]],
                  ABILITY_SHORT[cls->save_prof[1]]);

    // Fertigkeiten-Info
    if (cls->any_skill)
        ui_draw_textf(10, 74, 0.5f, 0.38f, COLOR_TEXT_DIM,
                      "Fertigkeiten: %d beliebige", cls->num_skills);
    else
        ui_draw_textf(10, 74, 0.5f, 0.38f, COLOR_TEXT_DIM,
                      "Fertigkeiten: %d aus %d", cls->num_skills, cls->skill_option_count);

    // Ruestung/Waffen
    ui_draw_textf(10, 88, 0.5f, 0.36f, COLOR_TEXT_DIM,
                  "Ruestung: %s", cls->armor_profs);

    ui_draw_separator(0, 104, SCREEN_TOP_W, COLOR_ACCENT);

    // Stufe-1-Features
    ui_draw_text(10, 108, 0.5f, 0.40f, COLOR_TEXT, "Stufe-1-Features:");
    int y = 122;
    for (int i = 0; i < cls->feature_count && y < 220; i++) {
        ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_GOLD, "* %s", cls->features[i].name);
        y += 14;
        // Beschreibung umbrechen wenn noetig
        const char* d = cls->features[i].description;
        int dlen = (int)strlen(d);
        if (dlen <= 58) {
            ui_draw_textf(18, y, 0.5f, 0.34f, COLOR_TEXT_DIM, "%s", d);
            y += 12;
        } else {
            char line[64];
            int cut = 57;
            while (cut > 0 && d[cut] != ' ') cut--;
            if (cut == 0) cut = 57;
            snprintf(line, sizeof(line), "%.*s", cut, d);
            ui_draw_textf(18, y, 0.5f, 0.34f, COLOR_TEXT_DIM, "%s", line);
            y += 11;
            ui_draw_textf(18, y, 0.5f, 0.34f, COLOR_TEXT_DIM, "%s", d + cut + 1);
            y += 12;
        }
    }

    // Zauberwirker-Info
    if (cls->is_spellcaster && y < 225) {
        ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM);
        y += 4;
        ui_draw_textf(10, y, 0.5f, 0.36f, COLOR_ACCENT2,
                      "Zauber: %s-basiert, %d ZT, %d Grad-1-Plaetze",
                      ABILITY_SHORT[cls->spellcasting_ability],
                      cls->cantrips_known,
                      cls->spell_slots_l1);
    }
}

void step_class_draw_bottom(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_TEXT_DIM, "Klasse waehlen");
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

    ui_list_draw(&s_class_list);

    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);
    s_btn_next.color_bg = (s_class_list.selected >= 0) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_back);
    ui_button_draw(&s_btn_next);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM,
                 "[DPad] Nav  [A/Tip] Auswaehlen  [B] Zurueck");
}

void step_class_update(u32 keys_down, u32 keys_held,
                       touchPosition* touch, int touch_down) {
    (void)keys_held;

    if (keys_down & KEY_B) { builder_prev_step(); return; }

    if (keys_down & KEY_DOWN) { s_class_list.selected++; ui_list_scroll(&s_class_list, 1); }
    if (keys_down & KEY_UP)   { s_class_list.selected--; ui_list_scroll(&s_class_list, -1); }
    if (s_class_list.selected < 0) s_class_list.selected = 0;
    if (s_class_list.selected >= SRD_CLASS_COUNT)
        s_class_list.selected = SRD_CLASS_COUNT - 1;

    if (keys_down & KEY_A && s_class_list.selected >= 0) {
        // Klassen-Auswahl speichern und Skills-Schritt starten
        g_builder.class_idx = s_class_list.selected;
        // Skill-Auswahl zuruecksetzen
        for (int i = 0; i < SKILL_COUNT; i++)
            g_builder.chosen_skills[i] = 0;
        builder_next_step(); // -> BUILDER_STEP_SKILLS
        return;
    }

    if (!touch_down) return;

    ui_list_handle_touch(&s_class_list, touch, touch_down);

    if (ui_button_touched(&s_btn_next, touch) && s_class_list.selected >= 0) {
        g_builder.class_idx = s_class_list.selected;
        for (int i = 0; i < SKILL_COUNT; i++)
            g_builder.chosen_skills[i] = 0;
        builder_next_step();
        return;
    }
    if (ui_button_touched(&s_btn_back, touch)) { builder_prev_step(); return; }

    // Tap auf Liste weiter
    if (ui_point_in_rect((float)touch->px, (float)touch->py,
                         s_class_list.x, s_class_list.y,
                         s_class_list.w, s_class_list.h)
        && s_class_list.selected >= 0) {
        g_builder.class_idx = s_class_list.selected;
        for (int i = 0; i < SKILL_COUNT; i++)
            g_builder.chosen_skills[i] = 0;
        builder_next_step();
    }
}

// ============================================================================
// SKILL-SCHRITT (BUILDER_STEP_SKILLS)
// ============================================================================

void step_skills_init(void) {
    refresh_skill_list();
    s_btn_skills_back = (UiButton){ 4,   202, 100, 26, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_skills_next = (UiButton){ 210, 202, 106, 26, "Weiter >",  COLOR_ACCENT,     COLOR_TEXT, 0 };
}

void step_skills_draw_top(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Schritt 2/6: Fertigkeiten");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    if (g_builder.class_idx < 0) return;
    const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];

    // Zaehler
    int chosen = 0;
    for (int i = 0; i < SKILL_COUNT; i++) chosen += g_builder.chosen_skills[i];

    ui_draw_textf(10, 36, 0.5f, 0.50f, COLOR_TEXT,
                  "Klasse: %s", cls->name);
    ui_draw_textf(10, 54, 0.5f, 0.42f, COLOR_GOLD,
                  "Gewaehlt: %d / %d", chosen, cls->num_skills);

    if (cls->any_skill)
        ui_draw_textf(10, 70, 0.5f, 0.38f, COLOR_TEXT_DIM,
                      "Du darfst beliebige %d Fertigkeiten waehlen.", cls->num_skills);
    else
        ui_draw_textf(10, 70, 0.5f, 0.38f, COLOR_TEXT_DIM,
                      "Waehle %d aus den angebotenen Fertigkeiten.", cls->num_skills);

    ui_draw_separator(0, 86, SCREEN_TOP_W, COLOR_TEXT_DIM);

    // Gewaehlte Fertigkeiten als Vorschau auflisten
    ui_draw_text(10, 92, 0.5f, 0.38f, COLOR_TEXT, "Deine Fertigkeiten:");
    int y = 106;
    for (int i = 0; i < SKILL_COUNT && y < 235; i++) {
        if (g_builder.chosen_skills[i]) {
            int ab = SKILL_ABILITY[i];
            int ability_final = builder_final_ability(ab);
            int mod = dnd_modifier(ability_final) + 2; // +2 Uebungsbonus Stufe 1
            ui_draw_textf(10, y, 0.5f, 0.38f, COLOR_GOLD,
                          "* %-18s %+d", SKILL_NAMES[i], mod);
            y += 14;
        }
    }

    if (chosen == 0)
        ui_draw_textf(10, 110, 0.5f, 0.40f, COLOR_TEXT_DIM,
                      "Noch keine Fertigkeit gewaehlt.");
}

void step_skills_draw_bottom(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_TEXT_DIM, "Fertigkeiten waehlen");
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

    ui_list_draw(&s_skill_list);

    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);

    // "Weiter" aktiv wenn genug Skills
    if (g_builder.class_idx >= 0) {
        const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];
        int chosen = 0;
        for (int i = 0; i < SKILL_COUNT; i++) chosen += g_builder.chosen_skills[i];
        s_btn_skills_next.color_bg = (chosen >= cls->num_skills)
                                     ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    }

    ui_button_draw(&s_btn_skills_back);
    ui_button_draw(&s_btn_skills_next);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM,
                 "[A/Tip] Toggle  [DPad] Nav  [B] Zurueck");
}

void step_skills_update(u32 keys_down, u32 keys_held,
                        touchPosition* touch, int touch_down) {
    (void)keys_held;

    if (keys_down & KEY_B) { builder_prev_step(); return; }

    if (keys_down & KEY_DOWN) { s_skill_list.selected++; ui_list_scroll(&s_skill_list, 1); }
    if (keys_down & KEY_UP)   { s_skill_list.selected--; ui_list_scroll(&s_skill_list, -1); }

    int max_items = (g_builder.class_idx >= 0 && !SRD_CLASSES[g_builder.class_idx].any_skill)
                    ? SRD_CLASSES[g_builder.class_idx].skill_option_count
                    : SKILL_COUNT;
    if (s_skill_list.selected < 0) s_skill_list.selected = 0;
    if (s_skill_list.selected >= max_items) s_skill_list.selected = max_items - 1;

    if (keys_down & KEY_A && s_skill_list.selected >= 0) {
        toggle_skill(s_skill_list.selected);
        return;
    }

    // "Weiter" per R-Taste wenn fertig
    if (keys_down & KEY_R) {
        if (g_builder.class_idx >= 0) {
            int chosen = 0;
            for (int i = 0; i < SKILL_COUNT; i++) chosen += g_builder.chosen_skills[i];
            if (chosen >= SRD_CLASSES[g_builder.class_idx].num_skills) {
                builder_next_step();
                return;
            }
        }
    }

    if (!touch_down) return;

    // Touch auf Liste
    int old_sel = s_skill_list.selected;
    ui_list_handle_touch(&s_skill_list, touch, touch_down);
    if (s_skill_list.selected != old_sel) {
        toggle_skill(s_skill_list.selected);
        return;
    }

    // Toggle wenn Tap auf selben Eintrag
    if (ui_point_in_rect((float)touch->px, (float)touch->py,
                         s_skill_list.x, s_skill_list.y,
                         s_skill_list.w, s_skill_list.h)
        && s_skill_list.selected >= 0) {
        toggle_skill(s_skill_list.selected);
        return;
    }

    if (ui_button_touched(&s_btn_skills_next, touch)) {
        if (g_builder.class_idx >= 0) {
            int chosen = 0;
            for (int i = 0; i < SKILL_COUNT; i++) chosen += g_builder.chosen_skills[i];
            if (chosen >= SRD_CLASSES[g_builder.class_idx].num_skills)
                builder_next_step();
        }
        return;
    }
    if (ui_button_touched(&s_btn_skills_back, touch)) { builder_prev_step(); return; }
}
