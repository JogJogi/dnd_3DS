#pragma once

// ---- Barden-Zauberliste (D&D 5e SRD) ----------------------------------------
// Enthaelt alle SRD-Zauber des Bardens von Zaubertrick bis Grad 9.
// Beschreibungen sind auf ~200 Zeichen gekuerzt fuer den 3DS-Bildschirm.

typedef struct {
    const char* name;           // Anzeigename (Deutsch)
    const char* name_en;        // Englischer Originalname (Referenz)
    int         level;          // 0 = Zaubertrick, 1-9 = Grad
    const char* school;         // Zauberspruch-Schule
    const char* casting_time;   // z.B. "1 Aktion"
    const char* range;          // z.B. "18 m"
    const char* components;     // z.B. "V, S, M"
    const char* duration;       // z.B. "Konzentration, bis zu 1 Min."
    const char* description;    // Kurze mechanische Beschreibung (~200 Zeichen)
    int         concentration;  // 1 = Konzentration noetig
    int         ritual;         // 1 = kann als Ritual gewirkt werden
} BardSpellEntry;

extern const BardSpellEntry BARD_CANTRIP_LIST[];
extern const int             BARD_CANTRIP_COUNT;

extern const BardSpellEntry BARD_SPELL_LIST[];
extern const int             BARD_SPELL_COUNT;

// Gibt Zeiger auf Zaubertricks zurueck, die ein Barde kennt
const BardSpellEntry* bard_get_cantrip(int index);
// Gibt Zeiger auf bekannte Zauber nach Stufe gefiltert zurueck
const BardSpellEntry* bard_get_spell(int index);
// Zaehlt Zauber eines bestimmten Grades in BARD_SPELL_LIST
int bard_spell_count_by_level(int level);
