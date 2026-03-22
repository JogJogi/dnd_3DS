#pragma once
#include "../models/character.h"

// ---- Hintergrund-Daten (SRD 5.1) -------------------------------------------

#define BG_MAX_SKILLS  2
#define BG_MAX_TOOLS   2

// Vollständige Hintergrunddefinition
typedef struct {
    const char* id;               // interner Schlüssel, z.B. "acolyte"
    const char* name;             // Anzeigename
    const char* description;      // Kurzbeschreibung (<100 Zeichen)
    const char* feature_name;     // Name des Hintergrundmerkmals
    const char* feature_desc;     // Beschreibung des Merkmals (<200 Zeichen)

    // Fertigkeits-Übungen (feste Auswahl, keine Wahl)
    int skill_profs[BG_MAX_SKILLS];  // SkillIndex-Werte, -1 = nicht belegt
    int skill_prof_count;

    // Werkzeug-/Sprachen-Übungen (als String)
    const char* tool_profs;       // z.B. "Zwei Sprachen nach Wahl"
    const char* languages;        // z.B. "Zwei Sprachen nach Wahl"

    // Startausrüstung (vereinfacht als String)
    const char* start_equipment;  // Beschreibung der Startausrüstung
    int  start_gold_cp;           // Startgold in Kupfermünzen (15 GP = 1500)
} SrdBackground;

// ---- Globale Hintergründe-Tabelle ------------------------------------------
extern const SrdBackground SRD_BACKGROUNDS[];
extern const int           SRD_BACKGROUND_COUNT;
