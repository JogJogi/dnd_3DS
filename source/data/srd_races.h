#pragma once
#include "../models/character.h"

// ---- Rassen-Daten (SRD 5.1 + Aarakocra) -----------------------------------
// Aarakocra: nicht im SRD 5.1, aber auf Benutzerwunsch enthalten.
// Alle anderen Rassen entsprechen dem SRD 5.1 (CC BY 4.0).

#define SRD_RACE_MAX_TRAITS   8
#define SRD_RACE_MAX_SUBRACES 6

// Eine einzelne Rassenfähigkeit
typedef struct {
    const char* name;
    const char* description;  // kurz (<120 Zeichen) für Top-Bildschirm
} RacialTrait;

// Unterrasse (z.B. Hügelzwerg, Bergelfe)
typedef struct {
    const char* id;
    const char* name;
    int  asi[ABILITY_COUNT];              // Zusätzliche ASI über Basisrasse hinaus
    const RacialTrait* traits;
    int  trait_count;
} Subrace;

// Vollständige Rassendefinition
typedef struct {
    const char* id;           // interner Schlüssel, z.B. "aarakocra"
    const char* name;         // Anzeigename
    const char* description;  // Kurzbeschreibung (<80 Zeichen)
    int  asi[ABILITY_COUNT];  // Attributs-Boni [STR,DEX,CON,INT,WIS,CHA]
    int  speed;               // Bewegungsrate in Fuß
    const char* size;         // "Klein" oder "Mittel"
    const char* languages;    // Sprachen als lesbarer String
    const char* extra_speed;  // NULL oder z.B. "50 Fuß Flug" für Sonderbeweg.
    const RacialTrait* traits;
    int  trait_count;
    const Subrace* subraces;
    int  subrace_count;
} SrdRace;

// ---- Globale Rassen-Tabelle ------------------------------------------------
extern const SrdRace SRD_RACES[];
extern const int     SRD_RACE_COUNT;

// Hilfsfunktion: ASI-Zusammenfassung formatieren, z.B. "+2 DEX +1 WIS"
// buf muss mindestens 48 Byte groß sein
void srd_race_asi_summary(const SrdRace* r, const Subrace* sub,
                          char* buf, int buflen);
