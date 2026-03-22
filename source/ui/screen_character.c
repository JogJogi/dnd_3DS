#include "screen_manager.h"
#include "ui_core.h"
#include "../db/character_db.h"
#include "../db/feature_db.h"
#include "../utils/dnd_rules.h"
#include "../models/character.h"
#include "../models/feature.h"
#include <stdio.h>
#include <string.h>

// ---- Skill-Namen (DE) -------------------------------------------------------
static const char* s_skill_names[SKILL_COUNT] = {
    "Akrobatik","Tierkunde","Arkanes W.","Athletik","Taeuschen",
    "Geschichte","Einsicht","Einschuecht.","Nachforschen","Medizin",
    "Naturkunde","Wahrnehmung","Vorfuehren","Ueberzeugen","Religion",
    "Taschenspiel.","Heimlichkeit","Ueberleben"
};

// ---- Zustand ---------------------------------------------------------------
static Character s_char;
static int       s_loaded = 0;

typedef enum { TAB_STATS = 0, TAB_SKILLS, TAB_INFO, TAB_ACTIONS, TAB_COUNT } CharTab;
static CharTab s_tab          = TAB_STATS;
static int     s_skill_cursor = 0;

// Aktionen-Tab: Angriffe + Fähigkeiten
static Attack   s_attacks[ATTACKS_MAX];
static int      s_attack_count = 0;
static Feature  s_features[FEATURES_MAX];
static int      s_feature_count = 0;

// Angriffs-Filter: -1=Alle, 0=Aktion, 1=Bonus, 2=Reaktion, 3=Sonst.
static int  s_act_filter = -1;
static int  s_filt_attacks[ATTACKS_MAX];
static int  s_filt_attack_count = 0;

static const char* ACT_TYPE_SHORT[4] = {"A","B","R","S"};
static const char* ACT_TYPE_NAME[4]  = {"Aktion","Bonusaktion","Reaktion","Sonstiges"};

typedef enum { ACT_ATTACKS = 0, ACT_FEATURES } ActSubtab;
static ActSubtab s_act_sub     = ACT_ATTACKS;
static int       s_act_cursor  = 0;   // Index in der aktiven (gefilterten) Liste
static int       s_act_scroll  = 0;   // Scroll-Offset

// ---- Persistente Buttons ---------------------------------------------------
// 4 Tabs: x=10, w=72, gap=4 → 4×72+3×4=300px
static UiButton s_btn_tab[TAB_COUNT];
static UiButton s_btn_hp_minus, s_btn_hp_plus, s_btn_edit_hp, s_btn_inspiration;
static UiButton s_btn_death_suc, s_btn_death_fail;
static UiButton s_btn_nav_inv, s_btn_nav_spell, s_btn_nav_notes, s_btn_nav_dice;
static UiButton s_btn_back;

// Edit: TAB_STATS
static char     s_lbl_ab[ABILITY_COUNT][12];
static char     s_lbl_sv[ABILITY_COUNT][8];
static char     s_lbl_maxhp[12], s_lbl_ac[10], s_lbl_spd[10], s_lbl_lv[10];
static char     s_lbl_class[20], s_lbl_race[20], s_lbl_bg[20];
static UiButton s_btn_ab[ABILITY_COUNT];
static UiButton s_btn_sv[ABILITY_COUNT];
static UiButton s_btn_maxhp, s_btn_ac, s_btn_spd, s_btn_lv;
static UiButton s_btn_class, s_btn_race, s_btn_bg;
static UiButton s_btn_levelup;

// Edit: TAB_INFO
static char     s_lbl_name_btn[24], s_lbl_align_btn[24], s_lbl_xp_btn[16];
static UiButton s_btn_name_edit, s_btn_align_edit, s_btn_xp_edit;
static UiButton s_btn_traits, s_btn_ideals, s_btn_bonds, s_btn_flaws, s_btn_backstory;

// Edit: TAB_ACTIONS
static UiButton s_btn_act_atk, s_btn_act_feat; // Sub-Tab Toggle
static UiButton s_btn_act_filt[4];              // Filter: Alle/Aktion/Bonus/Reaktion/Sonst
static UiButton s_btn_act_add, s_btn_act_edit, s_btn_act_del;

// ---- Hilfsfunktionen -------------------------------------------------------
static u32 hp_color(int current, int max) {
    if (max <= 0) return COLOR_TEXT;
    float r = (float)current / max;
    if (r > 0.5f) return COLOR_HP_GREEN;
    if (r > 0.25f) return COLOR_HP_YELLOW;
    return COLOR_HP_RED;
}

static void rebuild_attack_filter(void) {
    s_filt_attack_count = 0;
    for (int i = 0; i < s_attack_count; i++) {
        if (s_act_filter < 0 || s_attacks[i].action_type == s_act_filter)
            s_filt_attacks[s_filt_attack_count++] = i;
    }
}

static void reload_actions(void) {
    s_attack_count  = attacks_db_load(g_active_char_id, s_attacks, ATTACKS_MAX);
    s_feature_count = features_db_load(g_active_char_id, s_features, FEATURES_MAX);
    if (s_attack_count  < 0) s_attack_count  = 0;
    if (s_feature_count < 0) s_feature_count = 0;
    rebuild_attack_filter();
    s_act_cursor = 0;
    s_act_scroll = 0;
}

static int act_count(void) {
    return (s_act_sub == ACT_ATTACKS) ? s_filt_attack_count : s_feature_count;
}

static void refresh_labels(void) {
    for (int i = 0; i < ABILITY_COUNT; i++) {
        snprintf(s_lbl_ab[i], sizeof(s_lbl_ab[i]), "%s:%d", ABILITY_SHORT[i], s_char.ability[i]);
        snprintf(s_lbl_sv[i], sizeof(s_lbl_sv[i]), "SV%s", ABILITY_SHORT[i]);
        s_btn_ab[i].label = s_lbl_ab[i];
        s_btn_sv[i].label = s_lbl_sv[i];
    }
    snprintf(s_lbl_maxhp, sizeof(s_lbl_maxhp), "HP:%d",  s_char.hp_max);
    snprintf(s_lbl_ac,    sizeof(s_lbl_ac),    "AC:%d",  s_char.armor_class);
    snprintf(s_lbl_spd,   sizeof(s_lbl_spd),   "Spd:%d", s_char.speed);
    snprintf(s_lbl_lv,    sizeof(s_lbl_lv),    "Lv:%d",  s_char.level);
    snprintf(s_lbl_class, sizeof(s_lbl_class), "%.18s",  s_char.class_name);
    snprintf(s_lbl_race,  sizeof(s_lbl_race),  "%.18s",  s_char.race);
    snprintf(s_lbl_bg,    sizeof(s_lbl_bg),    "%.18s",  s_char.background);
    s_btn_maxhp.label = s_lbl_maxhp;
    s_btn_ac.label    = s_lbl_ac;
    s_btn_spd.label   = s_lbl_spd;
    s_btn_lv.label    = s_lbl_lv;
    s_btn_class.label = s_lbl_class;
    s_btn_race.label  = s_lbl_race;
    s_btn_bg.label    = s_lbl_bg;

    snprintf(s_lbl_name_btn,  sizeof(s_lbl_name_btn),  "%.22s", s_char.name);
    snprintf(s_lbl_align_btn, sizeof(s_lbl_align_btn), "%.22s", s_char.alignment);
    snprintf(s_lbl_xp_btn,    sizeof(s_lbl_xp_btn),    "XP:%d", s_char.experience);
    s_btn_name_edit.label  = s_lbl_name_btn;
    s_btn_align_edit.label = s_lbl_align_btn;
    s_btn_xp_edit.label    = s_lbl_xp_btn;
}

// ---- on_enter --------------------------------------------------------------
static void on_enter(void) {
    s_loaded       = (character_db_load(g_active_char_id, &s_char) == 0);
    s_tab          = TAB_STATS;
    s_skill_cursor = 0;
    s_act_filter   = -1;
    reload_actions();

    // === NEUES LAYOUT: Tab-Bar oben, persistente Nav-Strip unten ===============
    // Tab-Bar: y=0-23 (24px) — volle Breite, 4 Tabs
    {
        float tw = 74.0f, tgap = 3.0f, tx = 4.0f;
        s_btn_tab[TAB_STATS]   = (UiButton){ tx+0*(tw+tgap), 0, tw, 24, "Stats",    COLOR_ACCENT,     COLOR_TEXT, 0 };
        s_btn_tab[TAB_SKILLS]  = (UiButton){ tx+1*(tw+tgap), 0, tw, 24, "Skills",   COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_tab[TAB_INFO]    = (UiButton){ tx+2*(tw+tgap), 0, tw, 24, "Info",     COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_tab[TAB_ACTIONS] = (UiButton){ tx+3*(tw+tgap), 0, tw, 24, "Aktionen", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    }

    // Persistente Bottom-Strip: y=214-239 (26px) — immer sichtbar
    // HP-Kontrollen:
    s_btn_hp_minus    = (UiButton){   4, 216, 30, 22, "-",     COLOR_ACCENT,     COLOR_TEXT, 0 };
    s_btn_hp_plus     = (UiButton){  38, 216, 30, 22, "+",     COLOR_HP_GREEN,   COLOR_TEXT, 0 };
    s_btn_edit_hp     = (UiButton){  72, 216, 52, 22, "Set HP",COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_inspiration = (UiButton){ 128, 216, 28, 22, "★",     COLOR_BTN_NORMAL, COLOR_GOLD, 0 };
    // Navigation:
    s_btn_nav_inv   = (UiButton){ 160, 216, 36, 22, "Inv",  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_nav_spell = (UiButton){ 200, 216, 36, 22, "Zau",  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_nav_notes = (UiButton){ 240, 216, 36, 22, "Not",  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_nav_dice  = (UiButton){ 280, 216, 36, 22, "W6",   COLOR_BTN_NORMAL, COLOR_TEXT, 0 };

    // Zurück-Button (kleiner, in Inhaltsbereich, nur für Touch)
    s_btn_back = (UiButton){ 4, 191, 60, 18, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };

    // --- Death Saves (im TAB_STATS Inhaltsbereich, y=28) ---
    s_btn_death_suc  = (UiButton){ 160, 29, 72, 20, "+Erfolg",  COLOR_HP_GREEN, COLOR_TEXT, 0 };
    s_btn_death_fail = (UiButton){ 238, 29, 78, 20, "+Scheit.",  COLOR_HP_RED,   COLOR_TEXT, 0 };

    // --- Edit: Ability (2×3 Buttons, Content-Bereich y=53) ---
    {
        float bx = 4.0f, by = 53.0f, bw = 100.0f, bh = 26.0f, gap = 4.0f;
        for (int i = 0; i < ABILITY_COUNT; i++) {
            int col = i % 3, row = i / 3;
            s_btn_ab[i] = (UiButton){
                bx + col * (bw + gap), by + row * (bh + gap), bw, bh,
                s_lbl_ab[i], COLOR_BTN_NORMAL, COLOR_TEXT, 0
            };
        }
    }
    // --- Edit: Save Profs (y=113) ---
    {
        float bx = 4.0f, by = 113.0f, bw = 48.0f, bh = 18.0f, gap = 3.0f;
        for (int i = 0; i < ABILITY_COUNT; i++) {
            s_btn_sv[i] = (UiButton){
                bx + i * (bw + gap), by, bw, bh,
                s_lbl_sv[i], COLOR_BTN_NORMAL, COLOR_TEXT, 0
            };
        }
    }
    // --- Edit: MaxHP/AC/Speed/Level (y=135) ---
    {
        float bx = 4.0f, by = 135.0f, bw = 72.0f, bh = 18.0f, gap = 4.0f;
        s_btn_maxhp = (UiButton){ bx+0*(bw+gap), by, bw, bh, s_lbl_maxhp, COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_ac    = (UiButton){ bx+1*(bw+gap), by, bw, bh, s_lbl_ac,    COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_spd   = (UiButton){ bx+2*(bw+gap), by, bw, bh, s_lbl_spd,   COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_lv    = (UiButton){ bx+3*(bw+gap), by, bw, bh, s_lbl_lv,    COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    }
    // --- Edit: Klasse/Volk/HG (y=157) ---
    {
        float bx = 4.0f, by = 157.0f, bw = 97.0f, bh = 18.0f, gap = 4.0f;
        s_btn_class = (UiButton){ bx+0*(bw+gap), by, bw, bh, s_lbl_class, COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_race  = (UiButton){ bx+1*(bw+gap), by, bw, bh, s_lbl_race,  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_bg    = (UiButton){ bx+2*(bw+gap), by, bw, bh, s_lbl_bg,    COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    }

    // --- Stufe-erhoehen-Button (y=179, nur im Stats-Tab) ---
    s_btn_levelup = (UiButton){ 4, 179, 200, 18, "Stufe erhoehen >>", COLOR_ACCENT, COLOR_TEXT, 0 };

    // --- Edit: TAB_INFO (Content-Bereich y=28..) ---
    {
        float fw = 308.0f, hw = 148.0f, gap = 8.0f, bh = 20.0f, bx = 4.0f;
        s_btn_name_edit  = (UiButton){ bx,         28, hw,  bh, s_lbl_name_btn,  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_align_edit = (UiButton){ bx+hw+gap,  28, hw,  bh, s_lbl_align_btn, COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_xp_edit    = (UiButton){ bx,         52, fw,  bh, s_lbl_xp_btn,    COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_traits     = (UiButton){ bx,         76, fw,  bh, "Merkmale",       COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_ideals     = (UiButton){ bx,        100, fw,  bh, "Ideale",         COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_bonds      = (UiButton){ bx,        124, fw,  bh, "Bindungen",      COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_flaws      = (UiButton){ bx,        148, fw,  bh, "Fehler",         COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        s_btn_backstory  = (UiButton){ bx,        172, fw,  bh, "Geschichte",     COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    }

    // --- TAB_ACTIONS Sub-Tabs + Buttons (Content-Bereich y=28..) ---
    s_btn_act_atk  = (UiButton){   4, 28, 152, 22, "Angriffe",     COLOR_ACCENT,     COLOR_TEXT, 0 };
    s_btn_act_feat = (UiButton){ 160, 28, 156, 22, "Faehigkeiten", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    // Filter-Buttons (y=54, für Angriffe): [Alle][Aktion][Bonus][Reaktion]
    {
        static const char* filt_labels[4] = {"Alle","Akt.","Bonus","Reak."};
        float fx = 4.0f, fw = 73.0f, fg = 4.0f;
        for (int i = 0; i < 4; i++)
            s_btn_act_filt[i] = (UiButton){ fx + i*(fw+fg), 54, fw, 18,
                                             filt_labels[i], COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    }
    // Aktions-Buttons: y=168
    s_btn_act_add  = (UiButton){   4, 168, 96, 20, "+ Neu",     COLOR_ACCENT,     COLOR_TEXT, 0 };
    s_btn_act_edit = (UiButton){ 106, 168, 96, 20, "Bearbeit.", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_act_del  = (UiButton){ 208, 168, 96, 20, "Loeschen",  COLOR_ACCENT,     COLOR_TEXT, 0 };

    refresh_labels();
}

// ---- on_exit ---------------------------------------------------------------
static void on_exit(void) {
    if (s_loaded) character_db_save(&s_char);
}

// ---- Hilfsfunktion: Attack/Feature per Tastatur eingeben -------------------
static void edit_attack(Attack* a) {
    ui_keyboard_input("Angriff - Name:", a->name, ATTACK_NAME_MAX);
    ui_keyboard_input("Trefferbonus (z.B. +4):", a->hit_bonus, ATTACK_BONUS_MAX);
    ui_keyboard_input("Schaden (z.B. 1d6+2):", a->damage, ATTACK_DMG_MAX);
    ui_keyboard_input("Schadenstyp (z.B. Stich):", a->damage_type, ATTACK_DMG_MAX);
    ui_keyboard_input("Notizen (optional):", a->notes, ATTACK_NOTE_MAX);
    // Aktionstyp: 0=Aktion 1=Bonusaktion 2=Reaktion 3=Sonstiges
    ui_keyboard_input_numeric("Aktionstyp (0=Aktion 1=Bonus 2=Reaktion 3=Sonst.):",
                               &a->action_type, 0, 3);
    attacks_db_save(a);
}

static void edit_feature(Feature* f) {
    ui_keyboard_input("Fähigkeit - Name:", f->name, FEATURE_NAME_MAX);
    ui_keyboard_input("Quelle (z.B. Barde 1):", f->source, FEATURE_SRC_MAX);
    ui_keyboard_input("Beschreibung:", f->description, FEATURE_DESC_MAX);
    ui_keyboard_input_numeric("Max. Verwendungen (0=passiv):", &f->uses_max, 0, 99);
    f->uses_current = f->uses_max;  // Startwert = max
    ui_keyboard_input("Aufladung bei (Langer Rast/Kurzer Rast/...):", f->recharge_on, FEATURE_RECH_MAX);
    features_db_save(f);
}

// ---- update ----------------------------------------------------------------
static void update(u32 keys_down, u32 keys_held, touchPosition* touch, int touch_down) {
    (void)keys_held;
    if (!s_loaded) return;

    if (keys_down & KEY_B) { screen_pop(); return; }
    if (keys_down & KEY_L) s_tab = (CharTab)((s_tab + TAB_COUNT - 1) % TAB_COUNT);
    if (keys_down & KEY_R) s_tab = (CharTab)((s_tab + 1) % TAB_COUNT);

    // ---- Tab-spezifische Physik-Buttons ------------------------------------
    if (s_tab == TAB_SKILLS) {
        if (keys_down & KEY_UP)
            s_skill_cursor = (s_skill_cursor + SKILL_COUNT - 1) % SKILL_COUNT;
        if (keys_down & KEY_DOWN)
            s_skill_cursor = (s_skill_cursor + 1) % SKILL_COUNT;
        if (keys_down & KEY_A) {
            s_char.skill_proficient[s_skill_cursor] =
                (s_char.skill_proficient[s_skill_cursor] + 1) % 3;
            character_db_save(&s_char);
        }
        if (keys_down & KEY_Y) {
            s_char.skill_proficient[s_skill_cursor] = 0;
            character_db_save(&s_char);
        }
    } else if (s_tab == TAB_ACTIONS) {
        // Maximale sichtbare Zeilen je nach Sub-Tab
        int max_rows = (s_act_sub == ACT_ATTACKS) ? 5 : 7;
        int cnt = act_count();
        if (keys_down & KEY_UP && cnt > 0)
            s_act_cursor = (s_act_cursor + cnt - 1) % cnt;
        if (keys_down & KEY_DOWN && cnt > 0)
            s_act_cursor = (s_act_cursor + 1) % cnt;
        // Scroll-Offset anpassen
        if (s_act_cursor < s_act_scroll) s_act_scroll = s_act_cursor;
        if (s_act_cursor >= s_act_scroll + max_rows) s_act_scroll = s_act_cursor - (max_rows - 1);

        // [A] = Verwendung verbrauchen (nur bei Features mit uses_max>0)
        if ((keys_down & KEY_A) && s_act_sub == ACT_FEATURES
            && s_act_cursor < s_feature_count) {
            Feature* f = &s_features[s_act_cursor];
            if (f->uses_max > 0 && f->uses_current > 0) {
                f->uses_current--;
                features_db_use(f->id, f->uses_current);
            }
        }
        // [Y] = Verwendungen wiederherstellen
        if ((keys_down & KEY_Y) && s_act_sub == ACT_FEATURES
            && s_act_cursor < s_feature_count) {
            Feature* f = &s_features[s_act_cursor];
            if (f->uses_max > 0) {
                f->uses_current = f->uses_max;
                features_db_use(f->id, f->uses_current);
            }
        }
    } else {
        // DPad = schnelle HP-Änderung
        if (keys_down & KEY_UP) {
            s_char.hp_current = dnd_hp_clamp(s_char.hp_current + 1, s_char.hp_max);
            character_db_save_hp(s_char.id, s_char.hp_current, s_char.hp_temp);
        }
        if (keys_down & KEY_DOWN) {
            s_char.hp_current = dnd_hp_clamp(s_char.hp_current - 1, s_char.hp_max);
            character_db_save_hp(s_char.id, s_char.hp_current, s_char.hp_temp);
        }
    }

    if (!touch_down) return;

    // --- Tabs ---
    for (int i = 0; i < TAB_COUNT; i++) {
        if (ui_button_touched(&s_btn_tab[i], touch)) { s_tab = (CharTab)i; return; }
    }

    // --- HP-Zeile ---
    if (ui_button_touched(&s_btn_hp_minus, touch)) {
        s_char.hp_current = dnd_hp_clamp(s_char.hp_current - 1, s_char.hp_max);
        character_db_save_hp(s_char.id, s_char.hp_current, s_char.hp_temp);
        return;
    }
    if (ui_button_touched(&s_btn_hp_plus, touch)) {
        s_char.hp_current = dnd_hp_clamp(s_char.hp_current + 1, s_char.hp_max);
        character_db_save_hp(s_char.id, s_char.hp_current, s_char.hp_temp);
        return;
    }
    if (ui_button_touched(&s_btn_edit_hp, touch)) {
        int val = s_char.hp_current;
        if (ui_keyboard_input_numeric("Aktuelles HP:", &val, 0, s_char.hp_max + 50)) {
            s_char.hp_current = val;
            character_db_save_hp(s_char.id, s_char.hp_current, s_char.hp_temp);
        }
        return;
    }
    if (ui_button_touched(&s_btn_inspiration, touch)) {
        s_char.inspiration ^= 1;
        character_db_save_status(s_char.id, s_char.inspiration,
                                  s_char.death_saves_successes, s_char.death_saves_failures);
        return;
    }

    // --- Death Saves (nur in TAB_STATS sichtbar und anklickbar) ---
    if (s_tab == TAB_STATS) {
        if (ui_button_touched(&s_btn_death_suc, touch)) {
            if (s_char.death_saves_successes < 3) s_char.death_saves_successes++;
            character_db_save_status(s_char.id, s_char.inspiration,
                                      s_char.death_saves_successes, s_char.death_saves_failures);
            return;
        }
        if (ui_button_touched(&s_btn_death_fail, touch)) {
            if (s_char.death_saves_failures < 3) s_char.death_saves_failures++;
            character_db_save_status(s_char.id, s_char.inspiration,
                                      s_char.death_saves_successes, s_char.death_saves_failures);
            return;
        }
    }

    // --- Screen-Navigation ---
    if (ui_button_touched(&s_btn_nav_inv, touch))   { screen_push(SCREEN_INVENTORY); return; }
    if (ui_button_touched(&s_btn_nav_spell, touch)) { screen_push(SCREEN_SPELLS);    return; }
    if (ui_button_touched(&s_btn_nav_notes, touch)) { screen_push(SCREEN_NOTES);     return; }
    if (ui_button_touched(&s_btn_nav_dice, touch))  { screen_push(SCREEN_DICE);      return; }
    if (ui_button_touched(&s_btn_back, touch)) { screen_pop(); return; }

    // === Tab-spezifische Touch-Logik =========================================

    // --- TAB_SKILLS: Proficiency-Buttons (y=118, 3 Buttons) ---
    if (s_tab == TAB_SKILLS) {
        float bx = 4.0f, bw = 98.0f, gap = 4.0f;
        for (int j = 0; j < 3; j++) {
            if (ui_point_in_rect((float)touch->px, (float)touch->py,
                                  bx + j*(bw+gap), 118.0f, bw, 22.0f)) {
                s_char.skill_proficient[s_skill_cursor] = j;
                character_db_save(&s_char);
                return;
            }
        }
        return;
    }

    // --- TAB_STATS ---
    if (s_tab == TAB_STATS) {
        for (int i = 0; i < ABILITY_COUNT; i++) {
            if (ui_button_touched(&s_btn_ab[i], touch)) {
                int val = s_char.ability[i];
                if (ui_keyboard_input_numeric(ABILITY_NAMES[i], &val, 1, 30)) {
                    s_char.ability[i] = val;
                    character_db_save(&s_char);
                    refresh_labels();
                }
                return;
            }
        }
        for (int i = 0; i < ABILITY_COUNT; i++) {
            if (ui_button_touched(&s_btn_sv[i], touch)) {
                s_char.save_proficient[i] ^= 1;
                character_db_save(&s_char);
                return;
            }
        }
        if (ui_button_touched(&s_btn_maxhp, touch)) {
            int val = s_char.hp_max;
            if (ui_keyboard_input_numeric("Maximale HP:", &val, 1, 999)) {
                s_char.hp_max = val;
                if (s_char.hp_current > s_char.hp_max) s_char.hp_current = s_char.hp_max;
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_ac, touch)) {
            int val = s_char.armor_class;
            if (ui_keyboard_input_numeric("Ruestungsklasse:", &val, 0, 30)) {
                s_char.armor_class = val;
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_spd, touch)) {
            int val = s_char.speed;
            if (ui_keyboard_input_numeric("Bewegungsrate:", &val, 0, 200)) {
                s_char.speed = val;
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_lv, touch)) {
            int val = s_char.level;
            if (ui_keyboard_input_numeric("Stufe (Level):", &val, 1, 20)) {
                s_char.level = val;
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_class, touch)) {
            char buf[CHAR_NAME_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.class_name);
            if (ui_keyboard_input("Klasse:", buf, CHAR_NAME_MAX)) {
                snprintf(s_char.class_name, CHAR_NAME_MAX, "%s", buf);
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_race, touch)) {
            char buf[CHAR_NAME_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.race);
            if (ui_keyboard_input("Volk (Rasse):", buf, CHAR_NAME_MAX)) {
                snprintf(s_char.race, CHAR_NAME_MAX, "%s", buf);
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_levelup, touch) && s_char.level < 20) {
            screen_push(SCREEN_LEVELUP);
            return;
        }
        if (ui_button_touched(&s_btn_bg, touch)) {
            char buf[CHAR_NAME_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.background);
            if (ui_keyboard_input("Hintergrund:", buf, CHAR_NAME_MAX)) {
                snprintf(s_char.background, CHAR_NAME_MAX, "%s", buf);
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
    }

    // --- TAB_INFO ---
    if (s_tab == TAB_INFO) {
        if (ui_button_touched(&s_btn_name_edit, touch)) {
            char buf[CHAR_NAME_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.name);
            if (ui_keyboard_input("Name:", buf, CHAR_NAME_MAX)) {
                snprintf(s_char.name, CHAR_NAME_MAX, "%s", buf);
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_align_edit, touch)) {
            char buf[CHAR_NAME_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.alignment);
            if (ui_keyboard_input("Gesinnung:", buf, CHAR_NAME_MAX)) {
                snprintf(s_char.alignment, CHAR_NAME_MAX, "%s", buf);
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_xp_edit, touch)) {
            int val = s_char.experience;
            if (ui_keyboard_input_numeric("Erfahrungspunkte:", &val, 0, 999999)) {
                s_char.experience = val;
                character_db_save(&s_char);
                refresh_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_traits, touch)) {
            char buf[CHAR_TEXT_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.personality_traits);
            if (ui_keyboard_input("Zuegedisposition:", buf, CHAR_TEXT_MAX)) {
                snprintf(s_char.personality_traits, CHAR_TEXT_MAX, "%s", buf);
                character_db_save(&s_char);
            }
            return;
        }
        if (ui_button_touched(&s_btn_ideals, touch)) {
            char buf[CHAR_TEXT_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.ideals);
            if (ui_keyboard_input("Ideale:", buf, CHAR_TEXT_MAX)) {
                snprintf(s_char.ideals, CHAR_TEXT_MAX, "%s", buf);
                character_db_save(&s_char);
            }
            return;
        }
        if (ui_button_touched(&s_btn_bonds, touch)) {
            char buf[CHAR_TEXT_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.bonds);
            if (ui_keyboard_input("Bindungen:", buf, CHAR_TEXT_MAX)) {
                snprintf(s_char.bonds, CHAR_TEXT_MAX, "%s", buf);
                character_db_save(&s_char);
            }
            return;
        }
        if (ui_button_touched(&s_btn_flaws, touch)) {
            char buf[CHAR_TEXT_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.flaws);
            if (ui_keyboard_input("Fehler:", buf, CHAR_TEXT_MAX)) {
                snprintf(s_char.flaws, CHAR_TEXT_MAX, "%s", buf);
                character_db_save(&s_char);
            }
            return;
        }
        if (ui_button_touched(&s_btn_backstory, touch)) {
            char buf[CHAR_LONG_MAX];
            snprintf(buf, sizeof(buf), "%s", s_char.backstory);
            if (ui_keyboard_input("Geschichte:", buf, CHAR_LONG_MAX)) {
                snprintf(s_char.backstory, CHAR_LONG_MAX, "%s", buf);
                character_db_save(&s_char);
            }
            return;
        }
    }

    // --- TAB_ACTIONS ---
    if (s_tab == TAB_ACTIONS) {
        // Sub-Tab Toggle
        if (ui_button_touched(&s_btn_act_atk, touch)) {
            s_act_sub = ACT_ATTACKS;
            s_act_cursor = 0; s_act_scroll = 0;
            return;
        }
        if (ui_button_touched(&s_btn_act_feat, touch)) {
            s_act_sub = ACT_FEATURES;
            s_act_cursor = 0; s_act_scroll = 0;
            return;
        }

        // Filter-Buttons (Angriffe): -1=Alle, 0-3=Aktionstyp
        if (s_act_sub == ACT_ATTACKS) {
            for (int i = 0; i < 4; i++) {
                if (ui_button_touched(&s_btn_act_filt[i], touch)) {
                    s_act_filter = (i == 0) ? -1 : (i - 1);
                    rebuild_attack_filter();
                    s_act_cursor = 0; s_act_scroll = 0;
                    return;
                }
            }
        }

        // + Neu
        if (ui_button_touched(&s_btn_act_add, touch)) {
            if (s_act_sub == ACT_ATTACKS) {
                Attack a = {0};
                a.character_id = g_active_char_id;
                snprintf(a.hit_bonus, sizeof(a.hit_bonus), "+0");
                snprintf(a.damage,    sizeof(a.damage),    "1d6");
                edit_attack(&a);
            } else {
                Feature f = {0};
                f.character_id = g_active_char_id;
                edit_feature(&f);
            }
            reload_actions();
            return;
        }

        int cnt = act_count();

        // Bearbeiten
        if (ui_button_touched(&s_btn_act_edit, touch)) {
            if (s_act_sub == ACT_ATTACKS && s_act_cursor < s_filt_attack_count) {
                edit_attack(&s_attacks[s_filt_attacks[s_act_cursor]]);
                reload_actions();
            } else if (s_act_sub == ACT_FEATURES && s_act_cursor < s_feature_count) {
                edit_feature(&s_features[s_act_cursor]);
                reload_actions();
            }
            return;
        }

        // Löschen
        if (ui_button_touched(&s_btn_act_del, touch)) {
            if (s_act_sub == ACT_ATTACKS && s_act_cursor < s_filt_attack_count) {
                attacks_db_delete(s_attacks[s_filt_attacks[s_act_cursor]].id);
                reload_actions();
            } else if (s_act_sub == ACT_FEATURES && s_act_cursor < s_feature_count) {
                features_db_delete(s_features[s_act_cursor].id);
                reload_actions();
            }
            return;
        }

        // Liste antippen → Cursor setzen (neue y-Positionen)
        {
            int max_rows = (s_act_sub == ACT_ATTACKS) ? 5 : 7;
            float list_y = (s_act_sub == ACT_ATTACKS) ? 76.0f : 54.0f;
            for (int i = 0; i < max_rows && i < cnt; i++) {
                int idx = s_act_scroll + i;
                if (idx >= cnt) break;
                float iy = list_y + 2.0f + i * 15.0f;
                if (ui_point_in_rect((float)touch->px, (float)touch->py, 4, iy, 312, 15)) {
                    s_act_cursor = idx;
                    // Tippen auf Feature = Verwendung verbrauchen
                    if (s_act_sub == ACT_FEATURES && idx < s_feature_count) {
                        Feature* f = &s_features[idx];
                        if (f->uses_max > 0 && f->uses_current > 0) {
                            f->uses_current--;
                            features_db_use(f->id, f->uses_current);
                        }
                    }
                    return;
                }
            }
        }
    }
}

// ---- Zeichnen Top-Screen ---------------------------------------------------
static void draw_top(void) {
    if (!s_loaded) {
        ui_draw_text(10, 100, 0.5f, 0.5f, COLOR_TEXT, "Fehler: Charakter nicht geladen.");
        return;
    }

    // Header
    ui_draw_rect(0, 0, SCREEN_TOP_W, 24, COLOR_PANEL);
    ui_draw_textf(10, 4, 0.5f, 0.58f, COLOR_GOLD, "%s", s_char.name);
    ui_draw_textf(240, 4, 0.5f, 0.42f, COLOR_TEXT_DIM,
                  "Lv%d %s %s", s_char.level, s_char.race, s_char.class_name);
    if (s_char.inspiration)
        ui_draw_text(370, 4, 0.5f, 0.38f, COLOR_GOLD, "[INSP]");

    ui_draw_hp_bar(10, 28, 180, 12, s_char.hp_current, s_char.hp_max);
    ui_draw_textf(196, 26, 0.5f, 0.43f, hp_color(s_char.hp_current, s_char.hp_max),
                  "%d/%d HP", s_char.hp_current, s_char.hp_max);
    if (s_char.hp_temp > 0)
        ui_draw_textf(196, 39, 0.5f, 0.36f, COLOR_HP_GREEN, "+%d tmp", s_char.hp_temp);

    ui_draw_textf(300, 26, 0.5f, 0.42f, COLOR_TEXT, "AC %d", s_char.armor_class);
    ui_draw_textf(340, 26, 0.5f, 0.42f, COLOR_TEXT, "Spd %d", s_char.speed);
    ui_draw_textf(300, 40, 0.5f, 0.38f, COLOR_TEXT_DIM, "Init %+d",
                  dnd_modifier(s_char.ability[ABILITY_DEX]));

    ui_draw_rect(0, 46, SCREEN_TOP_W, 1, COLOR_ACCENT);

    int pb = dnd_proficiency_bonus(s_char.level);

    // ---- TAB: Stats --------------------------------------------------------
    if (s_tab == TAB_STATS) {
        for (int i = 0; i < ABILITY_COUNT; i++) {
            int col = i % 2, row = i / 2;
            float x = 10.0f + col * 195.0f;
            float y = 52.0f + row * 55.0f;
            int score = s_char.ability[i];
            int mod   = dnd_modifier(score);
            int save  = mod + (s_char.save_proficient[i] ? pb : 0);

            ui_draw_rect(x, y, 180, 48, COLOR_PANEL);
            ui_draw_rect_outline(x, y, 180, 48, 1.0f,
                                 s_char.save_proficient[i] ? COLOR_HP_GREEN : COLOR_TEXT_DIM);
            ui_draw_textf(x +  4, y +  3, 0.5f, 0.38f, COLOR_TEXT_DIM, "%s", ABILITY_SHORT[i]);
            ui_draw_textf(x + 36, y +  3, 0.5f, 0.55f, COLOR_TEXT, "%d", score);
            ui_draw_textf(x +  4, y + 26, 0.5f, 0.60f, COLOR_ACCENT2, mod >= 0 ? "+%d" : "%d", mod);
            u32 sc = s_char.save_proficient[i] ? COLOR_HP_GREEN : COLOR_TEXT_DIM;
            ui_draw_textf(x + 80, y + 26, 0.5f, 0.40f, sc, "Save%+d", save);
            if (i == ABILITY_WIS) {
                int sp = s_char.skill_proficient[SKILL_PERCEPTION];
                int passive = 10 + mod + (sp > 0 ? pb * sp : 0);
                ui_draw_textf(x + 80, y + 12, 0.5f, 0.35f, COLOR_TEXT_DIM, "Passiv:%d", passive);
            }
        }
        ui_draw_textf(10, 220, 0.5f, 0.38f, COLOR_TEXT_DIM, "Prof.Bonus: +%d  |  DPad: HP", pb);

    // ---- TAB: Skills -------------------------------------------------------
    } else if (s_tab == TAB_SKILLS) {
        for (int i = 0; i < SKILL_COUNT; i++) {
            int col = i % 2, row = i / 2;
            float x = 8.0f  + col * 196.0f;
            float y = 52.0f + row * 21.0f;
            int ability_mod = dnd_modifier(s_char.ability[SKILL_ABILITY[i]]);
            int prof        = s_char.skill_proficient[i];
            int total       = ability_mod + (prof > 0 ? pb * prof : 0);
            u32 col_text = (prof == 2) ? COLOR_GOLD :
                           (prof == 1) ? COLOR_HP_GREEN : COLOR_TEXT_DIM;
            char prefix = (prof == 2) ? 'E' : (prof == 1) ? '*' : ' ';
            if (i == s_skill_cursor) {
                ui_draw_rect(x - 2, y - 1, 192, 19, COLOR_PANEL);
                ui_draw_rect_outline(x - 2, y - 1, 192, 19, 1.0f, COLOR_ACCENT2);
                col_text = COLOR_WHITE;
            }
            ui_draw_textf(x, y, 0.5f, 0.38f, col_text,
                          "%c %-13s %+3d", prefix, s_skill_names[i], total);
        }
        ui_draw_text(8, 222, 0.5f, 0.33f, COLOR_TEXT_DIM,
                     "* Proficient   E Expertise   DPad: auswaehlen");

    // ---- TAB: Info ---------------------------------------------------------
    } else if (s_tab == TAB_INFO) {
        float y = 52.0f;
        struct { const char* label; const char* value; } fields[] = {
            { "Rasse:",       s_char.race },
            { "Klasse:",      s_char.class_name },
            { "Unterklasse:", s_char.subclass },
            { "Hintergrund:", s_char.background },
            { "Gesinnung:",   s_char.alignment },
        };
        for (int i = 0; i < 5; i++) {
            ui_draw_text(10,  y, 0.5f, 0.42f, COLOR_TEXT_DIM, fields[i].label);
            ui_draw_text(130, y, 0.5f, 0.42f, COLOR_TEXT,     fields[i].value);
            y += 20.0f;
        }
        ui_draw_textf(10,  y, 0.5f, 0.42f, COLOR_TEXT_DIM, "EP:");
        ui_draw_textf(130, y, 0.5f, 0.42f, COLOR_TEXT, "%d", s_char.experience);
        y += 24.0f;
        ui_draw_rect(0, y, SCREEN_TOP_W, 1, COLOR_ACCENT);
        y += 4.0f;
        ui_draw_text(10, y +  0, 0.5f, 0.38f, COLOR_TEXT_DIM, "Zuegedisposition:");
        ui_draw_text(10, y + 14, 0.5f, 0.36f, COLOR_TEXT,     s_char.personality_traits);
        ui_draw_text(10, y + 34, 0.5f, 0.38f, COLOR_TEXT_DIM, "Ideale:");
        ui_draw_text(10, y + 48, 0.5f, 0.36f, COLOR_TEXT,     s_char.ideals);
        ui_draw_text(10, y + 68, 0.5f, 0.38f, COLOR_TEXT_DIM, "Bindungen:");
        ui_draw_text(10, y + 82, 0.5f, 0.36f, COLOR_TEXT,     s_char.bonds);

    // ---- TAB: Aktionen — Detail des ausgewählten Eintrags -----------------
    } else {
        if (s_act_sub == ACT_ATTACKS) {
            if (s_act_cursor < s_filt_attack_count) {
                Attack* a = &s_attacks[s_filt_attacks[s_act_cursor]];
                // Aktionstyp-Badge + Name
                u32 bc = (a->action_type == 0) ? COLOR_HP_GREEN :
                         (a->action_type == 1) ? COLOR_GOLD :
                         (a->action_type == 2) ? COLOR_ACCENT2 : COLOR_TEXT_DIM;
                ui_draw_textf(10, 50, 0.5f, 0.38f, bc,
                              "[%s] %s", ACT_TYPE_SHORT[a->action_type], ACT_TYPE_NAME[a->action_type]);
                ui_draw_textf(10, 68, 0.5f, 0.62f, COLOR_TEXT, "%s", a->name);
                ui_draw_rect(0, 90, SCREEN_TOP_W, 1, COLOR_ACCENT);

                float y = 96.0f;
                if (a->hit_bonus[0]) {
                    ui_draw_text(10, y, 0.5f, 0.38f, COLOR_TEXT_DIM, "Trefferbonus:");
                    ui_draw_textf(130, y, 0.5f, 0.42f, COLOR_ACCENT2, "%s", a->hit_bonus);
                    y += 18.0f;
                }
                if (a->damage[0]) {
                    ui_draw_text(10, y, 0.5f, 0.38f, COLOR_TEXT_DIM, "Schaden:");
                    ui_draw_textf(130, y, 0.5f, 0.42f, COLOR_TEXT,
                                  "%s%s%s", a->damage,
                                  a->damage_type[0] ? "  " : "",
                                  a->damage_type);
                    y += 18.0f;
                }
                if (a->notes[0]) {
                    ui_draw_rect(0, y + 2, SCREEN_TOP_W, 1, COLOR_TEXT_DIM);
                    y += 8.0f;
                    // Notes mehrzeilig
                    const char* src = a->notes;
                    char line[80]; int ci = 0, li = 0;
                    while (src[ci] && y < 234.0f) {
                        if (src[ci] == '\n' || li >= 78) {
                            line[li] = '\0';
                            ui_draw_text(10, y, 0.5f, 0.37f, COLOR_TEXT_DIM, line);
                            y += 14.0f; li = 0;
                            if (src[ci] == '\n') ci++;
                        } else { line[li++] = src[ci++]; }
                    }
                    if (li > 0) { line[li] = '\0'; ui_draw_text(10, y, 0.5f, 0.37f, COLOR_TEXT_DIM, line); }
                }
            } else {
                ui_draw_text(100, 130, 0.5f, 0.42f, COLOR_TEXT_DIM,
                             s_attack_count == 0 ? "Keine Angriffe" : "Angriff auswaehlen");
            }
        } else { // ACT_FEATURES
            if (s_act_cursor < s_feature_count) {
                Feature* f = &s_features[s_act_cursor];
                ui_draw_textf(10, 50, 0.5f, 0.62f, COLOR_TEXT, "%s", f->name);
                if (f->source[0])
                    ui_draw_textf(10, 72, 0.5f, 0.38f, COLOR_TEXT_DIM, "Quelle: %s", f->source);
                if (f->uses_max > 0) {
                    u32 uc = (f->uses_current > 0) ? COLOR_HP_GREEN : COLOR_HP_RED;
                    ui_draw_textf(10, 87, 0.5f, 0.38f, uc,
                                  "Verwendungen: %d/%d", f->uses_current, f->uses_max);
                    if (f->recharge_on[0])
                        ui_draw_textf(220, 87, 0.5f, 0.35f, COLOR_TEXT_DIM,
                                      "(%s)", f->recharge_on);
                }
                ui_draw_rect(0, 104, SCREEN_TOP_W, 1, COLOR_ACCENT);
                if (f->description[0]) {
                    const char* src = f->description;
                    float y = 110.0f;
                    char line[80]; int ci = 0, li = 0;
                    while (src[ci] && y < 234.0f) {
                        if (src[ci] == '\n' || li >= 78) {
                            line[li] = '\0';
                            ui_draw_text(10, y, 0.5f, 0.37f, COLOR_TEXT, line);
                            y += 14.0f; li = 0;
                            if (src[ci] == '\n') ci++;
                        } else { line[li++] = src[ci++]; }
                    }
                    if (li > 0) { line[li] = '\0'; ui_draw_text(10, y, 0.5f, 0.37f, COLOR_TEXT, line); }
                } else {
                    ui_draw_text(10, 112, 0.5f, 0.37f, COLOR_TEXT_DIM, "(Keine Beschreibung)");
                }
            } else {
                ui_draw_text(80, 130, 0.5f, 0.42f, COLOR_TEXT_DIM,
                             s_feature_count == 0 ? "Keine Faehigkeiten" : "Faehigkeit auswaehlen");
            }
        }
    }
}

// ---- Zeichnen Bottom-Screen ------------------------------------------------
static void draw_bottom(void) {
    if (!s_loaded) return;

    refresh_labels();

    // === TAB-BAR: y=0-23 ====================================================
    for (int i = 0; i < TAB_COUNT; i++) {
        s_btn_tab[i].color_bg = (i == (int)s_tab) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
        ui_button_draw(&s_btn_tab[i]);
    }
    ui_draw_separator(0, 24, 320, COLOR_TEXT_DIM);

    // === PERSISTENTE BOTTOM-STRIP: y=212-239 ================================
    ui_draw_rect(0, 212, 320, 28, COLOR_PANEL);
    ui_draw_separator(0, 212, 320, COLOR_ACCENT);

    // HP-Label (mini)
    ui_draw_textf(4, 213, 0.6f, 0.30f, COLOR_TEXT_DIM,
                  "HP:%d/%d", s_char.hp_current, s_char.hp_max);
    // HP-Buttons
    ui_button_draw(&s_btn_hp_minus);
    ui_button_draw(&s_btn_hp_plus);
    ui_button_draw(&s_btn_edit_hp);
    // Inspiration (★ = gefüllt wenn aktiv)
    s_btn_inspiration.color_bg   = s_char.inspiration ? COLOR_GOLD       : COLOR_BTN_NORMAL;
    s_btn_inspiration.color_text = s_char.inspiration ? COLOR_BLACK : COLOR_GOLD;
    ui_button_draw(&s_btn_inspiration);
    // Separator zwischen HP und Nav
    ui_draw_separator(158, 215, 1, COLOR_TEXT_DIM);
    // Nav-Buttons
    ui_button_draw(&s_btn_nav_inv);
    ui_button_draw(&s_btn_nav_spell);
    ui_button_draw(&s_btn_nav_notes);
    ui_button_draw(&s_btn_nav_dice);

    // === TAB-INHALT: y=26-210 ===============================================

    if (s_tab == TAB_STATS) {
        // --- Death Saves Zeile (y=28) ---
        ui_draw_text(4, 31, 0.5f, 0.36f, COLOR_TEXT_DIM, "Sterbend:");
        for (int i = 0; i < 3; i++) {
            u32 c = (i < s_char.death_saves_successes) ? COLOR_HP_GREEN : COLOR_PANEL2;
            ui_draw_rect(68 + i * 12, 31, 10, 10, c);
            ui_draw_rect_outline(68 + i * 12, 31, 10, 10, 1.0f, COLOR_TEXT_DIM);
        }
        for (int i = 0; i < 3; i++) {
            u32 c = (i < s_char.death_saves_failures) ? COLOR_HP_RED : COLOR_PANEL2;
            ui_draw_rect(110 + i * 12, 31, 10, 10, c);
            ui_draw_rect_outline(110 + i * 12, 31, 10, 10, 1.0f, COLOR_TEXT_DIM);
        }
        ui_button_draw(&s_btn_death_suc);
        ui_button_draw(&s_btn_death_fail);

        ui_draw_separator(0, 52, 320, COLOR_TEXT_DIM);

        // --- Ability-Buttons (2×3 Raster, y=53) ---
        for (int i = 0; i < ABILITY_COUNT; i++)
            ui_button_draw(&s_btn_ab[i]);

        ui_draw_separator(0, 111, 320, COLOR_TEXT_DIM);

        // --- Save Profs (y=113) ---
        ui_draw_text(4, 114, 0.6f, 0.32f, COLOR_TEXT_DIM, "Rettungsw.:");
        for (int i = 0; i < ABILITY_COUNT; i++) {
            s_btn_sv[i].color_bg = s_char.save_proficient[i] ? COLOR_HP_GREEN : COLOR_BTN_NORMAL;
            ui_button_draw(&s_btn_sv[i]);
        }

        ui_draw_separator(0, 133, 320, COLOR_TEXT_DIM);

        // --- MaxHP/AC/Speed/Level (y=135) ---
        ui_button_draw(&s_btn_maxhp);
        ui_button_draw(&s_btn_ac);
        ui_button_draw(&s_btn_spd);
        ui_button_draw(&s_btn_lv);

        ui_draw_separator(0, 155, 320, COLOR_TEXT_DIM);

        // --- Klasse/Volk/HG (y=157) ---
        ui_button_draw(&s_btn_class);
        ui_button_draw(&s_btn_race);
        ui_button_draw(&s_btn_bg);

        ui_draw_separator(0, 177, 320, COLOR_TEXT_DIM);
        if (s_char.level < 20) ui_button_draw(&s_btn_levelup);
        ui_button_draw(&s_btn_back);
        ui_draw_text(70, 195, 0.5f, 0.33f, COLOR_TEXT_DIM, "[L/R] Tab   [DPad] HP");

    } else if (s_tab == TAB_SKILLS) {
        int idx = s_skill_cursor;
        int pb  = dnd_proficiency_bonus(s_char.level);
        int mod = dnd_modifier(s_char.ability[SKILL_ABILITY[idx]]);
        int prf = s_char.skill_proficient[idx];
        int tot = mod + (prf > 0 ? pb * prf : 0);
        const char* prf_str = (prf == 2) ? "Expertise" : (prf == 1) ? "Proficient" : "Keine";
        u32 prf_col = (prf == 2) ? COLOR_GOLD : (prf == 1) ? COLOR_HP_GREEN : COLOR_TEXT_DIM;

        // Ausgewählter Skill - großes Info-Panel
        ui_draw_panel(4, 28, 312, 72, COLOR_PANEL, COLOR_ACCENT);
        ui_draw_textf(10, 32, 0.5f, 0.52f, COLOR_GOLD, "%s", s_skill_names[idx]);
        ui_draw_textf(10, 50, 0.5f, 0.42f, prf_col, "%s", prf_str);
        ui_draw_textf(200, 32, 0.5f, 0.60f, COLOR_TEXT, "%+d", tot);
        ui_draw_textf(200, 52, 0.5f, 0.36f, COLOR_TEXT_DIM, "Bonus");

        // Proficiency Touch-Buttons (y=106)
        ui_draw_text(4, 106, 0.5f, 0.36f, COLOR_TEXT_DIM, "Proficiency:");
        {
            const char* labels[3] = {"Keine", "Proficient", "Expertise"};
            u32         colors[3] = { COLOR_BTN_NORMAL, COLOR_HP_GREEN, COLOR_GOLD };
            float bx = 4.0f, bw = 98.0f, gap = 4.0f;
            for (int j = 0; j < 3; j++) {
                UiButton b = { bx + j*(bw+gap), 118, bw, 22,
                               labels[j], (prf == j) ? colors[j] : COLOR_BTN_NORMAL,
                               (prf == j) ? COLOR_BLACK : COLOR_TEXT, 0 };
                ui_button_draw(&b);
                // Touch-Hit für diese Buttons im update() behandelt
            }
        }

        // Navigation (y=148)
        ui_draw_text(4, 148, 0.5f, 0.36f, COLOR_TEXT_DIM, "[DPad] oben/unten   [A] Proficiency");
        ui_draw_text(4, 164, 0.5f, 0.36f, COLOR_TEXT_DIM, "[Y] Reset   [L/R] Tab");
        ui_button_draw(&s_btn_back);

    } else if (s_tab == TAB_INFO) {
        ui_button_draw(&s_btn_name_edit);
        ui_button_draw(&s_btn_align_edit);
        ui_button_draw(&s_btn_xp_edit);
        ui_draw_separator(0, 74, 320, COLOR_TEXT_DIM);
        ui_button_draw(&s_btn_traits);
        ui_button_draw(&s_btn_ideals);
        ui_button_draw(&s_btn_bonds);
        ui_button_draw(&s_btn_flaws);
        ui_button_draw(&s_btn_backstory);
        ui_draw_separator(0, 194, 320, COLOR_TEXT_DIM);
        ui_draw_text(4, 198, 0.5f, 0.33f, COLOR_TEXT_DIM, "[L/R] Tab   [B] Zurueck");

    } else { // TAB_ACTIONS
        // --- Sub-Tab-Buttons (y=28) ---
        s_btn_act_atk.color_bg  = (s_act_sub == ACT_ATTACKS)  ? COLOR_ACCENT : COLOR_BTN_NORMAL;
        s_btn_act_feat.color_bg = (s_act_sub == ACT_FEATURES) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
        ui_button_draw(&s_btn_act_atk);
        ui_button_draw(&s_btn_act_feat);

        int cnt = act_count();

        if (s_act_sub == ACT_ATTACKS) {
            // Filter-Buttons [Alle][Akt.][Bonus][Reak.] (y=54)
            for (int i = 0; i < 4; i++) {
                int flevel = (i == 0) ? -1 : (i - 1);
                s_btn_act_filt[i].color_bg = (flevel == s_act_filter) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
                ui_button_draw(&s_btn_act_filt[i]);
            }
        }

        // Liste (y=76-160, 6 Einträge à 14px)
        float list_y = (s_act_sub == ACT_ATTACKS) ? 76.0f : 54.0f;
        float list_h = 162.0f - list_y;
        int max_rows = (int)(list_h / 15.0f);

        ui_draw_panel(4, list_y, 312, list_h, COLOR_PANEL, COLOR_TEXT_DIM);
        for (int i = 0; i < max_rows; i++) {
            int row_idx = s_act_scroll + i;
            if (row_idx >= cnt) break;
            float iy = list_y + 2.0f + i * 15.0f;
            if (row_idx == s_act_cursor)
                ui_draw_rect(5, iy, 310, 14, COLOR_PANEL2);
            if (s_act_sub == ACT_ATTACKS) {
                Attack* a = &s_attacks[s_filt_attacks[row_idx]];
                u32 c  = (row_idx == s_act_cursor) ? COLOR_WHITE : COLOR_TEXT;
                u32 bc = (a->action_type == 0) ? COLOR_HP_GREEN :
                         (a->action_type == 1) ? COLOR_GOLD :
                         (a->action_type == 2) ? COLOR_ACCENT2 : COLOR_TEXT_DIM;
                ui_draw_textf( 8, iy+1, 0.6f, 0.34f, bc, "[%s]", ACT_TYPE_SHORT[a->action_type]);
                ui_draw_textf(34, iy+1, 0.6f, 0.36f, c,  "%-16s", a->name);
                ui_draw_textf(200, iy+1, 0.6f, 0.36f, COLOR_ACCENT2, "%s", a->hit_bonus);
                ui_draw_textf(240, iy+1, 0.6f, 0.34f, COLOR_TEXT_DIM, "%s", a->damage);
            } else {
                Feature* f = &s_features[row_idx];
                u32 c = (row_idx == s_act_cursor) ? COLOR_WHITE : COLOR_TEXT;
                ui_draw_textf( 8, iy+1, 0.6f, 0.36f, c, "%-22s", f->name);
                if (f->uses_max > 0) {
                    u32 uc = (f->uses_current > 0) ? COLOR_HP_GREEN : COLOR_HP_RED;
                    ui_draw_textf(246, iy+1, 0.6f, 0.34f, uc,
                                  "%d/%d", f->uses_current, f->uses_max);
                }
            }
        }
        if (cnt > max_rows)
            ui_draw_textf(270, list_y+2, 0.6f, 0.28f, COLOR_TEXT_DIM,
                          "%d/%d", s_act_cursor+1, cnt);

        ui_draw_separator(0, 164, 320, COLOR_TEXT_DIM);

        // CRUD-Buttons (y=168)
        ui_button_draw(&s_btn_act_add);
        ui_button_draw(&s_btn_act_edit);
        ui_button_draw(&s_btn_act_del);

        ui_draw_separator(0, 190, 320, COLOR_TEXT_DIM);
        if (s_act_sub == ACT_FEATURES)
            ui_draw_text(4, 194, 0.5f, 0.32f, COLOR_TEXT_DIM,
                         "[A] Verwend.-1  [Y] Reset  [DPad] Nav");
        else
            ui_draw_text(4, 194, 0.5f, 0.32f, COLOR_TEXT_DIM,
                         "[DPad] Navigieren");
    }
}

// ---- Screen-Objekt ---------------------------------------------------------
Screen g_screen_character = {
    .id          = SCREEN_CHARACTER,
    .on_enter    = on_enter,
    .on_exit     = on_exit,
    .update      = update,
    .draw_top    = draw_top,
    .draw_bottom = draw_bottom
};
