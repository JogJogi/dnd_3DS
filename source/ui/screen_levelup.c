// screen_levelup.c – Level-Up Multi-Step-Wizard (SCREEN_LEVELUP)
// Schritte: OVERVIEW -> SUBCLASS -> ASI_OR_FEAT -> EXPERTISE -> SPELLS
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
#include "../data/srd_subclasses.h"
#include "../data/srd_feats.h"
#include "../data/srd_spells_bard.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// Wizard-Schritte
// ============================================================================
typedef enum {
    LU_STEP_OVERVIEW = 0,
    LU_STEP_SUBCLASS,
    LU_STEP_ASI_OR_FEAT,
    LU_STEP_EXPERTISE,
    LU_STEP_SPELLS,
    LU_STEP_COUNT
} LuStep;

// ============================================================================
// Zustand
// ============================================================================
static Character          s_char;
static int                s_loaded        = 0;
static int                s_new_level     = 2;
static char               s_class_id[CHAR_NAME_MAX];
static const ClassProg*   s_prog          = NULL;
static const ProgFeat*    s_new_feats     = NULL;
static int                s_new_feat_cnt  = 0;
static int                s_new_slots[PROG_SLOT_LEVELS];
static SpellSlots         s_spell_slots;
static int                s_has_slots     = 0;

static LuStep             s_step          = LU_STEP_OVERVIEW;
static int                s_step_active[LU_STEP_COUNT];

// ---- OVERVIEW ---------------------------------------------------------------
static int                s_hp_choice     = 0;   // 0=Durchschnitt, 1=Maximum
static int                s_hp_gain       = 0;
static int                s_feat_scroll   = 0;

// ---- SUBCLASS ---------------------------------------------------------------
static const SrdSubclass* s_subclasses[8];
static int                s_subclass_count = 0;
static int                s_subclass_sel   = -1;
static int                s_subclass_scroll = 0;

// ---- ASI / FEAT -------------------------------------------------------------
static int                s_asi_mode      = 0;   // 0=ASI, 1=Feat
static int                s_asi_type      = 0;   // 0=+2 ein Attr., 1=+1+1 zwei
static int                s_asi_sel1      = -1;
static int                s_asi_sel2      = -1;
static int                s_feat_sel      = -1;
static int                s_feat_list_scroll = 0;
static char               s_asi_lbl[ABILITY_COUNT][16];

// ---- EXPERTISE --------------------------------------------------------------
static int                s_exp_count     = 0;
static int                s_exp_chosen[SKILL_COUNT];
static int                s_exp_chosen_cnt = 0;
static int                s_exp_scroll    = 0;

// ---- SPELLS -----------------------------------------------------------------
static int                s_spells_to_pick    = 0;
static int                s_cantrips_to_pick  = 0;
static int                s_spell_mode        = 0; // 0=Cantrips, 1=Zauber
static int                s_spell_chosen[32];
static int                s_spell_chosen_cnt  = 0;
static int                s_cantrip_chosen[16];
static int                s_cantrip_chosen_cnt = 0;
static int                s_spell_scroll      = 0;
static int                s_max_spell_level   = 1;

// ---- Buttons ----------------------------------------------------------------
static UiButton           s_btn_back, s_btn_next;

// ============================================================================
// Hilfsfunktionen
// ============================================================================
static void find_class_id(void) {
    s_class_id[0] = '\0';
    for (int i = 0; i < SRD_CLASS_COUNT; i++) {
        if (strcmp(SRD_CLASSES[i].name, s_char.class_name) == 0) {
            snprintf(s_class_id, sizeof(s_class_id), "%s", SRD_CLASSES[i].id);
            return;
        }
    }
}

static void recalc_hp(void) {
    int hit_die = 8;
    for (int i = 0; i < SRD_CLASS_COUNT; i++) {
        if (strcmp(SRD_CLASSES[i].name, s_char.class_name) == 0) {
            hit_die = SRD_CLASSES[i].hit_die;
            break;
        }
    }
    int con_mod = dnd_modifier(s_char.ability[ABILITY_CON]);
    s_hp_gain = (s_hp_choice == 0)
        ? srd_prog_hp_average(hit_die, con_mod)
        : srd_prog_hp_max(hit_die, con_mod);
}

static int calc_max_spell_level(void) {
    for (int i = PROG_SLOT_LEVELS - 1; i >= 0; i--)
        if (s_new_slots[i] > 0) return i + 1;
    return 1;
}

static void compute_active_steps(void) {
    memset(s_step_active, 0, sizeof(s_step_active));
    s_step_active[LU_STEP_OVERVIEW] = 1;

    // Unterklasse: nur wenn noch keine gesetzt und es Optionen fuer dieses Level gibt
    const SrdSubclass* all_sc[8];
    int all_cnt = srd_subclass_get_for_class(s_class_id, all_sc, 8);
    s_subclass_count = 0;
    if (s_char.subclass[0] == '\0') {
        for (int i = 0; i < all_cnt; i++) {
            if (all_sc[i]->unlock_level == s_new_level)
                s_subclasses[s_subclass_count++] = all_sc[i];
        }
    }
    if (s_subclass_count > 0) s_step_active[LU_STEP_SUBCLASS] = 1;

    // ASI / Feat
    if (srd_prog_is_asi(s_class_id, s_new_level))
        s_step_active[LU_STEP_ASI_OR_FEAT] = 1;

    // Expertise
    s_exp_count = 0;
    if (strcmp(s_class_id, "bard") == 0 &&
        (s_new_level == 3 || s_new_level == 10))
        s_exp_count = 2;
    if (strcmp(s_class_id, "rogue") == 0 && s_new_level == 6)
        s_exp_count = 2;
    if (s_exp_count > 0) s_step_active[LU_STEP_EXPERTISE] = 1;

    // Zauber / Cantrips
    s_spells_to_pick   = 0;
    s_cantrips_to_pick = 0;
    const ClassProg* cp = srd_prog_get(s_class_id);
    if (cp && s_new_level >= 1 && s_new_level <= PROG_LEVELS) {
        int sk = cp->levels[s_new_level - 1].spells_known;
        if (sk > 0) s_spells_to_pick = sk;
    }
    if (strcmp(s_class_id, "bard") == 0 &&
        (s_new_level == 4 || s_new_level == 10))
        s_cantrips_to_pick = 1;
    if (s_spells_to_pick > 0 || s_cantrips_to_pick > 0)
        s_step_active[LU_STEP_SPELLS] = 1;
}

static LuStep next_active_step(LuStep from) {
    for (int s = from + 1; s < LU_STEP_COUNT; s++)
        if (s_step_active[s]) return (LuStep)s;
    return from;
}

static LuStep prev_active_step(LuStep from) {
    for (int s = from - 1; s >= 0; s--)
        if (s_step_active[s]) return (LuStep)s;
    return from;
}

static int is_last_step(void) {
    return next_active_step(s_step) == s_step;
}

static int step_complete(void) {
    switch (s_step) {
        case LU_STEP_OVERVIEW:    return 1;
        case LU_STEP_SUBCLASS:    return s_subclass_sel >= 0;
        case LU_STEP_ASI_OR_FEAT:
            if (s_asi_mode == 0) {
                if (s_asi_type == 0) return s_asi_sel1 >= 0;
                return s_asi_sel1 >= 0 && s_asi_sel2 >= 0 && s_asi_sel1 != s_asi_sel2;
            }
            return s_feat_sel >= 0;
        case LU_STEP_EXPERTISE:
            return s_exp_chosen_cnt == s_exp_count;
        case LU_STEP_SPELLS: {
            int cd = (s_cantrips_to_pick == 0 || s_cantrip_chosen_cnt == s_cantrips_to_pick);
            int sd = (s_spells_to_pick   == 0 || s_spell_chosen_cnt   == s_spells_to_pick);
            return cd && sd;
        }
        default: return 1;
    }
}

// Forward declaration
static void levelup_apply(void);

static void handle_advance(void) {
    if (!step_complete()) return;
    if (is_last_step()) levelup_apply();
    else                s_step = next_active_step(s_step);
}

static void handle_back(void) {
    LuStep prev = prev_active_step(s_step);
    if (prev == s_step) screen_pop();
    else                s_step = prev;
}

// ============================================================================
// Klassen-Ressourcen aktualisieren (Bardische Inspiration etc.)
// ============================================================================
static void update_bardic_inspiration(void) {
    Feature feats[FEATURES_MAX];
    int cnt = features_db_load(s_char.id, feats, FEATURES_MAX);

    int die = 6;
    if      (s_new_level >= 15) die = 12;
    else if (s_new_level >= 10) die = 10;
    else if (s_new_level >=  5) die = 8;

    const char* recharge = (s_new_level >= 5) ? "Kurze Rast" : "Langer Rast";
    int cha_mod = dnd_modifier(s_char.ability[ABILITY_CHA]);
    int uses    = cha_mod > 0 ? cha_mod : 1;

    for (int i = 0; i < cnt; i++) {
        if (strstr(feats[i].name, "Inspiration") != NULL) {
            snprintf(feats[i].name, sizeof(feats[i].name),
                     "Bardische Inspiration (W%d)", die);
            feats[i].uses_max     = uses;
            feats[i].uses_current = uses;
            snprintf(feats[i].recharge_on, sizeof(feats[i].recharge_on),
                     "%.15s", recharge);
            features_db_save(&feats[i]);
            return;
        }
    }
}

static void save_subclass_features(const SrdSubclass* sc) {
    const SubclassLevel* sl = srd_subclass_get_level(sc, s_new_level);
    if (!sl) return;
    char src[FEATURE_SRC_MAX];
    snprintf(src, sizeof(src), "%.20s", sc->name);
    for (int i = 0; i < sl->feature_count; i++) {
        Feature f;
        memset(&f, 0, sizeof(f));
        f.character_id = s_char.id;
        snprintf(f.name,        sizeof(f.name),        "%.63s", sl->features[i].name);
        snprintf(f.source,      sizeof(f.source),      "%.31s", src);
        snprintf(f.description, sizeof(f.description), "%.255s", sl->features[i].description);
        f.uses_max = f.uses_current = sl->features[i].uses_max;
        snprintf(f.recharge_on, sizeof(f.recharge_on), "%.15s", sl->features[i].recharge_on);
        features_db_save(&f);
    }
}

// ============================================================================
// apply_levelup
// ============================================================================
static void levelup_apply(void) {
    s_char.level      = s_new_level;
    s_char.hp_max    += s_hp_gain;
    s_char.hp_current += s_hp_gain;

    // ASI
    if (s_step_active[LU_STEP_ASI_OR_FEAT] && s_asi_mode == 0) {
        if (s_asi_type == 0 && s_asi_sel1 >= 0) {
            s_char.ability[s_asi_sel1] += 2;
            if (s_char.ability[s_asi_sel1] > 20) s_char.ability[s_asi_sel1] = 20;
        } else if (s_asi_type == 1) {
            if (s_asi_sel1 >= 0) { s_char.ability[s_asi_sel1]++; if (s_char.ability[s_asi_sel1] > 20) s_char.ability[s_asi_sel1] = 20; }
            if (s_asi_sel2 >= 0) { s_char.ability[s_asi_sel2]++; if (s_char.ability[s_asi_sel2] > 20) s_char.ability[s_asi_sel2] = 20; }
        }
    }

    // Unterklasse setzen
    if (s_step_active[LU_STEP_SUBCLASS] && s_subclass_sel >= 0)
        snprintf(s_char.subclass, sizeof(s_char.subclass), "%.63s",
                 s_subclasses[s_subclass_sel]->name);

    character_db_save(&s_char);

    // Standard-Features
    char src[FEATURE_SRC_MAX];
    snprintf(src, sizeof(src), "%.10s Stufe %d", s_char.class_name, s_new_level);
    for (int i = 0; i < s_new_feat_cnt; i++) {
        Feature f;
        memset(&f, 0, sizeof(f));
        f.character_id = s_char.id;
        snprintf(f.name,        sizeof(f.name),        "%.63s",  s_new_feats[i].name);
        snprintf(f.source,      sizeof(f.source),      "%.31s",  src);
        snprintf(f.description, sizeof(f.description), "%.255s", s_new_feats[i].description);
        f.uses_max = f.uses_current = s_new_feats[i].uses_max;
        snprintf(f.recharge_on, sizeof(f.recharge_on), "%.15s",  s_new_feats[i].recharge_on);
        features_db_save(&f);
    }

    // Unterklassen-Features
    if (s_step_active[LU_STEP_SUBCLASS] && s_subclass_sel >= 0)
        save_subclass_features(s_subclasses[s_subclass_sel]);

    // Feat
    if (s_step_active[LU_STEP_ASI_OR_FEAT] && s_asi_mode == 1 && s_feat_sel >= 0) {
        const SrdFeat* feat = &SRD_FEATS[s_feat_sel];
        Feature f;
        memset(&f, 0, sizeof(f));
        f.character_id = s_char.id;
        snprintf(f.name,        sizeof(f.name),        "%.63s",  feat->name);
        snprintf(f.source,      sizeof(f.source),      "Feat Stufe %d", s_new_level);
        snprintf(f.description, sizeof(f.description), "%.255s", feat->description);
        f.uses_max = 0;
        features_db_save(&f);
        if (feat->asi_ability >= 0 && feat->asi_ability < ABILITY_COUNT) {
            s_char.ability[feat->asi_ability]++;
            if (s_char.ability[feat->asi_ability] > 20)
                s_char.ability[feat->asi_ability] = 20;
            character_db_save(&s_char);
        }
    }

    // Expertise
    if (s_step_active[LU_STEP_EXPERTISE]) {
        for (int i = 0; i < SKILL_COUNT; i++)
            if (s_exp_chosen[i]) s_char.skill_proficient[i] = 2;
        character_db_save(&s_char);
    }

    // Neue Zauber/Cantrips (Bard)
    if (s_step_active[LU_STEP_SPELLS] && strcmp(s_class_id, "bard") == 0) {
        for (int i = 0; i < s_cantrip_chosen_cnt; i++) {
            const BardSpellEntry* e = bard_get_cantrip(s_cantrip_chosen[i]);
            if (!e) continue;
            Spell sp; memset(&sp, 0, sizeof(sp));
            sp.character_id = s_char.id;
            snprintf(sp.name,         sizeof(sp.name),         "%.127s", e->name);
            snprintf(sp.school,       sizeof(sp.school),       "%.63s",  e->school);
            snprintf(sp.casting_time, sizeof(sp.casting_time), "%.63s",  e->casting_time);
            snprintf(sp.range,        sizeof(sp.range),        "%.63s",  e->range);
            snprintf(sp.components,   sizeof(sp.components),   "%.63s",  e->components);
            snprintf(sp.duration,     sizeof(sp.duration),     "%.63s",  e->duration);
            snprintf(sp.description,  sizeof(sp.description),  "%.511s", e->description);
            sp.spell_level = 0; sp.prepared = 1;
            sp.concentration = e->concentration; sp.ritual = e->ritual;
            spells_db_save(&sp);
        }
        for (int i = 0; i < s_spell_chosen_cnt; i++) {
            const BardSpellEntry* e = bard_get_spell(s_spell_chosen[i]);
            if (!e) continue;
            Spell sp; memset(&sp, 0, sizeof(sp));
            sp.character_id = s_char.id;
            snprintf(sp.name,         sizeof(sp.name),         "%.127s", e->name);
            snprintf(sp.school,       sizeof(sp.school),       "%.63s",  e->school);
            snprintf(sp.casting_time, sizeof(sp.casting_time), "%.63s",  e->casting_time);
            snprintf(sp.range,        sizeof(sp.range),        "%.63s",  e->range);
            snprintf(sp.components,   sizeof(sp.components),   "%.63s",  e->components);
            snprintf(sp.duration,     sizeof(sp.duration),     "%.63s",  e->duration);
            snprintf(sp.description,  sizeof(sp.description),  "%.511s", e->description);
            sp.spell_level = e->level; sp.prepared = 1;
            sp.concentration = e->concentration; sp.ritual = e->ritual;
            spells_db_save(&sp);
        }
    }

    // Zauberplaetze
    int has_new = 0;
    for (int i = 0; i < PROG_SLOT_LEVELS; i++)
        if (s_new_slots[i] != 0) { has_new = 1; break; }
    if (has_new) {
        SpellSlots upd;
        if (s_has_slots) memcpy(&upd, &s_spell_slots, sizeof(upd));
        else { memset(&upd, 0, sizeof(upd)); upd.character_id = s_char.id; }
        for (int i = 0; i < PROG_SLOT_LEVELS; i++) {
            if (s_new_slots[i] != 0) upd.total[i] = s_new_slots[i];
        }
        spell_slots_db_save(&upd);
    }

    // Barden-Ressourcen aktualisieren
    if (strcmp(s_class_id, "bard") == 0)
        update_bardic_inspiration();

    screen_pop();
}

// ============================================================================
// on_enter / on_exit
// ============================================================================
static void on_enter(void) {
    s_loaded = (character_db_load(g_active_char_id, &s_char) == 0);
    if (!s_loaded) return;

    s_new_level = s_char.level + 1;
    if (s_new_level > 20) s_new_level = 20;

    find_class_id();
    s_prog         = srd_prog_get(s_class_id);
    s_new_feat_cnt = srd_prog_feats(s_class_id, s_new_level, &s_new_feats);
    srd_prog_spell_slots(s_class_id, s_new_level, s_new_slots);
    s_has_slots    = (spell_slots_db_load(g_active_char_id, &s_spell_slots) == 0);

    // Zustand zuruecksetzen
    s_step = LU_STEP_OVERVIEW;
    s_hp_choice = 0; s_feat_scroll = 0;
    s_subclass_sel = -1; s_subclass_scroll = 0;
    s_asi_mode = 0; s_asi_type = 0; s_asi_sel1 = -1; s_asi_sel2 = -1;
    s_feat_sel = -1; s_feat_list_scroll = 0;
    memset(s_exp_chosen, 0, sizeof(s_exp_chosen)); s_exp_chosen_cnt = 0; s_exp_scroll = 0;
    s_spell_mode = 0; s_spell_scroll = 0;
    memset(s_spell_chosen,   0, sizeof(s_spell_chosen));
    memset(s_cantrip_chosen, 0, sizeof(s_cantrip_chosen));
    s_spell_chosen_cnt = 0; s_cantrip_chosen_cnt = 0;

    recalc_hp();
    s_max_spell_level = calc_max_spell_level();
    compute_active_steps();
    s_spell_mode = (s_cantrips_to_pick > 0) ? 0 : 1;

    s_btn_back = (UiButton){   4, 206, 100, 26, "< Zurueck",      COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_next = (UiButton){ 116, 206, 200, 26, "Weiter >",        COLOR_ACCENT,     COLOR_TEXT, 0 };
}
static void on_exit(void) {}

// ============================================================================
// DRAW OVERVIEW
// ============================================================================
static void draw_top_overview(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD,
                  "Stufenaufstieg: %s  Stufe %d -> %d",
                  s_char.name, s_char.level, s_new_level);
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);
    int y = 32; float sz = 0.40f, szs = 0.36f;
    ui_draw_textf(10, y, 0.5f, sz, COLOR_HP_GREEN,
                  "HP: %d -> %d  (+%d %s)",
                  s_char.hp_max, s_char.hp_max + s_hp_gain, s_hp_gain,
                  s_hp_choice == 0 ? "(Durchschn.)" : "(Maximum)");
    y += 16;
    int old_pb = dnd_proficiency_bonus(s_char.level);
    int new_pb = dnd_proficiency_bonus(s_new_level);
    if (new_pb != old_pb) {
        ui_draw_textf(10, y, 0.5f, sz, COLOR_GOLD,
                      "Uebungsbonus: +%d -> +%d", old_pb, new_pb);
        y += 16;
    }
    int has_slots = 0;
    for (int i = 0; i < PROG_SLOT_LEVELS; i++)
        if (s_new_slots[i] != 0) { has_slots = 1; break; }
    if (has_slots) {
        ui_draw_text(10, y, 0.5f, szs, COLOR_ACCENT2, "Neue Zauberplaetze:");
        y += 12;
        for (int i = 0; i < 5 && y < 200; i++) {
            if (s_new_slots[i] == 0) continue;
            int old_s = s_has_slots ? s_spell_slots.total[i] : 0;
            if (s_new_slots[i] != old_s)
                ui_draw_textf(10 + (i % 3)*130, y+(i/3)*12, 0.5f, szs,
                              COLOR_TEXT, "Grad %d: %d->%d", i+1, old_s, s_new_slots[i]);
        }
        y += 24;
    }
    ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM); y += 4;
    if (s_new_feat_cnt > 0) {
        ui_draw_text(10, y, 0.5f, szs, COLOR_TEXT, "Neue Faehigkeiten:"); y += 12;
        for (int i = s_feat_scroll; i < s_new_feat_cnt && y < 228; i++) {
            const ProgFeat* f = &s_new_feats[i];
            ui_draw_textf(10, y, 0.5f, sz, COLOR_GOLD, "%.40s", f->name); y += 13;
            const char* d = f->description; int dlen = (int)strlen(d);
            if (dlen <= 72) { ui_draw_textf(14, y, 0.5f, szs, COLOR_TEXT_DIM, "%.72s", d); y += 11; }
            else {
                char l1[80]; int cut = 71;
                while (cut > 0 && d[cut] != ' ') cut--;
                if (!cut) cut = 71;
                snprintf(l1, sizeof(l1), "%.*s", cut, d);
                ui_draw_text(14, y, 0.5f, szs, COLOR_TEXT_DIM, l1); y += 10;
                ui_draw_textf(14, y, 0.5f, szs, COLOR_TEXT_DIM, "%.72s", d+cut+1); y += 11;
            }
            if (f->uses_max > 0) {
                ui_draw_textf(14, y, 0.5f, szs, COLOR_ACCENT2, "%dx / %s",
                              f->uses_max, f->recharge_on[0] ? f->recharge_on : "unbegrenzt");
                y += 10;
            }
            ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM); y += 3;
        }
    } else if (!has_slots) {
        ui_draw_text(10, y, 0.5f, szs, COLOR_TEXT_DIM, "(Nur HP und Uebungsbonus)");
    }
}

static void draw_bottom_overview(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_GOLD,
                  "Stufe %d -> %d  (%s)", s_char.level, s_new_level, s_char.class_name);
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);
    ui_draw_text(8, 32, 0.5f, 0.40f, COLOR_TEXT, "HP-Zuwachs:");
    ui_draw_textf(130, 32, 0.5f, 0.40f, COLOR_HP_GREEN, "+%d", s_hp_gain);
    UiButton ba = { 4, 56, 152, 26, "Durchschnitt", s_hp_choice==0?COLOR_ACCENT:COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    UiButton bm = { 164, 56, 152, 26, "Maximum",    s_hp_choice==1?COLOR_ACCENT:COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    ui_button_draw(&ba); ui_button_draw(&bm);
    ui_draw_separator(0, 88, SCREEN_BTM_W, COLOR_TEXT_DIM);
    int y = 94; float szs = 0.34f;
    ui_draw_text(8, y, 0.5f, szs, COLOR_TEXT_DIM, "Naechste Schritte:"); y += 12;
    if (s_step_active[LU_STEP_SUBCLASS])    { ui_draw_text(12, y, 0.5f, szs, COLOR_TEXT, "* Unterklasse waehlen");  y += 11; }
    if (s_step_active[LU_STEP_ASI_OR_FEAT]) { ui_draw_text(12, y, 0.5f, szs, COLOR_TEXT, "* ASI oder Feat");        y += 11; }
    if (s_step_active[LU_STEP_EXPERTISE])   { ui_draw_text(12, y, 0.5f, szs, COLOR_TEXT, "* Expertise waehlen");    y += 11; }
    if (s_step_active[LU_STEP_SPELLS])      { ui_draw_text(12, y, 0.5f, szs, COLOR_TEXT, "* Zauber lernen");        y += 11; }
    if (y == 94+12) ui_draw_text(12, y, 0.5f, szs, COLOR_TEXT_DIM, "(Keine weiteren Entscheidungen)");
    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);
    UiButton bc = { 4, 202, 100, 26, "Abbrechen", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    UiButton bn = { 116, 202, 200, 26,
        is_last_step() ? "Stufe erhoehen!" : "Weiter >",
        COLOR_ACCENT, COLOR_TEXT, 0 };
    ui_button_draw(&bc); ui_button_draw(&bn);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM, "[A] Weiter  [B] Abbrechen  [L/R] HP");
}

// ============================================================================
// DRAW SUBCLASS
// ============================================================================
static void draw_top_subclass(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_text(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Unterklasse waehlen");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);
    if (s_subclass_sel >= 0 && s_subclass_sel < s_subclass_count) {
        const SrdSubclass* sc = s_subclasses[s_subclass_sel];
        ui_draw_textf(10, 34, 0.5f, 0.46f, COLOR_GOLD, "%s", sc->name);
        ui_draw_separator(0, 52, SCREEN_TOP_W, COLOR_TEXT_DIM);
        const char* d = sc->description; int y = 56;
        while (*d && y < 200) {
            char line[80]; int len = 0, ls = -1;
            while (d[len] && len < 72) { if (d[len]==' ') ls=len; len++; }
            if (d[len] && ls>0) len=ls;
            snprintf(line, sizeof(line), "%.*s", len, d);
            ui_draw_text(10, y, 0.5f, 0.36f, COLOR_TEXT, line); y += 13;
            d += len; if (*d==' ') d++;
        }
        const SubclassLevel* sl = srd_subclass_get_level(sc, s_new_level);
        if (sl) {
            ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM); y += 4;
            ui_draw_text(10, y, 0.5f, 0.36f, COLOR_ACCENT2, "Features auf dieser Stufe:"); y += 12;
            for (int i = 0; i < sl->feature_count && y < 230; i++) {
                ui_draw_textf(14, y, 0.5f, 0.36f, COLOR_GOLD, "%.50s", sl->features[i].name);
                y += 11;
            }
        }
    } else {
        ui_draw_text(10, 40, 0.5f, 0.40f, COLOR_TEXT_DIM, "Waehle eine Unterklasse unten.");
    }
}

static void draw_bottom_subclass(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_text(8, 5, 0.5f, 0.45f, COLOR_GOLD, "Unterklasse");
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);
    int y = 30;
    for (int i = s_subclass_scroll; i < s_subclass_count && y < 196; i++) {
        int sel = (i == s_subclass_sel);
        ui_draw_rect(4, y, 312, 38, sel ? COLOR_ACCENT : COLOR_BTN_NORMAL);
        ui_draw_textf(10, y+4,  0.5f, 0.40f, sel?COLOR_BLACK:COLOR_GOLD,     "%.40s", s_subclasses[i]->name);
        ui_draw_textf(10, y+20, 0.5f, 0.33f, sel?COLOR_BLACK:COLOR_TEXT_DIM, "%.55s", s_subclasses[i]->description);
        y += 42;
    }
    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);
    s_btn_back.color_bg = COLOR_BTN_NORMAL;
    s_btn_next.color_bg = step_complete() ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    s_btn_next.label    = is_last_step() ? "Stufe erhoehen!" : "Weiter >";
    ui_button_draw(&s_btn_back); ui_button_draw(&s_btn_next);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM, "[A/Tap] Waehlen  [B] Zurueck");
}

// ============================================================================
// DRAW ASI / FEAT
// ============================================================================
static void refresh_asi_labels(void) {
    for (int i = 0; i < ABILITY_COUNT; i++) {
        int b = 0;
        if (i == s_asi_sel1) b += (s_asi_type == 0) ? 2 : 1;
        if (i == s_asi_sel2) b += 1;
        snprintf(s_asi_lbl[i], sizeof(s_asi_lbl[i]), "%s %d", ABILITY_SHORT[i], s_char.ability[i]+b);
    }
}

static void draw_top_asi(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_text(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Attribut-Steigerung / Feat");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);
    int y = 34; float sz = 0.38f, szs = 0.34f;
    if (s_asi_mode == 0) {
        ui_draw_text(10, y, 0.5f, sz, COLOR_TEXT, "Aktuelle Attribute:"); y += 14;
        for (int i = 0; i < ABILITY_COUNT; i++) {
            int b = 0;
            if (i == s_asi_sel1) b += (s_asi_type == 0) ? 2 : 1;
            if (i == s_asi_sel2) b += 1;
            int x = 10 + (i%3)*130, yy = y + (i/3)*16;
            if (b > 0) ui_draw_textf(x, yy, 0.5f, sz, COLOR_GOLD,     "%s: %d->%d", ABILITY_SHORT[i], s_char.ability[i], s_char.ability[i]+b);
            else       ui_draw_textf(x, yy, 0.5f, sz, COLOR_TEXT_DIM, "%s: %d",     ABILITY_SHORT[i], s_char.ability[i]);
        }
    } else if (s_feat_sel >= 0) {
        const SrdFeat* feat = &SRD_FEATS[s_feat_sel];
        ui_draw_textf(10, y, 0.5f, 0.42f, COLOR_GOLD, "%s", feat->name); y += 16;
        if (feat->prerequisite) {
            ui_draw_textf(10, y, 0.5f, szs, COLOR_TEXT_DIM, "Voraus.: %s", feat->prerequisite); y += 12;
        }
        if (feat->asi_ability >= 0)  {
            ui_draw_textf(10, y, 0.5f, szs, COLOR_ACCENT2, "+1 auf %s", ABILITY_NAMES[feat->asi_ability]); y += 12;
        }
        ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM); y += 4;
        const char* d = feat->description;
        while (*d && y < 230) {
            char line[80]; int len=0, ls=-1;
            while (d[len] && len<68) { if(d[len]==' ') ls=len; len++; }
            if (d[len] && ls>0) len=ls;
            snprintf(line, sizeof(line), "%.*s", len, d);
            ui_draw_text(10, y, 0.5f, szs, COLOR_TEXT, line); y += 12;
            d += len; if (*d==' ') d++;
        }
    } else {
        ui_draw_text(10, y, 0.5f, sz, COLOR_TEXT_DIM, "Waehle unten eine Option.");
    }
}

static void draw_bottom_asi(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_text(8, 5, 0.5f, 0.45f, COLOR_GOLD, "ASI oder Feat");
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);
    UiButton tog = { 4, 30, 152, 22, s_asi_mode==0?"[ASI] | Feat":"ASI | [Feat]", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    ui_button_draw(&tog);
    if (s_asi_mode == 0) {
        UiButton btype = { 160, 30, 156, 22,
            s_asi_type==0?"+2 auf ein Attr.":"+1+1 auf zwei", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        ui_button_draw(&btype);
        refresh_asi_labels();
        for (int i = 0; i < ABILITY_COUNT; i++) {
            int col=i%3, row=i/2;
            int sel = (i==s_asi_sel1||i==s_asi_sel2);
            UiButton b = { 4.0f+col*106.0f, 58.0f+row*30.0f, 100.0f, 26.0f,
                s_asi_lbl[i], sel?COLOR_GOLD:COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
            ui_button_draw(&b);
        }
    } else {
        int y = 56; float sz = 0.36f;
        for (int i = s_feat_list_scroll; i < SRD_FEAT_COUNT && y < 196; i++) {
            int sel = (i == s_feat_sel);
            u32 fg = (SRD_FEATS[i].prerequisite && !sel) ? COLOR_TEXT_DIM : COLOR_TEXT;
            ui_draw_rect(4, y, 312, 20, sel?COLOR_ACCENT:COLOR_BTN_NORMAL);
            ui_draw_textf(8, y+3, 0.5f, sz, sel?COLOR_BLACK:fg, "%.38s", SRD_FEATS[i].name);
            if (SRD_FEATS[i].asi_ability >= 0)
                ui_draw_textf(262, y+3, 0.5f, sz, sel?COLOR_BLACK:COLOR_ACCENT2, "+1");
            y += 22;
        }
    }
    if (!step_complete())
        ui_draw_text(8, 186, 0.5f, 0.34f, COLOR_HP_RED,
                     s_asi_mode==0?"Bitte Attribut waehlen!":"Bitte Feat waehlen!");
    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);
    s_btn_back.color_bg = COLOR_BTN_NORMAL;
    s_btn_next.color_bg = step_complete() ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    s_btn_next.label    = is_last_step() ? "Stufe erhoehen!" : "Weiter >";
    ui_button_draw(&s_btn_back); ui_button_draw(&s_btn_next);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM, "[A] OK  [B] Zurueck  [DPad] Scrollen");
}

// ============================================================================
// DRAW EXPERTISE
// ============================================================================
static void draw_top_expertise(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_text(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Expertise waehlen");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);
    int y = 34; float sz = 0.40f, szs = 0.36f;
    ui_draw_textf(10, y, 0.5f, sz, COLOR_TEXT, "Waehle %d Fertigkeiten fuer Expertise:", s_exp_count); y += 16;
    ui_draw_textf(10, y, 0.5f, szs, COLOR_ACCENT2, "Ausgewaehlt: %d / %d", s_exp_chosen_cnt, s_exp_count); y += 14;
    ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM); y += 4;
    ui_draw_text(10, y, 0.5f, szs, COLOR_TEXT_DIM, "Expertise = doppelter Uebungsbonus"); y += 12;
    ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM); y += 4;
    ui_draw_text(10, y, 0.5f, szs, COLOR_TEXT, "Gewaehlt:"); y += 12;
    int found = 0;
    for (int i = 0; i < SKILL_COUNT; i++) {
        if (s_exp_chosen[i] && y < 230) {
            ui_draw_textf(14, y, 0.5f, szs, COLOR_GOLD, "* %s", SKILL_NAMES[i]); y += 11; found++;
        }
    }
    if (!found) ui_draw_text(14, y, 0.5f, szs, COLOR_TEXT_DIM, "(keine)");
}

static void draw_bottom_expertise(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_GOLD, "Expertise (%d/%d)", s_exp_chosen_cnt, s_exp_count);
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);
    int y = 30; float sz = 0.37f;
    for (int i = s_exp_scroll; i < SKILL_COUNT && y < 196; i++) {
        int prof   = s_char.skill_proficient[i];
        int expert = s_exp_chosen[i];
        if (prof < 1 || prof == 2) {
            ui_draw_textf(8, y, 0.5f, sz, COLOR_TEXT_DIM, "[ ] %s", SKILL_NAMES[i]);
        } else {
            ui_draw_textf(8, y, 0.5f, sz, expert?COLOR_GOLD:COLOR_TEXT,
                          "[%s] %s", expert?"E":" ", SKILL_NAMES[i]);
        }
        y += 18;
    }
    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);
    s_btn_back.color_bg = COLOR_BTN_NORMAL;
    s_btn_next.color_bg = step_complete() ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    s_btn_next.label    = is_last_step() ? "Stufe erhoehen!" : "Weiter >";
    ui_button_draw(&s_btn_back); ui_button_draw(&s_btn_next);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM, "[Tap] Waehlen  [B] Zurueck  [DPad] Scrollen");
}

// ============================================================================
// DRAW SPELLS
// ============================================================================
static int cantrip_already_chosen(int idx) {
    for (int i = 0; i < s_cantrip_chosen_cnt; i++) if (s_cantrip_chosen[i]==idx) return 1;
    return 0;
}
static int spell_already_chosen(int idx) {
    for (int i = 0; i < s_spell_chosen_cnt; i++) if (s_spell_chosen[i]==idx) return 1;
    return 0;
}

static void draw_top_spells(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    if (s_spell_mode == 0)
        ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Zaubertricks (%d/%d)", s_cantrip_chosen_cnt, s_cantrips_to_pick);
    else
        ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Neue Zauber (%d/%d)", s_spell_chosen_cnt, s_spells_to_pick);
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);
    const BardSpellEntry* e = NULL;
    if (s_spell_mode == 0) {
        e = bard_get_cantrip(s_spell_scroll);
    } else {
        int shown = 0;
        for (int i = 0; i < BARD_SPELL_COUNT; i++) {
            const BardSpellEntry* se = bard_get_spell(i);
            if (!se || se->level < 1 || se->level > s_max_spell_level) continue;
            if (shown == s_spell_scroll) { e = se; break; }
            shown++;
        }
    }
    if (e) {
        int y = 34; float sz = 0.40f, szs = 0.34f;
        ui_draw_textf(10, y, 0.5f, sz, COLOR_GOLD, "%s", e->name); y += 14;
        ui_draw_textf(10, y, 0.5f, szs, COLOR_TEXT_DIM, "%s | %s",
                      e->school, e->level==0 ? "Zaubertrick" : "Grad");
        if (e->level > 0) ui_draw_textf(200, y, 0.5f, szs, COLOR_TEXT_DIM, "%d", e->level);
        y += 12;
        ui_draw_textf(10, y, 0.5f, szs, COLOR_TEXT, "Zeit: %s  Rw: %s", e->casting_time, e->range); y += 12;
        ui_draw_textf(10, y, 0.5f, szs, COLOR_TEXT, "Dauer: %s", e->duration);
        if (e->concentration) ui_draw_text(240, y, 0.5f, szs, COLOR_HP_YELLOW, "[K]");
        if (e->ritual)        ui_draw_text(264, y, 0.5f, szs, COLOR_ACCENT2,   "[R]");
        y += 12;
        ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM); y += 4;
        const char* d = e->description;
        while (*d && y < 230) {
            char line[80]; int len=0, ls=-1;
            while (d[len] && len<72) { if(d[len]==' ') ls=len; len++; }
            if (d[len] && ls>0) len=ls;
            snprintf(line, sizeof(line), "%.*s", len, d);
            ui_draw_text(10, y, 0.5f, szs, COLOR_TEXT, line); y += 12;
            d += len; if (*d==' ') d++;
        }
    } else {
        ui_draw_text(10, 40, 0.5f, 0.40f, COLOR_TEXT_DIM, "Kein Eintrag.");
    }
}

static void draw_bottom_spells(void) {
    if (s_cantrips_to_pick > 0 && s_spells_to_pick > 0) {
        UiButton bt1 = { 4,   0, 155, 26, "Zaubertricks",   s_spell_mode==0?COLOR_ACCENT:COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        UiButton bt2 = { 161, 0, 155, 26, "Bekannte Zauber", s_spell_mode==1?COLOR_ACCENT:COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
        ui_button_draw(&bt1); ui_button_draw(&bt2);
    } else {
        ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
        ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_GOLD, "%s",
                      s_spell_mode==0 ? "Zaubertricks waehlen" : "Bekannte Zauber");
    }
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);
    int y = 30; float sz = 0.37f;
    if (s_spell_mode == 0) {
        for (int i = s_spell_scroll; i < BARD_CANTRIP_COUNT && y < 196; i++) {
            const BardSpellEntry* e = bard_get_cantrip(i);
            if (!e) continue;
            int chosen = cantrip_already_chosen(i);
            ui_draw_rect(4, y, 312, 20, chosen?COLOR_ACCENT:COLOR_BTN_NORMAL);
            ui_draw_textf(8, y+3, 0.5f, sz, chosen?COLOR_BLACK:COLOR_TEXT,
                          "[%s] %s", chosen?"X":" ", e->name);
            y += 22;
        }
    } else {
        int shown = 0;
        for (int i = 0; i < BARD_SPELL_COUNT && y < 196; i++) {
            const BardSpellEntry* e = bard_get_spell(i);
            if (!e || e->level < 1 || e->level > s_max_spell_level) continue;
            if (shown < s_spell_scroll) { shown++; continue; }
            int chosen = spell_already_chosen(i);
            ui_draw_rect(4, y, 312, 20, chosen?COLOR_ACCENT:COLOR_BTN_NORMAL);
            ui_draw_textf(8, y+3, 0.5f, sz, chosen?COLOR_BLACK:COLOR_TEXT,
                          "[%s] G%d %s", chosen?"X":" ", e->level, e->name);
            y += 22; shown++;
        }
    }
    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);
    s_btn_back.color_bg = COLOR_BTN_NORMAL;
    s_btn_next.color_bg = step_complete() ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    s_btn_next.label    = is_last_step() ? "Stufe erhoehen!" : "Weiter >";
    ui_button_draw(&s_btn_back); ui_button_draw(&s_btn_next);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM, "[Tap] Waehlen  [B] Zurueck  [DPad] Scrollen");
}

// ============================================================================
// DRAW Dispatcher
// ============================================================================
static void draw_top(void) {
    if (!s_loaded) return;
    switch (s_step) {
        case LU_STEP_OVERVIEW:    draw_top_overview();   break;
        case LU_STEP_SUBCLASS:    draw_top_subclass();   break;
        case LU_STEP_ASI_OR_FEAT: draw_top_asi();        break;
        case LU_STEP_EXPERTISE:   draw_top_expertise();  break;
        case LU_STEP_SPELLS:      draw_top_spells();     break;
        default: break;
    }
}

static void draw_bottom(void) {
    if (!s_loaded) {
        ui_draw_text(10, 10, 0.5f, 0.45f, COLOR_HP_RED, "Charakter konnte nicht geladen werden!");
        return;
    }
    switch (s_step) {
        case LU_STEP_OVERVIEW:    draw_bottom_overview();  break;
        case LU_STEP_SUBCLASS:    draw_bottom_subclass();  break;
        case LU_STEP_ASI_OR_FEAT: draw_bottom_asi();       break;
        case LU_STEP_EXPERTISE:   draw_bottom_expertise(); break;
        case LU_STEP_SPELLS:      draw_bottom_spells();    break;
        default: break;
    }
}

// ============================================================================
// UPDATE
// ============================================================================
static void update(u32 keys_down, u32 keys_held,
                   touchPosition* touch, int touch_down) {
    (void)keys_held;
    if (!s_loaded) { screen_pop(); return; }
    if (keys_down & KEY_B) { handle_back(); return; }

    switch (s_step) {

    case LU_STEP_OVERVIEW:
        if (keys_down & KEY_L) { s_hp_choice = 0; recalc_hp(); }
        if (keys_down & KEY_R) { s_hp_choice = 1; recalc_hp(); }
        if (keys_down & KEY_DOWN && s_feat_scroll < s_new_feat_cnt-1) s_feat_scroll++;
        if (keys_down & KEY_UP   && s_feat_scroll > 0)               s_feat_scroll--;
        if (keys_down & KEY_A)   handle_advance();
        if (touch_down) {
            if (touch->py>=56&&touch->py<=82) {
                if (touch->px>=4&&touch->px<=156)   { s_hp_choice=0; recalc_hp(); }
                if (touch->px>=164&&touch->px<=316) { s_hp_choice=1; recalc_hp(); }
            }
            if (touch->py>=202&&touch->py<=228) {
                if (touch->px<=104)  { screen_pop(); return; }
                if (touch->px>=116)  handle_advance();
            }
        }
        break;

    case LU_STEP_SUBCLASS:
        if (keys_down & KEY_DOWN && s_subclass_scroll < s_subclass_count-1) s_subclass_scroll++;
        if (keys_down & KEY_UP   && s_subclass_scroll > 0)                  s_subclass_scroll--;
        if (keys_down & KEY_A && step_complete()) handle_advance();
        if (touch_down) {
            if (touch->py>=30&&touch->py<196) {
                int row = (touch->py-30)/42 + s_subclass_scroll;
                if (row>=0&&row<s_subclass_count) {
                    if (row==s_subclass_sel) handle_advance();
                    else s_subclass_sel = row;
                }
            }
            if (touch->py>=202&&touch->py<=228) {
                if (touch->px<=104) handle_back();
                else if (touch->px>=116) handle_advance();
            }
        }
        break;

    case LU_STEP_ASI_OR_FEAT:
        if (keys_down & KEY_DOWN && s_asi_mode==1 && s_feat_list_scroll<SRD_FEAT_COUNT-1) s_feat_list_scroll++;
        if (keys_down & KEY_UP   && s_asi_mode==1 && s_feat_list_scroll>0)                s_feat_list_scroll--;
        if (keys_down & KEY_A && step_complete()) handle_advance();
        if (touch_down) {
            // Toggle ASI/Feat
            if (touch->py>=30&&touch->py<=52&&touch->px>=4&&touch->px<=156) {
                s_asi_mode=1-s_asi_mode; s_asi_sel1=s_asi_sel2=-1; s_feat_sel=-1; break;
            }
            if (s_asi_mode == 0) {
                // Toggle +2/+1+1
                if (touch->py>=30&&touch->py<=52&&touch->px>=160) {
                    s_asi_type=1-s_asi_type; s_asi_sel1=s_asi_sel2=-1; break;
                }
                // Attribut-Buttons: 2x3 grid bei y=58, h=26, x=4+col*106, w=100
                for (int i = 0; i < ABILITY_COUNT; i++) {
                    int col=i%3, row=i/3;
                    int bx=4+col*106, by=58+row*30;
                    if (touch->px>=bx&&touch->px<=bx+100&&touch->py>=by&&touch->py<=by+26) {
                        if (s_asi_type==0) { s_asi_sel1=i; s_asi_sel2=-1; }
                        else {
                            if (s_asi_sel1<0) s_asi_sel1=i;
                            else if (s_asi_sel2<0&&i!=s_asi_sel1) s_asi_sel2=i;
                            else { s_asi_sel1=i; s_asi_sel2=-1; }
                        }
                        break;
                    }
                }
            } else {
                // Feat-Liste
                if (touch->py>=56&&touch->py<196) {
                    int row=(touch->py-56)/22+s_feat_list_scroll;
                    if (row>=0&&row<SRD_FEAT_COUNT) s_feat_sel=row;
                }
            }
            if (touch->py>=202&&touch->py<=228) {
                if (touch->px<=104) handle_back();
                else if (touch->px>=116) handle_advance();
            }
        }
        break;

    case LU_STEP_EXPERTISE:
        if (keys_down & KEY_DOWN && s_exp_scroll<SKILL_COUNT-1) s_exp_scroll++;
        if (keys_down & KEY_UP   && s_exp_scroll>0)             s_exp_scroll--;
        if (keys_down & KEY_A && step_complete()) handle_advance();
        if (touch_down) {
            if (touch->py>=30&&touch->py<196) {
                int idx=(touch->py-30)/18+s_exp_scroll;
                if (idx>=0&&idx<SKILL_COUNT&&s_char.skill_proficient[idx]==1) {
                    if (s_exp_chosen[idx]) { s_exp_chosen[idx]=0; s_exp_chosen_cnt--; }
                    else if (s_exp_chosen_cnt<s_exp_count) { s_exp_chosen[idx]=1; s_exp_chosen_cnt++; }
                }
            }
            if (touch->py>=202&&touch->py<=228) {
                if (touch->px<=104) handle_back();
                else if (touch->px>=116) handle_advance();
            }
        }
        break;

    case LU_STEP_SPELLS:
        if (keys_down & KEY_DOWN) { int mx=(s_spell_mode==0)?BARD_CANTRIP_COUNT:BARD_SPELL_COUNT; if(s_spell_scroll<mx-1) s_spell_scroll++; }
        if (keys_down & KEY_UP   && s_spell_scroll>0) s_spell_scroll--;
        if ((keys_down & KEY_L) && s_cantrips_to_pick>0) { s_spell_mode=0; s_spell_scroll=0; }
        if ((keys_down & KEY_R) && s_spells_to_pick>0)   { s_spell_mode=1; s_spell_scroll=0; }
        if (keys_down & KEY_A && step_complete()) handle_advance();
        if (touch_down) {
            // Tabs
            if (s_cantrips_to_pick>0&&s_spells_to_pick>0&&touch->py<=26) {
                if (touch->px<=159) { s_spell_mode=0; s_spell_scroll=0; }
                else                { s_spell_mode=1; s_spell_scroll=0; }
                break;
            }
            if (touch->py>=30&&touch->py<196) {
                int row=(touch->py-30)/22;
                if (s_spell_mode==0) {
                    int idx=row+s_spell_scroll;
                    if (idx>=0&&idx<BARD_CANTRIP_COUNT) {
                        if (cantrip_already_chosen(idx)) {
                            for (int i=0;i<s_cantrip_chosen_cnt;i++) if(s_cantrip_chosen[i]==idx){ s_cantrip_chosen[i]=s_cantrip_chosen[--s_cantrip_chosen_cnt]; break; }
                        } else if (s_cantrip_chosen_cnt<s_cantrips_to_pick) {
                            s_cantrip_chosen[s_cantrip_chosen_cnt++]=idx;
                        }
                    }
                } else {
                    int shown=0, real=-1;
                    for (int i=0;i<BARD_SPELL_COUNT;i++) {
                        const BardSpellEntry* e=bard_get_spell(i);
                        if (!e||e->level<1||e->level>s_max_spell_level) continue;
                        if (shown==row+s_spell_scroll) { real=i; break; }
                        shown++;
                    }
                    if (real>=0) {
                        if (spell_already_chosen(real)) {
                            for (int i=0;i<s_spell_chosen_cnt;i++) if(s_spell_chosen[i]==real){ s_spell_chosen[i]=s_spell_chosen[--s_spell_chosen_cnt]; break; }
                        } else if (s_spell_chosen_cnt<s_spells_to_pick) {
                            s_spell_chosen[s_spell_chosen_cnt++]=real;
                        }
                    }
                }
            }
            if (touch->py>=202&&touch->py<=228) {
                if (touch->px<=104) handle_back();
                else if (touch->px>=116) handle_advance();
            }
        }
        break;

    default: break;
    }
}

// ============================================================================
// Screen-Objekt
// ============================================================================
Screen g_screen_levelup = {
    .id          = SCREEN_LEVELUP,
    .on_enter    = on_enter,
    .on_exit     = on_exit,
    .update      = update,
    .draw_top    = draw_top,
    .draw_bottom = draw_bottom,
};
