#pragma once
#include <3ds.h>
#include "../models/character.h"
#include "screen_manager.h"

// ---- Builder-Schritte -------------------------------------------------------
// Intern 8 Sub-Schritte, fuer den Nutzer sichtbar als "Schritt X/6"
#define BUILDER_STEP_RACE        0
#define BUILDER_STEP_CLASS       1
#define BUILDER_STEP_SKILLS      2   // Teil der Klassen-Auswahl
#define BUILDER_STEP_ABILITIES   3
#define BUILDER_STEP_BACKGROUND  4
#define BUILDER_STEP_DETAILS     5
#define BUILDER_STEP_REVIEW      6
#define BUILDER_STEP_COUNT       7

// Methode fuer Attributswerte
#define ABILITY_METHOD_STANDARD  0   // Standard-Array {15,14,13,12,10,8}
#define ABILITY_METHOD_POINTBUY  1   // Punkt-Kauf (27 Punkte)
#define ABILITY_METHOD_MANUAL    2   // Freieeingabe

#define POINTBUY_BUDGET         27
#define POINTBUY_MIN             8
#define POINTBUY_MAX            15

// Anzahl der anzeigbaren Nutzer-Schritte (fuer Fortschrittsanzeige)
// Skills ist interner Sub-Schritt von CLASS → 6 sichtbare Schritte
#define BUILDER_VISIBLE_STEPS    6

// ---- Builder-Status (globaler Zustand durch die Erstellung) ----------------
typedef struct {
    int step;                          // BUILDER_STEP_*
    int dirty;                         // 1 wenn Schritt-Init noetig

    // ---- Auswahl ------------------------------------------------------------
    int race_idx;                      // Index in SRD_RACES, -1 = keine Wahl
    int subrace_idx;                   // Index in race.subraces, -1 = keine/nicht noetig

    int class_idx;                     // Index in SRD_CLASSES
    int chosen_skills[SKILL_COUNT];    // 1 = gewaehlt (Klassen-Fertigkeiten)

    int bg_idx;                        // Index in SRD_BACKGROUNDS

    // Hintergrunds-ASI (D&D 2024): +2 auf ein Attribut, +1 auf ein anderes
    // bg_asi[i] = 0, 1 oder 2 (Boni die auf Attribut i verteilt werden)
    // Gesamtsumme muss 3 ergeben (2+1 oder 1+1+1)
    int bg_asi[ABILITY_COUNT];

    // ---- Attribute ----------------------------------------------------------
    int ability_method;                // ABILITY_METHOD_*
    int abilities[ABILITY_COUNT];      // Basis-Scores (VOR Rassen-Boni)
    // Standard-Array: abilities[] ist bereits eine Permutation von {15,14,13,12,10,8}
    // Point Buy / Manual: abilities[] sind direkte Werte

    // ---- Details ------------------------------------------------------------
    char name[CHAR_NAME_MAX];
    char alignment[CHAR_NAME_MAX];

    // Ausrüstungsoption: 0=Option A, 1=Option B
    int  equip_choice;
} BuilderState;

extern BuilderState   g_builder;
extern Screen         g_screen_builder;

// ---- Schritt-Navigation (von Sub-Screens aufgerufen) -----------------------
void builder_next_step(void);
void builder_prev_step(void);

// Charakter aus Builder-Zustand zusammensetzen und speichern
// Gibt 1 bei Erfolg zurueck, 0 bei Fehler
int builder_finish(void);

// Berechneter End-Wert eines Attributs (Basis + Rassen-Bonus)
int builder_final_ability(int ability_idx);

// Berechnete HP bei Stufe 1 (max Hit Die + CON-Mod)
int builder_initial_hp(void);

// Berechnete AC bei Stufe 1 (10 + DEX-Mod, ungeruestet)
int builder_initial_ac(void);

// Pruefen ob Schritt vollstaendig ausgefuellt ist
int builder_step_complete(int step);

// Zeigt den Fortschrittsschritt fuer den Nutzer an (1-6)
int builder_visible_step(void);
