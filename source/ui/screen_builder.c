#include "screen_builder.h"
#include "ui_core.h"
#include "../data/srd_races.h"
#include "../data/srd_classes.h"
#include "../data/srd_backgrounds.h"
#include "../models/character.h"
#include "../utils/dnd_rules.h"
#include "../db/character_db.h"
#include "../db/inventory_db.h"
#include "../db/spell_db.h"
#include "../db/feature_db.h"
#include <string.h>
#include <stdio.h>

// ---- Globaler Builder-Zustand ----------------------------------------------
BuilderState g_builder;

// ---- Forward-Deklarationen der Sub-Schritt-Handler -------------------------
// Jede screen_builder_XYZ.c Datei exportiert diese Funktionen:
extern void step_race_init(void);
extern void step_race_draw_top(void);
extern void step_race_draw_bottom(void);
extern void step_race_update(u32 keys_down, u32 keys_held,
                             touchPosition* touch, int touch_down);

extern void step_class_init(void);
extern void step_class_draw_top(void);
extern void step_class_draw_bottom(void);
extern void step_class_update(u32 keys_down, u32 keys_held,
                              touchPosition* touch, int touch_down);

extern void step_skills_init(void);
extern void step_skills_draw_top(void);
extern void step_skills_draw_bottom(void);
extern void step_skills_update(u32 keys_down, u32 keys_held,
                               touchPosition* touch, int touch_down);

extern void step_abilities_init(void);
extern void step_abilities_draw_top(void);
extern void step_abilities_draw_bottom(void);
extern void step_abilities_update(u32 keys_down, u32 keys_held,
                                  touchPosition* touch, int touch_down);

extern void step_background_init(void);
extern void step_background_draw_top(void);
extern void step_background_draw_bottom(void);
extern void step_background_update(u32 keys_down, u32 keys_held,
                                   touchPosition* touch, int touch_down);

extern void step_details_init(void);
extern void step_details_draw_top(void);
extern void step_details_draw_bottom(void);
extern void step_details_update(u32 keys_down, u32 keys_held,
                                touchPosition* touch, int touch_down);

extern void step_review_init(void);
extern void step_review_draw_top(void);
extern void step_review_draw_bottom(void);
extern void step_review_update(u32 keys_down, u32 keys_held,
                               touchPosition* touch, int touch_down);

// ---- Dispatch-Tabelle ------------------------------------------------------
typedef struct {
    void (*init)(void);
    void (*draw_top)(void);
    void (*draw_bottom)(void);
    void (*update)(u32, u32, touchPosition*, int);
} StepHandler;

static const StepHandler STEP_HANDLERS[BUILDER_STEP_COUNT] = {
    { step_race_init,       step_race_draw_top,       step_race_draw_bottom,       step_race_update       },
    { step_class_init,      step_class_draw_top,      step_class_draw_bottom,      step_class_update      },
    { step_skills_init,     step_skills_draw_top,     step_skills_draw_bottom,     step_skills_update     },
    { step_abilities_init,  step_abilities_draw_top,  step_abilities_draw_bottom,  step_abilities_update  },
    { step_background_init, step_background_draw_top, step_background_draw_bottom, step_background_update },
    { step_details_init,    step_details_draw_top,    step_details_draw_bottom,    step_details_update    },
    { step_review_init,     step_review_draw_top,     step_review_draw_bottom,     step_review_update     },
};

// ---- Standard-Array fuer Attribut-Zuweisung --------------------------------
static const int STD_ARRAY[ABILITY_COUNT] = { 15, 14, 13, 12, 10, 8 };

// ---- Schritt-Initialisierung -----------------------------------------------
static void init_step(int step) {
    if (step >= 0 && step < BUILDER_STEP_COUNT && STEP_HANDLERS[step].init)
        STEP_HANDLERS[step].init();
}

// ---- Screen-Callbacks ------------------------------------------------------

static void on_enter(void) {
    // Builder-Zustand zuruecksetzen
    memset(&g_builder, 0, sizeof(g_builder));
    g_builder.race_idx    = -1;
    g_builder.subrace_idx = -1;
    g_builder.class_idx   = -1;
    g_builder.bg_idx      = -1;
    g_builder.step        = BUILDER_STEP_RACE;
    g_builder.dirty       = 1;

    // Standard-Array als Default-Zuweisung setzen
    g_builder.ability_method = ABILITY_METHOD_STANDARD;
    for (int i = 0; i < ABILITY_COUNT; i++)
        g_builder.abilities[i] = STD_ARRAY[i];

    // Alle Klassen-Skills abwaehlen
    for (int i = 0; i < SKILL_COUNT; i++)
        g_builder.chosen_skills[i] = 0;

    init_step(BUILDER_STEP_RACE);
    g_builder.dirty = 0;
}

static void on_exit(void) {
    // nichts zu bereinigen
}

static void update(u32 keys_down, u32 keys_held,
                   touchPosition* touch, int touch_down) {
    // Dirty-Flag: Schritt wurde gerade gewechselt
    if (g_builder.dirty) {
        init_step(g_builder.step);
        g_builder.dirty = 0;
    }

    int step = g_builder.step;
    if (step >= 0 && step < BUILDER_STEP_COUNT && STEP_HANDLERS[step].update)
        STEP_HANDLERS[step].update(keys_down, keys_held, touch, touch_down);
}

static void draw_top(void) {
    int step = g_builder.step;
    if (step >= 0 && step < BUILDER_STEP_COUNT && STEP_HANDLERS[step].draw_top)
        STEP_HANDLERS[step].draw_top();
}

static void draw_bottom(void) {
    int step = g_builder.step;
    if (step >= 0 && step < BUILDER_STEP_COUNT && STEP_HANDLERS[step].draw_bottom)
        STEP_HANDLERS[step].draw_bottom();
}

// ---- Screen-Objekt ---------------------------------------------------------
Screen g_screen_builder = {
    .id          = SCREEN_BUILDER,
    .on_enter    = on_enter,
    .on_exit     = on_exit,
    .update      = update,
    .draw_top    = draw_top,
    .draw_bottom = draw_bottom,
};

// ---- Schritt-Navigation (public) -------------------------------------------

void builder_next_step(void) {
    int next = g_builder.step + 1;
    if (next >= BUILDER_STEP_COUNT) return;
    g_builder.step  = next;
    g_builder.dirty = 1;
}

void builder_prev_step(void) {
    int prev = g_builder.step - 1;
    if (prev < 0) {
        // Zurueck zum Hauptmenue
        screen_pop();
        return;
    }
    g_builder.step  = prev;
    g_builder.dirty = 1;
}

// ---- Berechnungshelfer -----------------------------------------------------

int builder_final_ability(int i) {
    if (i < 0 || i >= ABILITY_COUNT) return 10;
    int base = g_builder.abilities[i];
    int bonus = g_builder.bg_asi[i];   // Hintergrunds-ASI (D&D 2024: +2/+1)
    if (g_builder.race_idx >= 0) {
        const SrdRace* r = &SRD_RACES[g_builder.race_idx];
        bonus += r->asi[i];
        if (g_builder.subrace_idx >= 0 && g_builder.subrace_idx < r->subrace_count)
            bonus += r->subraces[g_builder.subrace_idx].asi[i];
    }
    return base + bonus;
}

int builder_initial_hp(void) {
    int hit_die = 8;
    if (g_builder.class_idx >= 0)
        hit_die = SRD_CLASSES[g_builder.class_idx].hit_die;
    int con_mod = dnd_modifier(builder_final_ability(ABILITY_CON));
    int hp = hit_die + con_mod;
    return hp < 1 ? 1 : hp;
}

int builder_initial_ac(void) {
    // Ungeruestete AC: 10 + DEX-Mod
    // Barbar hat eigene AC-Formel, aber fuer Stufe-1-Start reicht das
    return 10 + dnd_modifier(builder_final_ability(ABILITY_DEX));
}

int builder_visible_step(void) {
    // SKILLS ist Sub-Schritt von CLASS → anzeigen als Schritt 2
    if (g_builder.step == BUILDER_STEP_SKILLS)
        return 2;
    if (g_builder.step > BUILDER_STEP_SKILLS)
        return g_builder.step; // SKILLS nicht zaehlen → Steps 3-6 = intern 3-6
    return g_builder.step + 1;
}

int builder_step_complete(int step) {
    switch (step) {
        case BUILDER_STEP_RACE:
            return g_builder.race_idx >= 0;
        case BUILDER_STEP_CLASS:
            return g_builder.class_idx >= 0;
        case BUILDER_STEP_SKILLS: {
            if (g_builder.class_idx < 0) return 0;
            const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];
            int count = 0;
            for (int i = 0; i < SKILL_COUNT; i++)
                count += g_builder.chosen_skills[i];
            return count >= cls->num_skills;
        }
        case BUILDER_STEP_ABILITIES:
            return 1; // immer vollstaendig (Standardwerte sind gesetzt)
        case BUILDER_STEP_BACKGROUND:
            return g_builder.bg_idx >= 0;
        case BUILDER_STEP_DETAILS:
            return g_builder.name[0] != '\0';
        case BUILDER_STEP_REVIEW:
            return 1;
        default:
            return 0;
    }
}

// ---- Waffen-Lookup fuer Angriffsgenerierung --------------------------------
typedef struct {
    const char* item_name;
    const char* dmg_dice;
    const char* dmg_type;
    int         is_finesse;
    int         is_ranged;
    const char* notes;
} WpnInfo;

static const WpnInfo WPN_TABLE[] = {
    { "Rapier",            "1W6",  "Stich",       1, 0, "Finesse" },
    { "Langschwert",       "1W8",  "Hieb",        0, 0, "Vielseitig: 1W10" },
    { "Streitkolben",      "1W6",  "Wucht",       0, 0, "" },
    { "Dolch",             "1W4",  "Stich",       1, 0, "Finesse, Wurfwaffe 20/60" },
    { "Stab",              "1W6",  "Wucht",       0, 0, "Vielseitig: 1W8" },
    { "Kurzschwert",       "1W6",  "Stich",       1, 0, "Finesse" },
    { "Handaxt",           "1W6",  "Hieb",        0, 0, "Wurfwaffe 20/60" },
    { "Grossaxt",          "1W12", "Hieb",        0, 0, "Zweihaendig" },
    { "Kriegshammer",      "1W8",  "Wucht",       0, 0, "Vielseitig: 1W10" },
    { "Langbogen",         "1W8",  "Durchbohren", 0, 1, "Zweihaendig, 150/600 Fuss" },
    { "Leichte Armbruest", "1W8",  "Durchbohren", 0, 1, "80/320 Fuss, Nachladen" },
    { "Wurfmesser",        "1W4",  "Stich",       1, 0, "Finesse, Wurfwaffe 20/60" },
};
#define WPN_TABLE_COUNT ((int)(sizeof(WPN_TABLE) / sizeof(WPN_TABLE[0])))

static const WpnInfo* wpn_lookup(const char* name) {
    for (int i = 0; i < WPN_TABLE_COUNT; i++)
        if (strcmp(WPN_TABLE[i].item_name, name) == 0)
            return &WPN_TABLE[i];
    return NULL;
}

// ---- Angriffe und Features nach Charaktererstellung generieren -------------
static void builder_create_attacks_features(const Character* c) {
    int pb      = 2;
    int str_mod = dnd_modifier(c->ability[ABILITY_STR]);
    int dex_mod = dnd_modifier(c->ability[ABILITY_DEX]);
    int cha_mod = dnd_modifier(c->ability[ABILITY_CHA]);

    // --- Rassen-Merkmale als Features ---
    if (g_builder.race_idx >= 0) {
        const SrdRace* race = &SRD_RACES[g_builder.race_idx];

        for (int i = 0; i < race->trait_count; i++) {
            Feature f;
            memset(&f, 0, sizeof(f));
            f.character_id = c->id;
            snprintf(f.name,        sizeof(f.name),        "%.63s", race->traits[i].name);
            snprintf(f.source,      sizeof(f.source),      "%.31s", race->name);
            snprintf(f.description, sizeof(f.description), "%.255s", race->traits[i].description);
            if (strcmp(race->id, "dragonborn") == 0 &&
                strncmp(race->traits[i].name, "Atem", 4) == 0) {
                f.uses_max = f.uses_current = 1;
                snprintf(f.recharge_on, sizeof(f.recharge_on), "Kurze Rast");
            } else if (strcmp(race->id, "half_orc") == 0 &&
                       strncmp(race->traits[i].name, "Unerbittliche", 13) == 0) {
                f.uses_max = f.uses_current = 1;
                snprintf(f.recharge_on, sizeof(f.recharge_on), "Langer Rast");
            }
            features_db_save(&f);
        }

        if (g_builder.subrace_idx >= 0 &&
            g_builder.subrace_idx < race->subrace_count) {
            const Subrace* sub = &race->subraces[g_builder.subrace_idx];
            for (int i = 0; i < sub->trait_count; i++) {
                Feature f;
                memset(&f, 0, sizeof(f));
                f.character_id = c->id;
                snprintf(f.name,        sizeof(f.name),        "%.63s", sub->traits[i].name);
                snprintf(f.source,      sizeof(f.source),      "%.31s", sub->name);
                snprintf(f.description, sizeof(f.description), "%.255s", sub->traits[i].description);
                features_db_save(&f);
            }
        }

        // Aarakocra: Klauen als Angriff
        if (strcmp(race->id, "aarakocra") == 0) {
            Attack a;
            memset(&a, 0, sizeof(a));
            a.character_id = c->id;
            snprintf(a.name,        sizeof(a.name),        "Klauen");
            snprintf(a.hit_bonus,   sizeof(a.hit_bonus),   "%+d", pb + str_mod);
            snprintf(a.damage,      sizeof(a.damage),      "1W4%+d", str_mod);
            snprintf(a.damage_type, sizeof(a.damage_type), "Hieb");
            snprintf(a.notes,       sizeof(a.notes),       "Natuerliche Waffe");
            a.action_type = ATTACK_ACTION;
            attacks_db_save(&a);
        }
    }

    // --- Klassen-Features ---
    if (g_builder.class_idx >= 0) {
        const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];
        char source[FEATURE_SRC_MAX];
        snprintf(source, sizeof(source), "%.10s Stufe 1", cls->name);

        for (int i = 0; i < cls->feature_count; i++) {
            Feature f;
            memset(&f, 0, sizeof(f));
            f.character_id = c->id;
            snprintf(f.name,        sizeof(f.name),        "%.63s", cls->features[i].name);
            snprintf(f.source,      sizeof(f.source),      "%.31s", source);
            snprintf(f.description, sizeof(f.description), "%.255s", cls->features[i].description);
            if (strcmp(cls->id, "bard") == 0 &&
                strncmp(cls->features[i].name, "Bardische", 9) == 0) {
                int uses = cha_mod > 0 ? cha_mod : 1;
                f.uses_max = f.uses_current = uses;
                snprintf(f.recharge_on, sizeof(f.recharge_on), "Langer Rast");
            } else if (strcmp(cls->id, "fighter") == 0 &&
                       strncmp(cls->features[i].name, "Zweiter", 7) == 0) {
                f.uses_max = f.uses_current = 1;
                snprintf(f.recharge_on, sizeof(f.recharge_on), "Kurze Rast");
            } else if (strcmp(cls->id, "barbarian") == 0 &&
                       strncmp(cls->features[i].name, "Raserei", 7) == 0) {
                f.uses_max = f.uses_current = 2;
                snprintf(f.recharge_on, sizeof(f.recharge_on), "Langer Rast");
            } else if (strcmp(cls->id, "monk") == 0 &&
                       strncmp(cls->features[i].name, "Ki", 2) == 0) {
                f.uses_max = f.uses_current = 1;
                snprintf(f.recharge_on, sizeof(f.recharge_on), "Kurze Rast");
            } else if (strcmp(cls->id, "paladin") == 0 &&
                       strncmp(cls->features[i].name, "Handauf", 7) == 0) {
                f.uses_max = f.uses_current = 5;
                snprintf(f.recharge_on, sizeof(f.recharge_on), "Langer Rast");
            }
            features_db_save(&f);
        }

        // Waffen aus Startausruestung
        const EquipOption* opt = (g_builder.equip_choice == 0)
                                 ? &cls->equip_a : &cls->equip_b;
        for (int i = 0; i < opt->item_count; i++) {
            const StartItem* it = &opt->items[i];
            if (it->category != ITEM_CAT_WEAPON || !it->equipped) continue;
            const WpnInfo* w = wpn_lookup(it->name);
            if (!w) continue;

            int ability_mod;
            if (w->is_ranged)        ability_mod = dex_mod;
            else if (w->is_finesse)  ability_mod = (dex_mod > str_mod) ? dex_mod : str_mod;
            else                     ability_mod = str_mod;

            Attack a;
            memset(&a, 0, sizeof(a));
            a.character_id = c->id;
            snprintf(a.name,        sizeof(a.name),        "%.63s",  it->name);
            snprintf(a.hit_bonus,   sizeof(a.hit_bonus),   "%+d",    pb + ability_mod);
            snprintf(a.damage,      sizeof(a.damage),      "%s%+d",  w->dmg_dice, ability_mod);
            snprintf(a.damage_type, sizeof(a.damage_type), "%.31s",  w->dmg_type);
            snprintf(a.notes,       sizeof(a.notes),       "%.127s", w->notes);
            a.action_type = ATTACK_ACTION;
            attacks_db_save(&a);
        }
    }
}

// ---- Charakter fertigstellen und speichern ---------------------------------

int builder_finish(void) {
    Character c;
    memset(&c, 0, sizeof(c));

    // Name
    snprintf(c.name, CHAR_NAME_MAX, "%s",
             g_builder.name[0] ? g_builder.name : "Held");

    // Rasse
    if (g_builder.race_idx >= 0)
        snprintf(c.race, CHAR_NAME_MAX, "%s", SRD_RACES[g_builder.race_idx].name);

    // Klasse
    if (g_builder.class_idx >= 0)
        snprintf(c.class_name, CHAR_NAME_MAX, "%s", SRD_CLASSES[g_builder.class_idx].name);

    // Unterklasse (leer bei Stufe 1)
    c.subclass[0] = '\0';

    // Hintergrund
    if (g_builder.bg_idx >= 0)
        snprintf(c.background, CHAR_NAME_MAX, "%s", SRD_BACKGROUNDS[g_builder.bg_idx].name);

    // Gesinnung
    snprintf(c.alignment, CHAR_NAME_MAX, "%s", g_builder.alignment);

    // Stufe und XP
    c.level      = 1;
    c.experience = 0;

    // Attribute (Basis + Rassen-Boni)
    for (int i = 0; i < ABILITY_COUNT; i++)
        c.ability[i] = builder_final_ability(i);

    // HP
    c.hp_max     = builder_initial_hp();
    c.hp_current = c.hp_max;
    c.hp_temp    = 0;

    // AC und Geschwindigkeit
    c.armor_class = builder_initial_ac();
    c.speed = 30;
    if (g_builder.race_idx >= 0)
        c.speed = SRD_RACES[g_builder.race_idx].speed;

    // Fertigkeits-Uebungen: Klassen-Skills
    for (int i = 0; i < SKILL_COUNT; i++)
        c.skill_proficient[i] = g_builder.chosen_skills[i] ? 1 : 0;

    // Hintergrunds-Skills (addieren, Doppel-Proficiency bleibt bei 1)
    if (g_builder.bg_idx >= 0) {
        const SrdBackground* bg = &SRD_BACKGROUNDS[g_builder.bg_idx];
        for (int j = 0; j < bg->skill_prof_count; j++) {
            int idx = bg->skill_profs[j];
            if (idx >= 0 && idx < SKILL_COUNT)
                c.skill_proficient[idx] = 1;
        }
    }

    // Rettungswurf-Uebungen aus Klasse
    if (g_builder.class_idx >= 0) {
        const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];
        c.save_proficient[cls->save_prof[0]] = 1;
        c.save_proficient[cls->save_prof[1]] = 1;
    }

    // Inspiration, Todesrettungswuerfe
    c.inspiration           = 0;
    c.death_saves_successes = 0;
    c.death_saves_failures  = 0;

    // In Datenbank speichern
    if (character_db_save(&c) != 0)
        return 0;

    // Startausruestung in Inventar speichern
    if (g_builder.class_idx >= 0) {
        const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];
        const EquipOption* opt = (g_builder.equip_choice == 0)
                                 ? &cls->equip_a
                                 : &cls->equip_b;
        for (int i = 0; i < opt->item_count; i++) {
            Item item;
            memset(&item, 0, sizeof(item));
            item.character_id = c.id;
            strncpy(item.name, opt->items[i].name, ITEM_NAME_MAX - 1);
            item.quantity  = opt->items[i].quantity;
            item.weight    = opt->items[i].weight;
            item.value_cp  = opt->items[i].value_cp;
            item.category  = opt->items[i].category;
            item.equipped  = opt->items[i].equipped;
            inventory_db_save(&item);
        }
    }

    // Zauberplaetze bei Stufe 1 initialisieren
    if (g_builder.class_idx >= 0) {
        const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];
        if (cls->is_spellcaster && cls->spell_slots_l1 > 0) {
            SpellSlots slots;
            memset(&slots, 0, sizeof(slots));
            slots.character_id = c.id;
            slots.total[0]     = cls->spell_slots_l1;  // Grad-1-Plaetze (Index 0 = Grad 1)
            slots.used[0]      = 0;
            spell_slots_db_save(&slots);
        }
    }

    // Angriffe und Fähigkeiten aus Rasse + Klasse generieren
    builder_create_attacks_features(&c);

    // Aktiven Charakter setzen und Charakter-Screen oeffnen
    g_active_char_id = c.id;
    screen_replace(SCREEN_CHARACTER);
    return 1;
}
