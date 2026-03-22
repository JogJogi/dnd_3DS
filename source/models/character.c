#include "character.h"
#include "../utils/dnd_rules.h"
#include <string.h>

const char* SKILL_NAMES[SKILL_COUNT] = {
    "Acrobatics", "Animal Handling", "Arcana", "Athletics",
    "Deception", "History", "Insight", "Intimidation",
    "Investigation", "Medicine", "Nature", "Perception",
    "Performance", "Persuasion", "Religion", "Sleight of Hand",
    "Stealth", "Survival"
};

// Welcher Ability Score (Index) steuert den Skill
const int SKILL_ABILITY[SKILL_COUNT] = {
    ABILITY_DEX,  // Acrobatics
    ABILITY_WIS,  // Animal Handling
    ABILITY_INT,  // Arcana
    ABILITY_STR,  // Athletics
    ABILITY_CHA,  // Deception
    ABILITY_INT,  // History
    ABILITY_WIS,  // Insight
    ABILITY_CHA,  // Intimidation
    ABILITY_INT,  // Investigation
    ABILITY_WIS,  // Medicine
    ABILITY_INT,  // Nature
    ABILITY_WIS,  // Perception
    ABILITY_CHA,  // Performance
    ABILITY_CHA,  // Persuasion
    ABILITY_INT,  // Religion
    ABILITY_DEX,  // Sleight of Hand
    ABILITY_DEX,  // Stealth
    ABILITY_WIS   // Survival
};

const char* ABILITY_NAMES[ABILITY_COUNT] = {
    "Strength", "Dexterity", "Constitution",
    "Intelligence", "Wisdom", "Charisma"
};

const char* ABILITY_SHORT[ABILITY_COUNT] = {
    "STR", "DEX", "CON", "INT", "WIS", "CHA"
};

int character_ability_modifier(const Character* c, int i) {
    return dnd_modifier(c->ability[i]);
}

int character_proficiency_bonus(const Character* c) {
    return dnd_proficiency_bonus(c->level);
}

// Jack of All Trades: Barden ab Stufe 2 addieren halben PB auf untrainierte Checks
static int character_has_jack_of_all_trades(const Character* c) {
    return (strcmp(c->class_name, "Barde") == 0 && c->level >= 2);
}

int character_skill_bonus(const Character* c, int skill_idx) {
    int ability = SKILL_ABILITY[skill_idx];
    int mod     = dnd_modifier(c->ability[ability]);
    int pb      = dnd_proficiency_bonus(c->level);
    int prof    = c->skill_proficient[skill_idx];

    if (prof == 0 && character_has_jack_of_all_trades(c)) {
        // Halber PB (abgerundet) auf alle untrainierten Fertigkeitswuerfe
        return mod + pb / 2;
    }
    return dnd_skill_bonus(mod, pb, prof);
}

int character_save_bonus(const Character* c, int ability_idx) {
    int mod = dnd_modifier(c->ability[ability_idx]);
    int pb  = dnd_proficiency_bonus(c->level);
    return mod + (c->save_proficient[ability_idx] ? pb : 0);
}

int character_passive_perception(const Character* c) {
    return 10 + character_skill_bonus(c, SKILL_PERCEPTION);
}

int character_initiative(const Character* c) {
    return dnd_modifier(c->ability[ABILITY_DEX]);
}

int character_carry_capacity(const Character* c) {
    return c->ability[ABILITY_STR] * 15;  // D&D 5e: STR * 15 lbs
}
