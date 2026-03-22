#pragma once
#include <3ds.h>
#include "../models/character.h"
#include "screen_manager.h"

// ---- Builder-Schritte -------------------------------------------------------
// Intern bis zu 9 Sub-Schritte; Zauberwirker haben SPELLS-Schritt,
// Schurken haben EXPERTISE-Schritt. Nicht alle Schritte erscheinen immer.
#define BUILDER_STEP_RACE        0
#define BUILDER_STEP_CLASS       1
#define BUILDER_STEP_SKILLS      2   // Teil der Klassen-Auswahl
#define BUILDER_STEP_EXPERTISE   3   // Nur fuer Schurken (Stufe 1)
#define BUILDER_STEP_SPELLS      4   // Nur fuer Zauberwirker (Cantrips + bekannte Zauber)
#define BUILDER_STEP_ABILITIES   5
#define BUILDER_STEP_BACKGROUND  6
#define BUILDER_STEP_DETAILS     7
#define BUILDER_STEP_REVIEW      8
#define BUILDER_STEP_COUNT       9

// Methode fuer Attributswerte
#define ABILITY_METHOD_STANDARD  0   // Standard-Array {15,14,13,12,10,8}
#define ABILITY_METHOD_POINTBUY  1   // Punkt-Kauf (27 Punkte)
#define ABILITY_METHOD_MANUAL    2   // Freieeingabe

#define POINTBUY_BUDGET         27
#define POINTBUY_MIN             8
#define POINTBUY_MAX            15

// Anzahl der anzeigbaren Nutzer-Schritte (fuer Fortschrittsanzeige)
// Skills, Expertise und Spells sind interne Sub-Schritte → max 7 sichtbare Schritte
#define BUILDER_VISIBLE_STEPS    7

// Maximale Anzahl waehlbarer Zauber/Cantrips im Builder
#define BUILDER_MAX_CANTRIPS     8
#define BUILDER_MAX_SPELLS      10

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

    // ---- Zauberauswahl (fuer Zauberwirker) ----------------------------------
    // Speichert Indizes in BARD_CANTRIP_LIST[] bzw. BARD_SPELL_LIST[]
    int chosen_cantrip_idx[BUILDER_MAX_CANTRIPS];
    int chosen_cantrip_count;
    int chosen_spell_idx[BUILDER_MAX_SPELLS];
    int chosen_spell_count;

    // ---- Expertise-Auswahl (fuer Schurken Stufe 1) -------------------------
    int chosen_expertise[SKILL_COUNT]; // 1 = Expertise fuer diesen Skill gewaehlt
    int expertise_count;

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
