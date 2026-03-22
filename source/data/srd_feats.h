#pragma once

// ---- Feats (D&D 5e SRD) -----------------------------------------------------
// Feats koennen als Alternative zu einem Attributspunktbonus (ASI) gewaehlt werden.
// Half-Feats: asi_ability >= 0 → gibt zusaetzlich +1 auf dieses Attribut.

#define FEAT_NAME_MAX 48

typedef struct {
    const char* id;           // interner Schluessel, z.B. "alert"
    const char* name;         // Anzeigename
    const char* description;  // Mechanische Beschreibung (<300 Zeichen)
    int         asi_ability;  // -1 = kein Half-Feat; 0-5 (ABILITY_*) = +1 auf dieses Attribut
    const char* prerequisite; // NULL oder Voraussetzung als Text
} SrdFeat;

extern const SrdFeat SRD_FEATS[];
extern const int     SRD_FEAT_COUNT;
