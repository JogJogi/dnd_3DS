#pragma once
#include "../models/character.h"
#include "../models/item.h"

// ---- Klassen-Daten (SRD 5.1) -----------------------------------------------

#define CLASS_MAX_FEATURES    8
#define CLASS_MAX_SKILL_OPTS 18   // Maximal alle Skills wählbar (z.B. Barde)
#define CLASS_MAX_ITEMS       8   // Items pro Ausrüstungsoption

// Ein Klassen-Feature (Stufe 1)
typedef struct {
    const char* name;
    const char* description;  // kurz (<140 Zeichen)
} ClassFeature;

// Ein einzelner Startausrüstungs-Gegenstand
typedef struct {
    const char* name;
    int         quantity;
    float       weight;     // in lbs
    int         value_cp;   // Wert in Kupfermünzen
    ItemCategory category;
    int         equipped;   // direkt ausgerüstet
} StartItem;

// Eine Ausrüstungsoption (A oder B)
typedef struct {
    const char*      desc;           // Lesbare Beschreibung für UI
    const StartItem* items;          // Zeiger auf statisches Item-Array
    int              item_count;
} EquipOption;

// Vollständige Klassendefinition
typedef struct {
    const char* id;           // interner Schlüssel, z.B. "bard"
    const char* name;         // Anzeigename
    const char* description;  // Kurzbeschreibung (<80 Zeichen)
    int  hit_die;             // 6, 8, 10 oder 12

    // Primärattribut (für Anzeige)
    int  primary_ability;     // ABILITY_* Index

    // Rettungswurf-Übungen (immer genau 2)
    int  save_prof[2];        // ABILITY_* Indizes

    // Fertigkeiten
    int  num_skills;          // Anzahl wählbarer Fertigkeiten
    int  any_skill;           // 1 = aus allen 18, 0 = aus skill_options
    const int* skill_options; // NULL wenn any_skill=1
    int  skill_option_count;  // 0 wenn any_skill=1

    // Rüstungs- und Waffen-Übungen
    const char* armor_profs;
    const char* weapon_profs;

    // Stufe-1-Features
    const ClassFeature* features;
    int  feature_count;

    // Zauberwirker-Info (0 wenn keine Magie)
    int  is_spellcaster;
    int  spellcasting_ability; // ABILITY_* oder -1
    int  cantrips_known;       // Zaubertricks bei Stufe 1
    int  spells_known;         // Bekannte Zauber bei Stufe 1
    int  spell_slots_l1;       // Zauberplätze Grad 1 bei Stufe 1

    // Startausrüstung (2 Optionen)
    EquipOption equip_a;
    EquipOption equip_b;
} SrdClass;

// ---- Globale Klassen-Tabelle -----------------------------------------------
extern const SrdClass SRD_CLASSES[];
extern const int      SRD_CLASS_COUNT;
