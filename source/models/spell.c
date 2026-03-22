#include "spell.h"

int spellslots_available(const SpellSlots* s, int level) {
    if (level < 1 || level > SPELL_SLOT_LEVELS) return 0;
    int idx = level - 1;
    return s->total[idx] - s->used[idx];
}

void spellslots_use(SpellSlots* s, int level) {
    if (level < 1 || level > SPELL_SLOT_LEVELS) return;
    int idx = level - 1;
    if (s->used[idx] < s->total[idx])
        s->used[idx]++;
}

void spellslots_restore_all(SpellSlots* s) {
    for (int i = 0; i < SPELL_SLOT_LEVELS; i++)
        s->used[i] = 0;
}
