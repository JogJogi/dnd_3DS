#pragma once
#include <stdint.h>

// Statische Referenz-Daten für Barden-Zauber (SRD 5.1)
// Wird NUR für das Zauberbuch (Browse) verwendet – nicht in der DB.

typedef struct {
    const char* name;
    uint8_t     level;        // 0 = Zaubertrick, 1-9 = Grad
    const char* school;       // z.B. "Verzauberung"
    const char* cast_time;    // z.B. "1 Aktion"
    const char* range;        // z.B. "60 Fuß"
    const char* duration;     // z.B. "Konz. 1 Min"
    const char* description;  // Kurzbeschreibung (Deutsch)
    uint8_t     ritual;       // 1 = kann als Ritual gewirkt werden
    uint8_t     concentration; // 1 = benötigt Konzentration
} SpellRef;

extern const SpellRef SPELL_REFS[];
extern const int      SPELL_REF_COUNT;
