#pragma once
#include <stdint.h>

#define CHAR_NAME_MAX   64
#define CHAR_TEXT_MAX  256
#define CHAR_LONG_MAX  512

// D&D 5e Fähigkeiten
typedef enum {
    SKILL_ACROBATICS = 0,
    SKILL_ANIMAL_HANDLING,
    SKILL_ARCANA,
    SKILL_ATHLETICS,
    SKILL_DECEPTION,
    SKILL_HISTORY,
    SKILL_INSIGHT,
    SKILL_INTIMIDATION,
    SKILL_INVESTIGATION,
    SKILL_MEDICINE,
    SKILL_NATURE,
    SKILL_PERCEPTION,
    SKILL_PERFORMANCE,
    SKILL_PERSUASION,
    SKILL_RELIGION,
    SKILL_SLEIGHT_OF_HAND,
    SKILL_STEALTH,
    SKILL_SURVIVAL,
    SKILL_COUNT
} SkillIndex;

extern const char* SKILL_NAMES[SKILL_COUNT];
extern const int   SKILL_ABILITY[SKILL_COUNT]; // Welcher Ability Score (0=STR..5=CHA)

// Ability Score Indizes
#define ABILITY_STR 0
#define ABILITY_DEX 1
#define ABILITY_CON 2
#define ABILITY_INT 3
#define ABILITY_WIS 4
#define ABILITY_CHA 5
#define ABILITY_COUNT 6

extern const char* ABILITY_NAMES[ABILITY_COUNT];
extern const char* ABILITY_SHORT[ABILITY_COUNT];

typedef struct {
    int id;
    char name[CHAR_NAME_MAX];
    char class_name[CHAR_NAME_MAX];
    char subclass[CHAR_NAME_MAX];
    int  level;
    char race[CHAR_NAME_MAX];
    char background[CHAR_NAME_MAX];
    char alignment[CHAR_NAME_MAX];
    int  experience;

    // Ability Scores
    int  ability[ABILITY_COUNT];  // [STR, DEX, CON, INT, WIS, CHA]

    // HP
    int  hp_max;
    int  hp_current;
    int  hp_temp;

    // Combat
    int  armor_class;
    int  speed;

    // Status
    int  inspiration;         // 0 oder 1
    int  death_saves_successes;
    int  death_saves_failures;

    // Proficiency
    int  skill_proficient[SKILL_COUNT];   // 0=keine, 1=proficient, 2=expertise
    int  save_proficient[ABILITY_COUNT];  // 0 oder 1

    // RP-Text
    char personality_traits[CHAR_TEXT_MAX];
    char ideals[CHAR_TEXT_MAX];
    char bonds[CHAR_TEXT_MAX];
    char flaws[CHAR_TEXT_MAX];
    char backstory[CHAR_LONG_MAX];
} Character;

// Berechnete Werte (nicht in DB gespeichert)
int character_ability_modifier(const Character* c, int ability_idx);
int character_proficiency_bonus(const Character* c);
int character_skill_bonus(const Character* c, int skill_idx);
int character_save_bonus(const Character* c, int ability_idx);
int character_passive_perception(const Character* c);
int character_initiative(const Character* c);
int character_carry_capacity(const Character* c);  // in lbs
