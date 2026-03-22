#include "screen_manager.h"
#include "ui_core.h"
#include "../models/character.h"
#include "../models/feature.h"
#include "../utils/dnd_rules.h"
#include "../db/character_db.h"
#include "../db/feature_db.h"
#include "../db/spell_db.h"
#include "../data/srd_classes.h"
#include "../data/srd_progression.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// Stufenaufstiegs-Wizard (SCREEN_LEVELUP)
// Aufgerufen von screen_character.c mit g_active_char_id gesetzt.
// ============================================================================

// ---- Zustand ---------------------------------------------------------------
static Character        s_char;
static int              s_loaded       = 0;
static int              s_new_level    = 2;
static int              s_hp_choice    = 0;    // 0=Durchschnitt, 1=Maximum
static int              s_hp_gain      = 0;    // berechneter HP-Zuwachs
static int              s_asi_avail    = 0;    // 1 wenn ASI verfuegbar
static int              s_asi_mode     = 0;    // 0=+2einzel, 1=+1+1zwei
static int              s_asi_sel1     = -1;   // erster gewaehlter Attribute
static int              s_asi_sel2     = -1;   // zweiter (nur bei Modus 1)

static const ClassProg* s_prog         = NULL;
static const ProgFeat*  s_new_feats    = NULL;
static int              s_new_feat_cnt = 0;
static int              s_feat_scroll  = 0;    // Scroll auf Top-Screen

static char             s_class_id[CHAR_NAME_MAX];

// Neue Zauberschlitze
static int              s_new_slots[PROG_SLOT_LEVELS];
static SpellSlots       s_spell_slots;
static int              s_has_slots    = 0;

// ---- Buttons ---------------------------------------------------------------
static UiButton s_btn_hp_avg, s_btn_hp_max;
static UiButton s_btn_asi_mode;
static UiButton s_btn_asi_ab[ABILITY_COUNT];
static UiButton s_btn_confirm, s_btn_cancel;
static char     s_lbl_asi[ABILITY_COUNT][16];

// ---- Hilfsfunktionen -------------------------------------------------------
static void recalc_hp(void) {
    int hit_die = 8;
    // Klasse aus char.class_name -> SrdClass suchen
    for (int i = 0; i < SRD_CLASS_COUNT; i++) {
        if (strcmp(SRD_CLASSES[i].name, s_char.class_name) == 0) {
            hit_die = SRD_CLASSES[i].hit_die;
            break;
        }
    }
    int con_mod = dnd_modifier(s_char.ability[ABILITY_CON]);
    if (s_hp_choice == 0)
        s_hp_gain = srd_prog_hp_average(hit_die, con_mod);
    else
        s_hp_gain = srd_prog_hp_max(hit_die, con_mod);
}

static void refresh_asi_labels(void) {
    for (int i = 0; i < ABILITY_COUNT; i++) {
        int val = s_char.ability[i];
        // ASI-Preview aufaddieren
        int bonus = 0;
        if (i == s_asi_sel1) bonus += (s_asi_mode == 0) ? 2 : 1;
        if (i == s_asi_sel2) bonus += 1;
        snprintf(s_lbl_asi[i], sizeof(s_lbl_asi[i]), "%s %d",
                 ABILITY_SHORT[i], val + bonus);
        s_btn_asi_ab[i].label = s_lbl_asi[i];
        // Farbe: ausgewaehlt = Goldgelb
        int sel = (i == s_asi_sel1 || i == s_asi_sel2);
        s_btn_asi_ab[i].color_bg = sel ? COLOR_GOLD : COLOR_BTN_NORMAL;
    }
}

static int asi_valid(void) {
    if (!s_asi_avail) return 1;  // kein ASI noetig -> immer gueltig
    if (s_asi_mode == 0) return s_asi_sel1 >= 0;
    return s_asi_sel1 >= 0 && s_asi_sel2 >= 0 && s_asi_sel1 != s_asi_sel2;
}

// Klassen-ID aus class_name ableiten
static void find_class_id(void) {
    s_class_id[0] = '\0';
    for (int i = 0; i < SRD_CLASS_COUNT; i++) {
        if (strcmp(SRD_CLASSES[i].name, s_char.class_name) == 0) {
            snprintf(s_class_id, sizeof(s_class_id), "%s", SRD_CLASSES[i].id);
            return;
        }
    }
}

// ---- on_enter --------------------------------------------------------------
static void on_enter(void) {
    s_loaded = (character_db_load(g_active_char_id, &s_char) == 0);
    if (!s_loaded) return;

    s_new_level = s_char.level + 1;
    if (s_new_level > 20) s_new_level = 20;

    find_class_id();

    // Progression laden
    s_prog = srd_prog_get(s_class_id);
    s_new_feat_cnt = srd_prog_feats(s_class_id, s_new_level, &s_new_feats);
    srd_prog_spell_slots(s_class_id, s_new_level, s_new_slots);
    s_asi_avail = srd_prog_is_asi(s_class_id, s_new_level);

    // HP-Voreinstellung
    s_hp_choice = 0;
    s_asi_mode  = 0;
    s_asi_sel1  = -1;
    s_asi_sel2  = -1;
    s_feat_scroll = 0;
    recalc_hp();

    // Aktuelle Zauberplaetze laden
    s_has_slots = (spell_slots_db_load(g_active_char_id, &s_spell_slots) == 0);

    // Buttons
    s_btn_hp_avg = (UiButton){   4,  56, 152, 26, "Durchschnitt",  COLOR_ACCENT,     COLOR_TEXT, 0 };
    s_btn_hp_max = (UiButton){ 164,  56, 152, 26, "Maximum",       COLOR_BTN_NORMAL, COLOR_TEXT, 0 };

    s_btn_asi_mode = (UiButton){ 4, 96, 312, 22, "+2 auf ein Attribut", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };

    // ASI-Attribut-Buttons (2 Reihen x 3)
    for (int i = 0; i < ABILITY_COUNT; i++) {
        int col = i % 3, row = i / 3;
        s_btn_asi_ab[i] = (UiButton){
            4.0f + col * 106.0f, 122.0f + row * 30.0f, 100.0f, 26.0f,
            s_lbl_asi[i], COLOR_BTN_NORMAL, COLOR_TEXT, 0
        };
    }
    refresh_asi_labels();

    s_btn_cancel  = (UiButton){   4, 202, 100, 26, "Abbrechen",     COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_confirm = (UiButton){ 116, 202, 200, 26, "Stufe erhoehen!", COLOR_ACCENT,   COLOR_TEXT, 0 };
}

static void on_exit(void) { /* nothing */ }

// ---- Top-Screen: Uebersicht der Gewinne ------------------------------------
void draw_top(void) {
    // Header
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD,
                  "Stufenaufstieg: %s  Stufe %d → %d",
                  s_char.name, s_char.level, s_new_level);
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    int y = 32;
    float sz = 0.40f, szs = 0.36f;

    // HP-Zuwachs
    ui_draw_textf(10, y, 0.5f, sz, COLOR_HP_GREEN,
                  "HP: %d  →  %d  (+%d %s)",
                  s_char.hp_max,
                  s_char.hp_max + s_hp_gain,
                  s_hp_gain,
                  s_hp_choice == 0 ? "(Durschn.)" : "(Max)");
    y += 16;

    // Uebungsbonus
    int old_pb = dnd_proficiency_bonus(s_char.level);
    int new_pb = dnd_proficiency_bonus(s_new_level);
    if (new_pb != old_pb) {
        ui_draw_textf(10, y, 0.5f, sz, COLOR_GOLD,
                      "Uebungsbonus: +%d  →  +%d", old_pb, new_pb);
        y += 16;
    }

    // ASI-Vorschau
    if (s_asi_avail) {
        if (s_asi_mode == 0 && s_asi_sel1 >= 0) {
            ui_draw_textf(10, y, 0.5f, sz, COLOR_ACCENT2,
                          "ASI: %s %d  →  %d",
                          ABILITY_SHORT[s_asi_sel1],
                          s_char.ability[s_asi_sel1],
                          s_char.ability[s_asi_sel1] + 2);
        } else if (s_asi_mode == 1 && s_asi_sel1 >= 0 && s_asi_sel2 >= 0) {
            ui_draw_textf(10, y, 0.5f, sz, COLOR_ACCENT2,
                          "ASI: %s+1, %s+1",
                          ABILITY_SHORT[s_asi_sel1],
                          ABILITY_SHORT[s_asi_sel2]);
        } else {
            ui_draw_textf(10, y, 0.5f, szs, COLOR_TEXT_DIM, "ASI: (noch nicht gewaehlt)");
        }
        y += 16;
    }

    // Zauberplaetze
    int has_slot_change = 0;
    for (int i = 0; i < PROG_SLOT_LEVELS; i++) {
        if (s_new_slots[i] != 0) { has_slot_change = 1; break; }
    }
    if (has_slot_change) {
        ui_draw_textf(10, y, 0.5f, szs, COLOR_ACCENT2, "Neue Zauberplaetze:");
        y += 12;
        for (int i = 0; i < 5; i++) {
            if (s_new_slots[i] == 0) continue;
            int old_s = s_has_slots ? s_spell_slots.total[i] : 0;
            if (s_new_slots[i] != old_s) {
                ui_draw_textf(10 + (i % 3) * 130, y + (i / 3) * 12,
                              0.5f, szs, COLOR_TEXT,
                              "Grad %d: %d→%d",
                              i + 1, old_s, s_new_slots[i]);
            }
        }
        y += 24;
    }

    ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM);
    y += 4;

    // Neue Features
    if (s_new_feat_cnt > 0) {
        ui_draw_text(10, y, 0.5f, szs, COLOR_TEXT, "Neue Faehigkeiten:");
        y += 12;
        for (int i = s_feat_scroll;
             i < s_new_feat_cnt && y < 228;
             i++) {
            const ProgFeat* f = &s_new_feats[i];
            ui_draw_textf(10, y, 0.5f, sz, COLOR_GOLD, "%.40s", f->name);
            y += 13;
            // Beschreibung umbrechen (max 2 Zeilen a 72 Zeichen)
            const char* d = f->description;
            if ((int)strlen(d) <= 72) {
                ui_draw_textf(14, y, 0.5f, szs, COLOR_TEXT_DIM, "%.72s", d);
                y += 11;
            } else {
                char l1[80];
                int cut = 71;
                while (cut > 0 && d[cut] != ' ') cut--;
                if (cut == 0) cut = 71;
                snprintf(l1, sizeof(l1), "%.*s", cut, d);
                ui_draw_textf(14, y, 0.5f, szs, COLOR_TEXT_DIM, "%s", l1);
                y += 10;
                ui_draw_textf(14, y, 0.5f, szs, COLOR_TEXT_DIM, "%.72s", d + cut + 1);
                y += 11;
            }
            if (f->uses_max > 0) {
                ui_draw_textf(14, y, 0.5f, szs, COLOR_ACCENT2,
                              "%dx / %s", f->uses_max,
                              f->recharge_on[0] ? f->recharge_on : "unbegrenzt");
                y += 10;
            }
            ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM);
            y += 3;
        }
        if (s_new_feat_cnt > 2)
            ui_draw_text(350, 228, 0.5f, szs, COLOR_TEXT_DIM, "[v]");
    } else if (!has_slot_change && !s_asi_avail) {
        ui_draw_text(10, y, 0.5f, szs, COLOR_TEXT_DIM, "(Nur HP und Uebungsbonus)");
    }
}

// ---- Bottom-Screen ---------------------------------------------------------
void draw_bottom(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_GOLD,
                  "Stufe %d  →  %d  (%s)",
                  s_char.level, s_new_level, s_char.class_name);
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

    // HP-Wahl
    ui_draw_text(8, 32, 0.5f, 0.40f, COLOR_TEXT, "HP-Zuwachs:");
    ui_draw_textf(130, 32, 0.5f, 0.40f, COLOR_HP_GREEN, "+%d", s_hp_gain);

    s_btn_hp_avg.color_bg = (s_hp_choice == 0) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    s_btn_hp_max.color_bg = (s_hp_choice == 1) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_hp_avg);
    ui_button_draw(&s_btn_hp_max);

    // ASI-Bereich
    if (s_asi_avail) {
        ui_draw_separator(0, 88, SCREEN_BTM_W, COLOR_TEXT_DIM);
        ui_draw_text(8, 92, 0.5f, 0.40f, COLOR_TEXT, "Attribut-Steigerung (ASI):");
        s_btn_asi_mode.label = (s_asi_mode == 0)
                               ? "+2 auf ein Attribut"
                               : "+1+1 auf zwei Attribute";
        s_btn_asi_mode.color_bg = COLOR_BTN_NORMAL;
        ui_button_draw(&s_btn_asi_mode);
        for (int i = 0; i < ABILITY_COUNT; i++)
            ui_button_draw(&s_btn_asi_ab[i]);
    }

    // Validierungshinweis
    if (s_asi_avail && !asi_valid()) {
        ui_draw_text(8, 188, 0.5f, 0.35f, COLOR_HP_RED,
                     "Bitte Attribut fuer ASI waehlen!");
    }

    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);

    s_btn_confirm.color_bg = asi_valid() ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_cancel);
    ui_button_draw(&s_btn_confirm);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM,
                 "[A] Bestaetigen  [B] Abbrechen  [L/R] HP-Wahl");
}

// ---- Stufenaufstieg anwenden und speichern ---------------------------------
static void apply_levelup(void) {
    // Stufe erhoehen
    s_char.level = s_new_level;

    // HP aktualisieren
    s_char.hp_max     += s_hp_gain;
    s_char.hp_current += s_hp_gain;

    // ASI anwenden
    if (s_asi_avail) {
        if (s_asi_mode == 0 && s_asi_sel1 >= 0) {
            s_char.ability[s_asi_sel1] += 2;
            if (s_char.ability[s_asi_sel1] > 20) s_char.ability[s_asi_sel1] = 20;
        } else if (s_asi_mode == 1) {
            if (s_asi_sel1 >= 0) {
                s_char.ability[s_asi_sel1]++;
                if (s_char.ability[s_asi_sel1] > 20) s_char.ability[s_asi_sel1] = 20;
            }
            if (s_asi_sel2 >= 0) {
                s_char.ability[s_asi_sel2]++;
                if (s_char.ability[s_asi_sel2] > 20) s_char.ability[s_asi_sel2] = 20;
            }
        }
    }

    // Charakter speichern
    character_db_save(&s_char);

    // Neue Features speichern
    char src[FEATURE_SRC_MAX];
    snprintf(src, sizeof(src), "%.10s Stufe %d", s_char.class_name, s_new_level);
    for (int i = 0; i < s_new_feat_cnt; i++) {
        Feature f;
        memset(&f, 0, sizeof(f));
        f.character_id = s_char.id;
        snprintf(f.name,        sizeof(f.name),        "%.63s", s_new_feats[i].name);
        snprintf(f.source,      sizeof(f.source),      "%.31s", src);
        snprintf(f.description, sizeof(f.description), "%.255s", s_new_feats[i].description);
        f.uses_max = f.uses_current = s_new_feats[i].uses_max;
        snprintf(f.recharge_on, sizeof(f.recharge_on), "%.15s", s_new_feats[i].recharge_on);
        features_db_save(&f);
    }

    // Zauberplaetze aktualisieren
    int has_new_slots = 0;
    for (int i = 0; i < PROG_SLOT_LEVELS; i++)
        if (s_new_slots[i] != 0) { has_new_slots = 1; break; }

    if (has_new_slots) {
        SpellSlots updated;
        if (s_has_slots) {
            memcpy(&updated, &s_spell_slots, sizeof(SpellSlots));
        } else {
            memset(&updated, 0, sizeof(updated));
            updated.character_id = s_char.id;
        }
        for (int i = 0; i < PROG_SLOT_LEVELS; i++) {
            if (s_new_slots[i] != 0) {
                // Delta: neue Slots hinzufuegen (nicht bereits verbrauchte reduzieren)
                int delta = s_new_slots[i] - updated.total[i];
                updated.total[i] = s_new_slots[i];
                if (delta > 0) updated.used[i] = 0;  // neue Slots voll verfuegbar
            }
        }
        spell_slots_db_save(&updated);
    }

    // Zurueck zum Charakter-Screen
    screen_pop();
}

// ---- Update ----------------------------------------------------------------
static void update(u32 keys_down, u32 keys_held,
                   touchPosition* touch, int touch_down) {
    (void)keys_held;
    if (!s_loaded) { screen_pop(); return; }

    if (keys_down & KEY_B) { screen_pop(); return; }

    // L/R wechseln HP-Modus
    if (keys_down & KEY_L) {
        s_hp_choice = 0;
        recalc_hp();
    }
    if (keys_down & KEY_R) {
        s_hp_choice = 1;
        recalc_hp();
    }

    // A = bestaetigen
    if (keys_down & KEY_A && asi_valid()) {
        apply_levelup();
        return;
    }

    // Scrollen fuer Features (DPad Up/Down nur wenn kein ASI-Bereich aktiv)
    if (!s_asi_avail) {
        if (keys_down & KEY_DOWN && s_feat_scroll < s_new_feat_cnt - 1)
            s_feat_scroll++;
        if (keys_down & KEY_UP && s_feat_scroll > 0)
            s_feat_scroll--;
    }

    if (!touch_down) return;

    // HP-Wahl
    if (ui_button_touched(&s_btn_hp_avg, touch)) {
        s_hp_choice = 0; recalc_hp(); return;
    }
    if (ui_button_touched(&s_btn_hp_max, touch)) {
        s_hp_choice = 1; recalc_hp(); return;
    }

    // ASI
    if (s_asi_avail) {
        if (ui_button_touched(&s_btn_asi_mode, touch)) {
            s_asi_mode = 1 - s_asi_mode;
            s_asi_sel1 = s_asi_sel2 = -1;
            refresh_asi_labels();
            return;
        }
        for (int i = 0; i < ABILITY_COUNT; i++) {
            if (ui_button_touched(&s_btn_asi_ab[i], touch)) {
                if (s_asi_mode == 0) {
                    s_asi_sel1 = i;
                    s_asi_sel2 = -1;
                } else {
                    if (s_asi_sel1 < 0) {
                        s_asi_sel1 = i;
                    } else if (s_asi_sel2 < 0 && i != s_asi_sel1) {
                        s_asi_sel2 = i;
                    } else {
                        // Auswahl zuruecksetzen und neu beginnen
                        s_asi_sel1 = i;
                        s_asi_sel2 = -1;
                    }
                }
                refresh_asi_labels();
                return;
            }
        }
    }

    // Confirm / Cancel
    if (ui_button_touched(&s_btn_confirm, touch) && asi_valid()) {
        apply_levelup(); return;
    }
    if (ui_button_touched(&s_btn_cancel, touch)) {
        screen_pop(); return;
    }
}

// ---- Screen-Objekt ---------------------------------------------------------
Screen g_screen_levelup = {
    .id          = SCREEN_LEVELUP,
    .on_enter    = on_enter,
    .on_exit     = on_exit,
    .update      = update,
    .draw_top    = draw_top,
    .draw_bottom = draw_bottom,
};
