#pragma once
#include <stdint.h>

#define SPELL_NAME_MAX    128
#define SPELL_TEXT_MAX    64
#define SPELL_DESC_MAX    512
#define SPELL_SLOT_LEVELS 9

typedef struct {
    int  id;
    int  character_id;
    char name[SPELL_NAME_MAX];
    int  spell_level;      // 0 = Cantrip, 1-9 = Spell Level
    char school[SPELL_TEXT_MAX];
    char casting_time[SPELL_TEXT_MAX];
    char range[SPELL_TEXT_MAX];
    char components[SPELL_TEXT_MAX];
    char duration[SPELL_TEXT_MAX];
    char description[SPELL_DESC_MAX];
    int  prepared;         // 0 oder 1
    int  concentration;    // 0 oder 1
    int  ritual;           // 0 oder 1
} Spell;

typedef struct {
    int character_id;
    int total[SPELL_SLOT_LEVELS];    // total[0] = Level-1-Slots usw.
    int used[SPELL_SLOT_LEVELS];
} SpellSlots;

int spellslots_available(const SpellSlots* s, int level);  // level 1-9
void spellslots_use(SpellSlots* s, int level);
void spellslots_restore_all(SpellSlots* s);
