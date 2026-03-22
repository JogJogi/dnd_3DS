#include "dnd_rules.h"

int dnd_modifier(int score) {
    // Korrekte Floor-Division auch für negative Zahlen
    int diff = score - 10;
    return diff >= 0 ? diff / 2 : (diff - 1) / 2;
}

int dnd_proficiency_bonus(int level) {
    if (level <= 0)  return 2;
    if (level <= 4)  return 2;
    if (level <= 8)  return 3;
    if (level <= 12) return 4;
    if (level <= 16) return 5;
    return 6;  // Level 17-20
}

int dnd_skill_bonus(int ability_mod, int proficiency_bonus, int proficient) {
    // proficient: 0 = keine, 1 = proficient, 2 = expertise (doppelter bonus)
    return ability_mod + proficiency_bonus * proficient;
}

// D&D 5e XP-Tabelle (Gesamt-XP für Level-Up)
static const int XP_TABLE[21] = {
    0,       // Level 0 (unused)
    0,       // Level 1
    300,     // Level 2
    900,     // Level 3
    2700,    // Level 4
    6500,    // Level 5
    14000,   // Level 6
    23000,   // Level 7
    34000,   // Level 8
    48000,   // Level 9
    64000,   // Level 10
    85000,   // Level 11
    100000,  // Level 12
    120000,  // Level 13
    140000,  // Level 14
    165000,  // Level 15
    195000,  // Level 16
    225000,  // Level 17
    265000,  // Level 18
    305000,  // Level 19
    355000   // Level 20
};

int dnd_xp_for_level(int level) {
    if (level < 1)  return 0;
    if (level > 20) return XP_TABLE[20];
    return XP_TABLE[level];
}

int dnd_level_from_xp(int xp) {
    int level = 1;
    for (int i = 2; i <= 20; i++) {
        if (xp >= XP_TABLE[i]) level = i;
        else break;
    }
    return level;
}

int dnd_hp_clamp(int hp, int hp_max) {
    if (hp < 0)       return 0;
    if (hp > hp_max)  return hp_max;
    return hp;
}
