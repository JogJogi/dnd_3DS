#pragma once

// ---- Unterklassen-Daten (D&D 5e) --------------------------------------------
// Bisher implementiert: Bard (Kolleg des Wissens, Kolleg der Tapferkeit)
// Weitere Klassen folgen in späteren Updates.

#define SUBCLASS_MAX_FEATURES 4

typedef struct {
    const char* name;
    const char* description;  // <200 Zeichen
    int         uses_max;     // 0 = passiv
    const char* recharge_on;  // "" = keine Aufladung
} SubclassFeature;

// Unterklassen-Feature-Gruppe für eine bestimmte Stufe
typedef struct {
    int                    level;
    const SubclassFeature* features;
    int                    feature_count;
} SubclassLevel;

// Vollständige Unterklassendefinition
typedef struct {
    const char*         id;           // z.B. "college_lore"
    const char*         name;         // "Kolleg des Wissens"
    const char*         class_id;     // "bard"
    const char*         description;  // Kurzbeschreibung (<100 Zeichen)
    int                 unlock_level; // Level an dem Unterklasse gewählt wird (3 für Barden)

    // Feature-Gruppen pro Level (NULL-terminiertes Array via count)
    const SubclassLevel* levels;
    int                  level_count;
} SrdSubclass;

extern const SrdSubclass SRD_SUBCLASSES[];
extern const int         SRD_SUBCLASS_COUNT;

// Gibt alle Unterklassen für eine Klasse zurück, out = Zeiger-Array, gibt Anzahl zurück
int srd_subclass_get_for_class(const char* class_id,
                                const SrdSubclass** out, int max_out);

// Gibt alle Features einer Unterklasse für eine bestimmte Stufe zurück
const SubclassLevel* srd_subclass_get_level(const SrdSubclass* sc, int level);
