#include "screen_builder.h"
#include "ui_core.h"
#include "../data/srd_races.h"
#include "../data/srd_classes.h"
#include "../data/srd_backgrounds.h"
#include "../utils/dnd_rules.h"
#include <stdio.h>
#include <string.h>

// ---- Zustand ---------------------------------------------------------------
static UiButton s_btn_confirm;
static UiButton s_btn_back;
static UiButton s_btn_equip_a;
static UiButton s_btn_equip_b;

static int s_top_scroll = 0;  // Scrollen auf dem Top-Screen

// ---- Initialisierung -------------------------------------------------------

void step_review_init(void) {
    s_top_scroll = 0;

    s_btn_equip_a = (UiButton){ 4,   100, 150, 28, "Option A", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_equip_b = (UiButton){ 162, 100, 154, 28, "Option B", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };

    s_btn_back    = (UiButton){ 4,   202, 100, 26, "< Aendern", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_confirm = (UiButton){ 154, 202, 162, 26, "Charakter erstellen!", COLOR_ACCENT, COLOR_TEXT, 0 };
}

// ---- Top-Screen: vollstaendige Zusammenfassung -----------------------------

void step_review_draw_top(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Schritt 6/6: Zusammenfassung");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    int y = 32;
    float sz_n = 0.40f;   // Normal
    float sz_s = 0.36f;   // Klein

    // ---- Name / Rasse / Klasse / Hintergrund / Gesinnung ------------------
    const char* rname = (g_builder.race_idx >= 0)
                        ? SRD_RACES[g_builder.race_idx].name : "?";
    const char* cname = (g_builder.class_idx >= 0)
                        ? SRD_CLASSES[g_builder.class_idx].name : "?";
    const char* bgname = (g_builder.bg_idx >= 0)
                         ? SRD_BACKGROUNDS[g_builder.bg_idx].name : "?";

    ui_draw_textf(10, y, 0.5f, 0.60f, COLOR_TEXT, "%s",
                  g_builder.name[0] ? g_builder.name : "Held");
    y += 20;

    ui_draw_textf(10, y, 0.5f, sz_n, COLOR_TEXT_DIM,
                  "%s  |  %s  |  Stufe 1", rname, cname);
    y += 14;
    ui_draw_textf(10, y, 0.5f, sz_s, COLOR_TEXT_DIM,
                  "Hintergrund: %s  |  %s", bgname, g_builder.alignment);
    y += 12;

    ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM);
    y += 4;

    // ---- Kernwerte ----------------------------------------------------------
    int hp  = builder_initial_hp();
    int ac  = builder_initial_ac();
    int spd = (g_builder.race_idx >= 0) ? SRD_RACES[g_builder.race_idx].speed : 30;
    int pb  = 2;  // Stufe 1 immer +2

    ui_draw_textf(10,  y, 0.5f, sz_n, COLOR_HP_GREEN, "HP %d", hp);
    ui_draw_textf(80,  y, 0.5f, sz_n, COLOR_TEXT,     "AC %d", ac);
    ui_draw_textf(130, y, 0.5f, sz_n, COLOR_TEXT,     "Spd %d", spd);
    ui_draw_textf(190, y, 0.5f, sz_n, COLOR_GOLD,     "Uebung +%d", pb);
    if (g_builder.race_idx >= 0 && SRD_RACES[g_builder.race_idx].extra_speed)
        ui_draw_textf(290, y, 0.5f, sz_s, COLOR_ACCENT2,
                      "%s", SRD_RACES[g_builder.race_idx].extra_speed);
    y += 14;

    // Zauberwirker-Info
    if (g_builder.class_idx >= 0 && SRD_CLASSES[g_builder.class_idx].is_spellcaster) {
        const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];
        ui_draw_textf(10, y, 0.5f, sz_s, COLOR_ACCENT2,
                      "Zauber: %d ZT  |  %d Grad-1-Plaetze  |  %s-basiert",
                      cls->cantrips_known, cls->spell_slots_l1,
                      ABILITY_SHORT[cls->spellcasting_ability]);
        y += 12;
    }

    ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM);
    y += 4;

    // ---- Attribute ----------------------------------------------------------
    ui_draw_text(10, y, 0.5f, sz_n, COLOR_TEXT, "Attribute:");
    y += 14;
    for (int i = 0; i < ABILITY_COUNT; i++) {
        int final = builder_final_ability(i);
        int mod   = dnd_modifier(final);
        float x   = 10 + i * 64;
        ui_draw_textf(x, y,      0.5f, sz_s, COLOR_TEXT_DIM, "%s",  ABILITY_SHORT[i]);
        ui_draw_textf(x, y + 11, 0.5f, sz_n, COLOR_TEXT,     "%2d", final);
        ui_draw_textf(x, y + 23, 0.5f, sz_s,
                      mod >= 0 ? COLOR_HP_GREEN : COLOR_HP_RED, "%+d", mod);
    }
    y += 38;

    ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM);
    y += 4;

    // ---- Fertigkeiten (proficient) ------------------------------------------
    ui_draw_text(10, y, 0.5f, sz_s, COLOR_TEXT, "Fertigkeits-Uebungen:");
    y += 12;
    char skill_line[256] = "";
    for (int i = 0; i < SKILL_COUNT; i++) {
        int bg_prof = 0;
        if (g_builder.bg_idx >= 0) {
            const SrdBackground* bg = &SRD_BACKGROUNDS[g_builder.bg_idx];
            for (int j = 0; j < bg->skill_prof_count; j++)
                if (bg->skill_profs[j] == i) { bg_prof = 1; break; }
        }
        if (g_builder.chosen_skills[i] || bg_prof) {
            if (strlen(skill_line) + strlen(SKILL_NAMES[i]) + 4 < sizeof(skill_line)) {
                if (skill_line[0]) strncat(skill_line, ", ", sizeof(skill_line) - strlen(skill_line) - 1);
                strncat(skill_line, SKILL_NAMES[i], sizeof(skill_line) - strlen(skill_line) - 1);
            }
        }
    }
    if (skill_line[0]) {
        // In zwei Zeilen aufteilen wenn noetig
        int slen = (int)strlen(skill_line);
        if (slen <= 70) {
            ui_draw_textf(10, y, 0.5f, sz_s, COLOR_GOLD, "%s", skill_line);
            y += 12;
        } else {
            int cut = 69;
            while (cut > 0 && skill_line[cut] != ',') cut--;
            if (cut == 0) cut = 69;
            char l1[128];
            snprintf(l1, sizeof(l1), "%.*s", cut + 1, skill_line);
            ui_draw_textf(10, y, 0.5f, sz_s, COLOR_GOLD, "%s", l1);
            y += 11;
            ui_draw_textf(10, y, 0.5f, sz_s, COLOR_GOLD, "%s", skill_line + cut + 2);
            y += 12;
        }
    } else {
        ui_draw_textf(10, y, 0.5f, sz_s, COLOR_TEXT_DIM, "(keine)");
        y += 12;
    }

    // ---- Rettungswuerfe -----------------------------------------------------
    if (g_builder.class_idx >= 0) {
        const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];
        ui_draw_textf(10, y, 0.5f, sz_s, COLOR_TEXT_DIM,
                      "Rettungswuerfe: %s, %s",
                      ABILITY_SHORT[cls->save_prof[0]],
                      ABILITY_SHORT[cls->save_prof[1]]);
        y += 12;
    }

    // Scroll-Hinweis
    if (y > 235) {
        ui_draw_text(350, 230, 0.5f, sz_s, COLOR_TEXT_DIM, "[v]");
    }
}

// ---- Bottom-Screen ---------------------------------------------------------

void step_review_draw_bottom(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_GOLD, "Bestaetigung");
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

    // Ausruestungs-Option
    ui_draw_text(8, 32, 0.5f, 0.40f, COLOR_TEXT, "Startausruestung:");

    if (g_builder.class_idx >= 0) {
        const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];

        ui_draw_textf(8, 50, 0.5f, 0.35f, COLOR_TEXT_DIM,
                      "A: %.46s", cls->equip_a.desc);
        ui_draw_textf(8, 62, 0.5f, 0.35f, COLOR_TEXT_DIM,
                      "B: %.46s", cls->equip_b.desc);

        s_btn_equip_a.color_bg = (g_builder.equip_choice == 0)
                                  ? COLOR_ACCENT : COLOR_BTN_NORMAL;
        s_btn_equip_b.color_bg = (g_builder.equip_choice == 1)
                                  ? COLOR_ACCENT : COLOR_BTN_NORMAL;
        ui_button_draw(&s_btn_equip_a);
        ui_button_draw(&s_btn_equip_b);
    } else {
        ui_draw_text(8, 50, 0.5f, 0.38f, COLOR_TEXT_DIM, "(keine Klasse gewaehlt)");
    }

    ui_draw_separator(0, 136, SCREEN_BTM_W, COLOR_TEXT_DIM);

    // Alles vollstaendig?
    int all_ok = (g_builder.race_idx  >= 0)
              && (g_builder.class_idx >= 0)
              && (g_builder.bg_idx    >= 0)
              && (g_builder.name[0] != '\0');

    if (!all_ok) {
        ui_draw_text(8, 144, 0.5f, 0.38f, COLOR_HP_RED,
                     "Bitte alle Schritte abschliessen!");
        if (g_builder.race_idx  < 0) ui_draw_text(8, 158, 0.5f, 0.35f, COLOR_HP_RED, "! Rasse fehlt");
        if (g_builder.class_idx < 0) ui_draw_text(8, 170, 0.5f, 0.35f, COLOR_HP_RED, "! Klasse fehlt");
        if (g_builder.bg_idx    < 0) ui_draw_text(8, 182, 0.5f, 0.35f, COLOR_HP_RED, "! Hintergrund fehlt");
        if (!g_builder.name[0])      ui_draw_text(8, 194, 0.5f, 0.35f, COLOR_HP_RED, "! Name fehlt");
    }

    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);

    s_btn_confirm.color_bg = all_ok ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_back);
    ui_button_draw(&s_btn_confirm);

    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM,
                 "[A] Bestaetigen  [B] Aendern");
}

// ---- Update ----------------------------------------------------------------

void step_review_update(u32 keys_down, u32 keys_held,
                        touchPosition* touch, int touch_down) {
    (void)keys_held;

    if (keys_down & KEY_B) { builder_prev_step(); return; }

    // A = Charakter erstellen
    if (keys_down & KEY_A) {
        int all_ok = (g_builder.race_idx  >= 0)
                  && (g_builder.class_idx >= 0)
                  && (g_builder.bg_idx    >= 0)
                  && (g_builder.name[0] != '\0');
        if (all_ok) builder_finish();
        return;
    }

    // L/R = Ausruestungsoption wechseln
    if (keys_down & KEY_L) g_builder.equip_choice = 0;
    if (keys_down & KEY_R) g_builder.equip_choice = 1;

    if (!touch_down) return;

    if (ui_button_touched(&s_btn_equip_a, touch)) { g_builder.equip_choice = 0; return; }
    if (ui_button_touched(&s_btn_equip_b, touch)) { g_builder.equip_choice = 1; return; }

    if (ui_button_touched(&s_btn_confirm, touch)) {
        int all_ok = (g_builder.race_idx  >= 0)
                  && (g_builder.class_idx >= 0)
                  && (g_builder.bg_idx    >= 0)
                  && (g_builder.name[0] != '\0');
        if (all_ok) builder_finish();
        return;
    }
    if (ui_button_touched(&s_btn_back, touch)) { builder_prev_step(); return; }
}
