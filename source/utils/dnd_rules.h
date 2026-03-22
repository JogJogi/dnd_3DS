#pragma once

// Ability Modifier: (score - 10) / 2, abgerundet
int dnd_modifier(int ability_score);

// Proficiency Bonus nach Level (D&D 5e)
int dnd_proficiency_bonus(int level);

// Skill-Bonus = Ability Modifier + (proficient * prof_bonus) + (expertise * prof_bonus)
int dnd_skill_bonus(int ability_mod, int proficiency_bonus, int proficient);

// XP-Schwellenwerte für Level-Up (5e Standard-Tabelle)
int dnd_xp_for_level(int level);     // XP die man für dieses Level braucht
int dnd_level_from_xp(int xp);       // Level aus XP berechnen

// HP-Berechnung Hilfe
int dnd_hp_clamp(int hp, int hp_max);  // Stellt sicher: 0 <= hp <= hp_max + temp
