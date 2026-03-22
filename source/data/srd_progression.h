#pragma once

// ---- Stufenaufstiegs-Daten (D&D 5e SRD) ------------------------------------
// Enthält Zauberschlitze, neue Features und ASI-Stufen pro Klasse und Level.
// Stufen 1-20 vollständig für Zauberschlitze; Features für Stufen 2-10.

#define PROG_SLOT_LEVELS  9    // Spell-Level 1-9
#define PROG_LEVELS      20    // Level 1-20
#define PROG_MAX_FEATS    4    // Max neue Features pro Stufe

typedef struct {
    const char* name;
    const char* description;
    int         uses_max;       // 0 = passiv / unbegrenzt
    const char* recharge_on;    // "" = keine Aufladung
} ProgFeat;

typedef struct {
    const int*       slots;          // Zeiger auf int[9] Zauberschlitze dieser Stufe
    int              spells_known;   // 0=unveraendert/-1=vorbereitete Klasse
    int              is_asi;         // 1 = ASI verfuegbar bei dieser Stufe
    const ProgFeat*  feats;          // neue Features (NULL wenn keine)
    int              feat_count;
} ProgLevel;

typedef struct {
    const char*     class_id;
    ProgLevel       levels[PROG_LEVELS]; // Index 0 = Stufe 1
} ClassProg;

// Globale Tabelle
extern const ClassProg SRD_CLASS_PROG[];
extern const int       SRD_CLASS_PROG_COUNT;

// Hilfsfunktionen
const ClassProg* srd_prog_get(const char* class_id);
void  srd_prog_spell_slots(const char* class_id, int level, int out_slots[PROG_SLOT_LEVELS]);
int   srd_prog_is_asi(const char* class_id, int level);
int   srd_prog_feats(const char* class_id, int level,
                     const ProgFeat** out_feats);  // gibt Anzahl zurueck
// HP-Zuwachs Optionen
int   srd_prog_hp_average(int hit_die, int con_mod); // hit_die/2+1 + CON
int   srd_prog_hp_max(int hit_die, int con_mod);     // hit_die + CON
