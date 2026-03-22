#pragma once
#include "../models/spell.h"

#define SPELLS_MAX 64

// Lädt alle Zauber eines Charakters. Gibt Anzahl zurück.
int  spells_db_load(int char_id, Spell* out, int max);

// Speichert Zauber. s->id==0 → INSERT (setzt id), sonst UPDATE.
int  spells_db_save(Spell* s);

void spells_db_delete(int spell_id);
void spells_db_set_prepared(int spell_id, int prepared);

// Zauberschlitze laden/speichern
int  spell_slots_db_load(int char_id, SpellSlots* out);
void spell_slots_db_save(const SpellSlots* s);
