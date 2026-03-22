#include "srd_subclasses.h"
#include <string.h>

// ============================================================================
// BARDE – KOLLEG DES WISSENS (College of Lore)
// ============================================================================

static const SubclassFeature LORE_L3[] = {
    {
        "Bonusfertigkeiten (Kolleg des Wissens)",
        "Du erhaeltst Uebung in 3 weiteren Fertigkeiten deiner Wahl.",
        0, ""
    },
    {
        "Schneidende Worte",
        "Reaktion: Wenn ein Wesen in 18m einen Angriff, Fertigkeit oder Schadenswurf macht, "
        "ziehe 1 Bardische Inspiration ab. Kostet 1 Inspiration-Wuerfel.",
        0, ""
    },
};

static const SubclassFeature LORE_L6[] = {
    {
        "Zusaetzliche Magische Geheimnisse",
        "Lerne 2 Zauber aus beliebigen Klassenlisten, die als Bardenzauber gelten. "
        "Dies geschieht 2 Stufen frueher als normal.",
        0, ""
    },
};

static const SubclassFeature LORE_L14[] = {
    {
        "Grenzenlose Begabung",
        "Wenn du einen Attributs- oder Fertigkeitswurf machst und keinen Erfolg erzielst, "
        "kannst du einen Bardischen Inspiration-Wuerfel zu deinem Ergebnis addieren.",
        0, ""
    },
};

static const SubclassLevel LORE_LEVELS[] = {
    { 3,  LORE_L3,  2 },
    { 6,  LORE_L6,  1 },
    { 14, LORE_L14, 1 },
};

// ============================================================================
// BARDE – KOLLEG DER TAPFERKEIT (College of Valor)
// ============================================================================

static const SubclassFeature VALOR_L3[] = {
    {
        "Bonusprofizienz (Kolleg der Tapferkeit)",
        "Du erhaeltst Uebung mit mittlerer Ruestung, Schilden und Kriegswaffen.",
        0, ""
    },
    {
        "Kampfinspiration",
        "Wesen, die deine Bardische Inspiration erhalten, koennen den Wuerfel zu "
        "einem Schadenswurf oder ihrer RK als Reaktion hinzufuegen, wenn sie getroffen werden.",
        0, ""
    },
};

static const SubclassFeature VALOR_L6[] = {
    {
        "Extra-Angriff",
        "Wenn du die Angriffsaktion verwendest, kannst du 2 Angriffe machen statt einem.",
        0, ""
    },
};

static const SubclassFeature VALOR_L14[] = {
    {
        "Kampfmagie",
        "Wenn du einen Zauber wirkst, kannst du als Bonus-Aktion einen Waffenangriff machen.",
        0, ""
    },
};

static const SubclassLevel VALOR_LEVELS[] = {
    { 3,  VALOR_L3,  2 },
    { 6,  VALOR_L6,  1 },
    { 14, VALOR_L14, 1 },
};

// ============================================================================
// GLOBALE TABELLE
// ============================================================================

const SrdSubclass SRD_SUBCLASSES[] = {
    {
        "college_lore",
        "Kolleg des Wissens",
        "bard",
        "Geschwister des Wortes: Schneidende Worte, Bonus-Fertigkeiten, Magische Geheimnisse.",
        3,
        LORE_LEVELS, 3
    },
    {
        "college_valor",
        "Kolleg der Tapferkeit",
        "bard",
        "Krieger-Bard: Kampfinspiration, Extra-Angriff, Kampfmagie.",
        3,
        VALOR_LEVELS, 3
    },
};

const int SRD_SUBCLASS_COUNT = 2;

// ============================================================================
// HILFSFUNKTIONEN
// ============================================================================

int srd_subclass_get_for_class(const char* class_id,
                                const SrdSubclass** out, int max_out) {
    int count = 0;
    for (int i = 0; i < SRD_SUBCLASS_COUNT && count < max_out; i++) {
        if (strcmp(SRD_SUBCLASSES[i].class_id, class_id) == 0) {
            out[count++] = &SRD_SUBCLASSES[i];
        }
    }
    return count;
}

const SubclassLevel* srd_subclass_get_level(const SrdSubclass* sc, int level) {
    for (int i = 0; i < sc->level_count; i++) {
        if (sc->levels[i].level == level)
            return &sc->levels[i];
    }
    return NULL;
}
