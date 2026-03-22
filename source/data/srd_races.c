#include "srd_races.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// AARAKOCRA (nicht im SRD 5.1, auf Benutzerwunsch enthalten)
// ============================================================================
static const RacialTrait AARAKOCRA_TRAITS[] = {
    {
        "Flug",
        "Fluggeschwindigkeit 50 Fuss. Nicht nutzbar mit mittlerer/schwerer Ruestung."
    },
    {
        "Klauen",
        "Natuerliche Waffe. Unbewaffneter Angriff: 1W6 + STR oder DEX Hiebschaden."
    },
    {
        "Windrufer",
        "Du kannst Windstoss (Gust of Wind) 1x pro langer Rast wirken, wenn du Stufe 3+ erreichst."
    },
};

// ============================================================================
// MENSCH (SRD 5.1 – Standard-Mensch: +1 auf alle Attribute)
// ============================================================================
static const RacialTrait HUMAN_TRAITS[] = {
    {
        "Vielseitigkeit",
        "+1 auf alle sechs Attribute."
    },
};

// ============================================================================
// ELF – Basisrasse (SRD 5.1)
// ============================================================================
static const RacialTrait ELF_TRAITS[] = {
    {
        "Dunkelsicht",
        "Siehst 60 Fuss weit in Dunkelheit als waerest es gedimmtes Licht."
    },
    {
        "Feenvorfahren",
        "Vorteil auf RW gegen Bezauberung. Immun gegen Magischen Schlaf."
    },
    {
        "Trance",
        "Kein Schlaf noetig. Meditierst 4 Stunden statt 8 Stunden."
    },
    {
        "Elf-Waffen-Training",
        "Uebung mit Langschwert, Kurzschwert, Kurzbogen und Langbogen."
    },
};

// Unterrasse: Hochelf
static const RacialTrait HIGH_ELF_TRAITS[] = {
    {
        "Zaubertrick",
        "Kennst einen Magier-Zaubertrick deiner Wahl (INT-basiert)."
    },
    {
        "Extra-Sprache",
        "Sprichst eine zusaetzliche Sprache deiner Wahl."
    },
};
// Unterrasse: Waldelfe
static const RacialTrait WOOD_ELF_TRAITS[] = {
    {
        "Natuerliche Schritt",
        "Bewegst dich ohne Abzug durch schwieriges natuerliches Gelaende."
    },
    {
        "Verstecken in der Wildnis",
        "Kannst dich auch bei leichter Verdeckung durch Natur verstecken."
    },
};
// Unterrasse: Dunkelelf (Drow)
static const RacialTrait DROW_TRAITS[] = {
    {
        "Ueberlegene Dunkelsicht",
        "Dunkelsicht-Reichweite 120 Fuss statt 60 Fuss."
    },
    {
        "Sonnenlicht-Empfindlichkeit",
        "Nachteil auf Angriffe und WEI (Wahrnehmung) bei direktem Sonnenlicht."
    },
    {
        "Drow-Magie",
        "Kannst Tanzen der Lichter (Zaubertrick), Dancing Lights, etc. wirken."
    },
};

static const Subrace ELF_SUBRACES[] = {
    {
        "high_elf", "Hochelf",
        { 0, 0, 0, 1, 0, 0 },   // +1 INT
        HIGH_ELF_TRAITS,
        2
    },
    {
        "wood_elf", "Waldelfe",
        { 0, 0, 0, 0, 1, 0 },   // +1 WIS
        WOOD_ELF_TRAITS,
        2
    },
    {
        "drow", "Dunkelelf (Drow)",
        { 0, 0, 0, 0, 0, 1 },   // +1 CHA
        DROW_TRAITS,
        3
    },
};

// ============================================================================
// ZWERG – Basisrasse (SRD 5.1)
// ============================================================================
static const RacialTrait DWARF_TRAITS[] = {
    {
        "Dunkelsicht",
        "Siehst 60 Fuss weit in Dunkelheit."
    },
    {
        "Zwerg-Stabilitaet",
        "Vorteil auf RW gegen Gift, Resistenz gegen Giftschaden."
    },
    {
        "Zwerg-Kampftraining",
        "Uebung mit Streitaxt, Handaxt, Leichtem Hammer und Kriegshammer."
    },
    {
        "Werkzeug-Uebung",
        "Uebung mit einem Handwerker-Werkzeug deiner Wahl."
    },
    {
        "Steinkunst",
        "Bonus auf Wahrnehmung bezueglich Stein: Fallen, Schraege, Ortskenntnis."
    },
};

static const RacialTrait HILL_DWARF_TRAITS[] = {
    {
        "Zwergenausdauer",
        "+1 Trefferpunkte pro Stufe."
    },
};
static const RacialTrait MOUN_DWARF_TRAITS[] = {
    {
        "Zwerg-Ruestungstraining",
        "Uebung mit leichter und mittlerer Ruestung."
    },
};

static const Subrace DWARF_SUBRACES[] = {
    {
        "hill_dwarf", "Huegelzwerg",
        { 0, 0, 0, 0, 1, 0 },   // +1 WIS
        HILL_DWARF_TRAITS,
        1
    },
    {
        "mountain_dwarf", "Bergzwerg",
        { 2, 0, 0, 0, 0, 0 },   // +2 STR
        MOUN_DWARF_TRAITS,
        1
    },
};

// ============================================================================
// HALBLING (SRD 5.1)
// ============================================================================
static const RacialTrait HALFLING_TRAITS[] = {
    {
        "Glueckspilz",
        "Bei 1 auf Angriff, Attribut- oder RW: nochmal wuerfeln, neues Ergebnis gilt."
    },
    {
        "Tapferkeit",
        "Vorteil auf RW gegen Angst."
    },
    {
        "Flinke Fuesse",
        "Kannst dich durch groessere Wesen bewegen (nicht im Feld aufhalten)."
    },
};

static const RacialTrait LIGHTFOOT_TRAITS[] = {
    {
        "Natuerlich heimlich",
        "Kannst dich hinter Wesen verstecken, die mindestens mittelgross sind."
    },
};
static const RacialTrait STOUT_TRAITS[] = {
    {
        "Zaehigkeit",
        "Vorteil auf RW gegen Gift, Resistenz gegen Giftschaden."
    },
};

static const Subrace HALFLING_SUBRACES[] = {
    {
        "lightfoot", "Leichtfuss",
        { 0, 0, 0, 0, 0, 1 },   // +1 CHA
        LIGHTFOOT_TRAITS,
        1
    },
    {
        "stout", "Staemmig",
        { 0, 0, 1, 0, 0, 0 },   // +1 CON
        STOUT_TRAITS,
        1
    },
};

// ============================================================================
// DRACHENBLÜTIGER (SRD 5.1) – keine Unterrassen, Drachenursprung ist Wahl
// ============================================================================
static const RacialTrait DRAGONBORN_TRAITS[] = {
    {
        "Drachenahnen-Resistenz",
        "Resistenz gegen den Schadenstyp deines Drachenerbes."
    },
    {
        "Atem-Waffe",
        "Benutze Aktion: kegel- oder linienfoermiger Angriff (Rettungswurf). 1/kurze Rast."
    },
};

// ============================================================================
// GNOM (SRD 5.1)
// ============================================================================
static const RacialTrait GNOME_TRAITS[] = {
    {
        "Dunkelsicht",
        "Siehst 60 Fuss weit in Dunkelheit."
    },
    {
        "Gnomische Gerissenheit",
        "Vorteil auf INT-, WEI- und CHA-Rettungswuerfe gegen Zauber."
    },
};

static const RacialTrait FOREST_GNOME_TRAITS[] = {
    {
        "Natuerlicher Illusionist",
        "Kennst den Zaubertrick Kleine Illusion (INT-basiert)."
    },
    {
        "Tiere sprechen",
        "Kommunizierst einfache Ideen mit kleinen Tieren."
    },
};
static const RacialTrait ROCK_GNOME_TRAITS[] = {
    {
        "Kunstfertigkeit",
        "Uebung mit Handwerker-Werkzeug deiner Wahl."
    },
    {
        "Techniker",
        "Kannst einfache Uhren und Mechanismen bauen (INT+Handwerk)."
    },
};

static const Subrace GNOME_SUBRACES[] = {
    {
        "forest_gnome", "Waldgnom",
        { 0, 1, 0, 0, 0, 0 },   // +1 DEX
        FOREST_GNOME_TRAITS,
        2
    },
    {
        "rock_gnome", "Felsengnom",
        { 0, 0, 1, 0, 0, 0 },   // +1 CON
        ROCK_GNOME_TRAITS,
        2
    },
};

// ============================================================================
// HALBELF (SRD 5.1) – Wahl: +1 auf 2 beliebige Attribute (kein STR+STR)
// Vereinfacht: vordefinierte +1 CHA (Halbelf-Basis) + Spieler wählt 2 weitere
// Im Builder wird das als Sonderfall behandelt (has_asi_choice=1)
// ============================================================================
static const RacialTrait HALF_ELF_TRAITS[] = {
    {
        "Dunkelsicht",
        "Siehst 60 Fuss weit in Dunkelheit."
    },
    {
        "Feenvorfahren",
        "Vorteil auf RW gegen Bezauberung. Immun gegen Magischen Schlaf."
    },
    {
        "Vielseitigkeit",
        "+2 CHA, +1 auf zwei weitere Attribute deiner Wahl (nicht CHA)."
    },
    {
        "Fertigkeiten",
        "Uebung in 2 Fertigkeiten deiner Wahl."
    },
};

// ============================================================================
// HALBORK (SRD 5.1)
// ============================================================================
static const RacialTrait HALF_ORC_TRAITS[] = {
    {
        "Dunkelsicht",
        "Siehst 60 Fuss weit in Dunkelheit."
    },
    {
        "Bedrohlich",
        "Uebung in der Fertigkeit Einstuechuern."
    },
    {
        "Unerbittliche Ausdauer",
        "1x pro langer Rast: Statt auf 0 HP zu fallen, bleibst du bei 1 HP."
    },
    {
        "Wilde Angriffe",
        "Bei krit. Treffer mit Nahkampfwaffe: +1 Schadenwuerfel."
    },
};

// ============================================================================
// TIEFLING (SRD 5.1)
// ============================================================================
static const RacialTrait TIEFLING_TRAITS[] = {
    {
        "Dunkelsicht",
        "Siehst 60 Fuss weit in Dunkelheit."
    },
    {
        "Hoellische Resistenz",
        "Resistenz gegen Feuerschaden."
    },
    {
        "Infernale Vermaechtnis",
        "Thaumaturgie (ZT), Hoellischer Tadel (Stufe 3), Dunkelheit (Stufe 5, 1/langer Rast)."
    },
};

// ============================================================================
// GLOBALE TABELLE
// ============================================================================
const SrdRace SRD_RACES[] = {
    {
        "aarakocra",
        "Aarakocra",
        "Vogelartige Wesen mit natuerlicher Flugfaehigkeit.",
        { 0, 2, 0, 0, 1, 0 },   // +2 DEX, +1 WIS
        25,
        "Mittel",
        "Gemein, Aarakocra",
        "50 Fuss Flug",
        AARAKOCRA_TRAITS,
        3,
        NULL,
        0
    },
    {
        "human",
        "Mensch",
        "Vielseitig und anpassungsfaehig, +1 auf alle Attribute.",
        { 1, 1, 1, 1, 1, 1 },   // +1 alle
        30,
        "Mittel",
        "Gemein + 1 Sprache nach Wahl",
        NULL,
        HUMAN_TRAITS,
        1,
        NULL,
        0
    },
    {
        "elf",
        "Elf",
        "Magisch begabtes, langlebiges Volk mit Unterrassen.",
        { 0, 2, 0, 0, 0, 0 },   // +2 DEX (Basis)
        30,
        "Mittel",
        "Gemein, Elfisch",
        NULL,
        ELF_TRAITS,
        4,
        ELF_SUBRACES,
        3
    },
    {
        "dwarf",
        "Zwerg",
        "Robustes Volk aus den Bergen, widerstandsfaehig gegen Gift.",
        { 0, 0, 2, 0, 0, 0 },   // +2 CON (Basis)
        25,
        "Mittel",
        "Gemein, Zwergisch",
        NULL,
        DWARF_TRAITS,
        5,
        DWARF_SUBRACES,
        2
    },
    {
        "halfling",
        "Halbling",
        "Kleines, glueckliches Volk das gerne zuhause bleibt.",
        { 0, 2, 0, 0, 0, 0 },   // +2 DEX (Basis)
        25,
        "Klein",
        "Gemein, Halblingisch",
        NULL,
        HALFLING_TRAITS,
        3,
        HALFLING_SUBRACES,
        2
    },
    {
        "dragonborn",
        "Drachenbluetiger",
        "Stolze Nachkommen von Drachen mit Atemwaffe und Resistenz.",
        { 2, 0, 0, 0, 0, 1 },   // +2 STR, +1 CHA
        30,
        "Mittel",
        "Gemein, Drakonisch",
        NULL,
        DRAGONBORN_TRAITS,
        2,
        NULL,
        0
    },
    {
        "gnome",
        "Gnom",
        "Erfindungsreiche, neugierige Wesen mit magischer Resistenz.",
        { 0, 0, 0, 2, 0, 0 },   // +2 INT (Basis)
        25,
        "Klein",
        "Gemein, Gnomisch",
        NULL,
        GNOME_TRAITS,
        2,
        GNOME_SUBRACES,
        2
    },
    {
        "half_elf",
        "Halbelf",
        "+2 CHA, +1 auf zwei beliebige Attribute, 2 Fertigkeiten.",
        { 0, 0, 0, 0, 0, 2 },   // +2 CHA (Rest: Spielerwahl – Sonderfall im Builder)
        30,
        "Mittel",
        "Gemein, Elfisch + 1 Sprache",
        NULL,
        HALF_ELF_TRAITS,
        4,
        NULL,
        0
    },
    {
        "half_orc",
        "Halbork",
        "Zaehe Kaempfer, unerbittlich im Angesicht des Todes.",
        { 2, 0, 1, 0, 0, 0 },   // +2 STR, +1 CON
        30,
        "Mittel",
        "Gemein, Orkisch",
        NULL,
        HALF_ORC_TRAITS,
        4,
        NULL,
        0
    },
    {
        "tiefling",
        "Tiefling",
        "Infernale Nachkommen mit Feuerwiderstand und innerer Magie.",
        { 0, 0, 0, 1, 0, 2 },   // +1 INT, +2 CHA
        30,
        "Mittel",
        "Gemein, Infernalisch",
        NULL,
        TIEFLING_TRAITS,
        3,
        NULL,
        0
    },
};

const int SRD_RACE_COUNT = sizeof(SRD_RACES) / sizeof(SRD_RACES[0]);

// ---- Hilfsfunktion: ASI-Zusammenfassung ------------------------------------

void srd_race_asi_summary(const SrdRace* r, const Subrace* sub,
                          char* buf, int buflen) {
    static const char* short_names[ABILITY_COUNT] = {
        "STR", "DEX", "CON", "INT", "WIS", "CHA"
    };

    // Kombiniere Basis-ASI und Unterrassen-ASI
    int total[ABILITY_COUNT];
    for (int i = 0; i < ABILITY_COUNT; i++) {
        total[i] = r->asi[i] + (sub ? sub->asi[i] : 0);
    }

    buf[0] = '\0';
    int written = 0;
    for (int i = 0; i < ABILITY_COUNT; i++) {
        if (total[i] != 0) {
            char tmp[16];
            snprintf(tmp, sizeof(tmp), "%+d %s ", total[i], short_names[i]);
            int len = (int)strlen(tmp);
            if (written + len < buflen - 1) {
                strncat(buf, tmp, buflen - written - 1);
                written += len;
            }
        }
    }

    // Sonderfall: Halbelf hat waehlbare ASI (+2 CHA + 2x +1 nach Wahl)
    if (strcmp(r->id, "half_elf") == 0) {
        const char* suffix = "+1+1 frei";
        if (written + (int)strlen(suffix) < buflen - 1)
            strncat(buf, suffix, buflen - written - 1);
    }

    // Trailing space entfernen
    int len = (int)strlen(buf);
    if (len > 0 && buf[len - 1] == ' ')
        buf[len - 1] = '\0';
}
