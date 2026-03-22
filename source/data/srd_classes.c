#include "srd_classes.h"
#include <stddef.h>

// ============================================================================
// BARDE (D&D 2024) – vollstaendig implementiert
// ============================================================================
static const ClassFeature BARD_FEATURES[] = {
    {
        "Zauberwirken",
        "Wirke Zauber mit CHA. 2 Zaubertricks, 4 bekannte Zauber, 2 Grad-1-Plaetze."
    },
    {
        "Bardische Inspiration (W6)",
        "Bonus-Aktion: Gib einem Wesen in 18m 1W6 Inspiration. Anzahl = CHA-Mod. Neu nach langer Rast."
    },
};

// Barde kann aus ALLEN Fertigkeiten 3 waehlen (any_skill = 1)

// Option A: Lederruestung + 2 Dolche + Laute + Unterhaltungs-Gepaeck + 19 GM (D&D 2024)
static const StartItem BARD_ITEMS_A[] = {
    { "Lederruestung",       1, 10.0f, 1000, ITEM_CAT_ARMOR,  1 },
    { "Dolch",               2, 1.0f,  200,  ITEM_CAT_WEAPON, 1 },
    { "Laute",               1, 2.0f,  350,  ITEM_CAT_TOOL,   0 },
    { "Unterhalt.-Gepaeck",  1, 0.0f,  0,    ITEM_CAT_OTHER,  0 },
};
// Option B: 90 GM (D&D 2024)
static const StartItem BARD_ITEMS_B[] = {
    { "Startgold (90 GM)",   1, 0.0f,  9000, ITEM_CAT_OTHER,  0 },
};

// ============================================================================
// KAEMPFER (SRD 5.1) – vollstaendig fuer Stufe 1
// ============================================================================
// SRD Kaempfer-Skills: Acrobatics, Animal Handling, Athletics, History,
// Insight, Intimidation, Perception, Survival
static const int FIGHTER_SKILL_OPTS[] = {
    0,   // Acrobatics
    1,   // Animal Handling
    3,   // Athletics
    5,   // History
    6,   // Insight
    7,   // Intimidation
    11,  // Perception
    17,  // Survival
};

static const ClassFeature FIGHTER_FEATURES[] = {
    {
        "Kampfstil",
        "Waehle einen Kampfstil: Bogenschiessen, Duell, Grosswaffe, Schutz, etc."
    },
    {
        "Zweiter Wind",
        "Bonus-Aktion: Erhole 1W10 + Stufe HP. Neu nach kurzer oder langer Rast."
    },
};

static const StartItem FIGHTER_ITEMS_A[] = {
    { "Kettenhemd",     1, 55.0f, 7500, ITEM_CAT_ARMOR,  1 },
    { "Langschwert",    1, 3.0f,  1500, ITEM_CAT_WEAPON, 1 },
    { "Schild",         1, 6.0f,  1000, ITEM_CAT_ARMOR,  1 },
    { "Abenteurer-Rucksack", 1, 0.0f, 0, ITEM_CAT_OTHER, 0 },
};
static const StartItem FIGHTER_ITEMS_B[] = {
    { "Lederruestung",  1, 10.0f, 1000, ITEM_CAT_ARMOR,  1 },
    { "Handaxt",        2, 2.0f,  500,  ITEM_CAT_WEAPON, 1 },
    { "Abenteurer-Rucksack", 1, 0.0f, 0, ITEM_CAT_OTHER, 0 },
    { "Wurfmesser",     4, 0.5f,  200,  ITEM_CAT_WEAPON, 0 },
};

// ============================================================================
// ZAUBERER (Wizard, SRD 5.1)
// ============================================================================
static const int WIZARD_SKILL_OPTS[] = {
    2,   // Arcana
    5,   // History
    8,   // Investigation
    9,   // Medicine
    10,  // Nature
    14,  // Religion
};

static const ClassFeature WIZARD_FEATURES[] = {
    {
        "Zauberwirken",
        "Wirke Zauber mit INT. 3 Zaubertricks, Zauberbuch mit 6 Grad-1-Zaubern."
    },
    {
        "Arkane Wiederherstellung",
        "1x/Tag (kurze Rast): Erhole Zauberplaetze bis halber Stufe (aufgerundet)."
    },
};

static const StartItem WIZARD_ITEMS_A[] = {
    { "Stab",           1, 4.0f,  500,  ITEM_CAT_WEAPON, 1 },
    { "Zauberbuch",     1, 3.0f,  0,    ITEM_CAT_OTHER,  0 },
    { "Gelehrten-Gepaeck", 1, 0.0f, 0,  ITEM_CAT_OTHER,  0 },
};
static const StartItem WIZARD_ITEMS_B[] = {
    { "Dolch",          1, 1.0f,  200,  ITEM_CAT_WEAPON, 1 },
    { "Zauberbuch",     1, 3.0f,  0,    ITEM_CAT_OTHER,  0 },
    { "Erkunder-Gepaeck",1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};

// ============================================================================
// SCHURKE (SRD 5.1)
// ============================================================================
static const int ROGUE_SKILL_OPTS[] = {
    0,   // Acrobatics
    4,   // Deception
    5,   // History (Athletics)
    3,   // Athletics
    6,   // Insight
    7,   // Intimidation
    8,   // Investigation
    11,  // Perception
    12,  // Performance
    13,  // Persuasion
    15,  // Sleight of Hand
    16,  // Stealth
};

static const ClassFeature ROGUE_FEATURES[] = {
    {
        "Expertise",
        "Waehle 2 Fertigkeiten (oder 1 Fertigkeit + Diebeswerkzeug): doppelter Uebungsbonus."
    },
    {
        "Heimtueckischer Angriff",
        "1x/Runde +1W6 Schaden bei Angriff mit Vorteil oder verbündetem Nachbarn."
    },
    {
        "Diebesgaunersprache",
        "Kennst die Geheimsprache der Diebe."
    },
};

static const StartItem ROGUE_ITEMS_A[] = {
    { "Rapier",         1, 2.0f,  2500, ITEM_CAT_WEAPON, 1 },
    { "Lederruestung",  1, 10.0f, 1000, ITEM_CAT_ARMOR,  1 },
    { "Diebeswerkzeug", 1, 1.0f,  250,  ITEM_CAT_TOOL,   0 },
    { "Einbrecher-Gepaeck", 1, 0.0f, 0, ITEM_CAT_OTHER,  0 },
};
static const StartItem ROGUE_ITEMS_B[] = {
    { "Kurzschwert",    1, 2.0f,  1000, ITEM_CAT_WEAPON, 1 },
    { "Kurzschwert",    1, 2.0f,  1000, ITEM_CAT_WEAPON, 0 },
    { "Lederruestung",  1, 10.0f, 1000, ITEM_CAT_ARMOR,  1 },
    { "Diebeswerkzeug", 1, 1.0f,  250,  ITEM_CAT_TOOL,   0 },
};

// ============================================================================
// KLERIKER (SRD 5.1)
// ============================================================================
static const int CLERIC_SKILL_OPTS[] = {
    5,   // History
    6,   // Insight
    9,   // Medicine
    13,  // Persuasion
    14,  // Religion
};

static const ClassFeature CLERIC_FEATURES[] = {
    {
        "Zauberwirken",
        "Wirke Zauber mit WIS. Bereite Level+WIS Zauber vor. 2 Zaubertricks."
    },
    {
        "Goettliche Domaene",
        "Waehle eine Domaene (Leben, Licht, Wissen, Betrug etc.) fuer Bonus-Features."
    },
};

static const StartItem CLERIC_ITEMS_A[] = {
    { "Streitkolben",   1, 4.0f,  500,  ITEM_CAT_WEAPON, 1 },
    { "Schuppenhemd",   1, 45.0f, 5000, ITEM_CAT_ARMOR,  1 },
    { "Heiliges Symbol",1, 1.0f,  500,  ITEM_CAT_OTHER,  1 },
    { "Priester-Gepaeck",1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};
static const StartItem CLERIC_ITEMS_B[] = {
    { "Streitkolben",   1, 4.0f,  500,  ITEM_CAT_WEAPON, 1 },
    { "Lederruestung",  1, 10.0f, 1000, ITEM_CAT_ARMOR,  1 },
    { "Heiliges Symbol",1, 1.0f,  500,  ITEM_CAT_OTHER,  1 },
    { "Entdecker-Gepaeck",1, 0.0f,0,    ITEM_CAT_OTHER,  0 },
};

// ============================================================================
// BARBAR (SRD 5.1)
// ============================================================================
static const int BARB_SKILL_OPTS[] = {
    1,   // Animal Handling
    3,   // Athletics
    7,   // Intimidation
    10,  // Nature
    11,  // Perception
    17,  // Survival
};

static const ClassFeature BARBAR_FEATURES[] = {
    {
        "Raserei",
        "Bonus-Aktion: Raserei fuer 1 Minute. +2 STR-Schaden, Resistenz gegen physischen Schaden."
    },
    {
        "Ungebundene Bewegung",
        "Kann Ruestung nicht tragen fuer AC = 10 + DEX-Mod + CON-Mod. Keine schwere Ruestung."
    },
};

static const StartItem BARBAR_ITEMS_A[] = {
    { "Grossaxt",       1, 7.0f,  3000, ITEM_CAT_WEAPON, 1 },
    { "Wurfmesser",     4, 0.5f,  200,  ITEM_CAT_WEAPON, 0 },
    { "Erkunder-Gepaeck",1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};
static const StartItem BARBAR_ITEMS_B[] = {
    { "Handaxt",        2, 2.0f,  500,  ITEM_CAT_WEAPON, 1 },
    { "Wurfmesser",     4, 0.5f,  200,  ITEM_CAT_WEAPON, 0 },
    { "Erkunder-Gepaeck",1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};

// ============================================================================
// DRUIDE (SRD 5.1)
// ============================================================================
static const int DRUID_SKILL_OPTS[] = {
    1,   // Animal Handling
    2,   // Arcana
    6,   // Insight
    9,   // Medicine
    10,  // Nature
    11,  // Perception
    14,  // Religion
    17,  // Survival
};

static const ClassFeature DRUID_FEATURES[] = {
    {
        "Druidische Sprache",
        "Kennst Druidisch, die Geheimsprache der Druiden."
    },
    {
        "Zauberwirken",
        "Wirke Zauber mit WIS. Bereite Level+WIS Zauber vor. 2 Zaubertricks."
    },
};

static const StartItem DRUID_ITEMS_A[] = {
    { "Schild",         1, 6.0f,  1000, ITEM_CAT_ARMOR,  1 },
    { "Druidenfokus",   1, 1.0f,  0,    ITEM_CAT_OTHER,  0 },
    { "Erkunder-Gepaeck",1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};
static const StartItem DRUID_ITEMS_B[] = {
    { "Streitkolben",   1, 4.0f,  500,  ITEM_CAT_WEAPON, 1 },
    { "Lederruestung",  1, 10.0f, 1000, ITEM_CAT_ARMOR,  1 },
    { "Druidenfokus",   1, 1.0f,  0,    ITEM_CAT_OTHER,  0 },
};

// ============================================================================
// MOENCHE (SRD 5.1)
// ============================================================================
static const int MONK_SKILL_OPTS[] = {
    0,   // Acrobatics
    5,   // History
    6,   // Insight
    14,  // Religion
    16,  // Stealth
    3,   // Athletics
};

static const ClassFeature MONK_FEATURES[] = {
    {
        "Unarmierter Kampf",
        "Schaden: 1W4 unbewaffnet. Bonus-Aktion: zweiter Angriff oder Greifer."
    },
    {
        "Ki (1 Punkt)",
        "Flurry of Blows, Patient Defense oder Step of the Wind. Neu nach kurzer Rast."
    },
    {
        "Ungebundene Bewegung",
        "Keine Ruestung noetig: AC = 10 + DEX-Mod + WIS-Mod."
    },
};

static const StartItem MONK_ITEMS_A[] = {
    { "Kurzschwert",    1, 2.0f,  1000, ITEM_CAT_WEAPON, 1 },
    { "Dungeon-Gepaeck",1, 0.0f,  0,    ITEM_CAT_OTHER,  0 },
    { "10 Wurfpfeile",  1, 1.0f,  100,  ITEM_CAT_WEAPON, 0 },
};
static const StartItem MONK_ITEMS_B[] = {
    { "Einfache Waffe", 1, 2.0f,  200,  ITEM_CAT_WEAPON, 1 },
    { "Erkunder-Gepaeck",1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
    { "10 Wurfpfeile",  1, 1.0f,  100,  ITEM_CAT_WEAPON, 0 },
};

// ============================================================================
// PALADIN (SRD 5.1)
// ============================================================================
static const int PALADIN_SKILL_OPTS[] = {
    3,   // Athletics
    6,   // Insight
    7,   // Intimidation
    9,   // Medicine
    13,  // Persuasion
    14,  // Religion
};

static const ClassFeature PALADIN_FEATURES[] = {
    {
        "Goettliche Wahrnehmung",
        "1 Stunde spaeter: Bonus-Aktion, senses evil/good/undead/consecrated (60 Fuss)."
    },
    {
        "Handauflegung",
        "Heile Gesamtpool = Level*5 HP/Tag. 5 HP = Krankheit oder Gift heilen."
    },
};

static const StartItem PALADIN_ITEMS_A[] = {
    { "Kriegshammer",   1, 2.0f,  1500, ITEM_CAT_WEAPON, 1 },
    { "Kettenhemd",     1, 55.0f, 7500, ITEM_CAT_ARMOR,  1 },
    { "Schild",         1, 6.0f,  1000, ITEM_CAT_ARMOR,  1 },
    { "Heiliges Symbol",1, 1.0f,  500,  ITEM_CAT_OTHER,  1 },
};
static const StartItem PALADIN_ITEMS_B[] = {
    { "Langschwert",    1, 3.0f,  1500, ITEM_CAT_WEAPON, 1 },
    { "Schuppenhemd",   1, 45.0f, 5000, ITEM_CAT_ARMOR,  1 },
    { "Heiliges Symbol",1, 1.0f,  500,  ITEM_CAT_OTHER,  1 },
    { "Priester-Gepaeck",1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};

// ============================================================================
// WALDLAEUFER (Ranger, SRD 5.1)
// ============================================================================
static const int RANGER_SKILL_OPTS[] = {
    1,   // Animal Handling
    3,   // Athletics
    6,   // Insight
    8,   // Investigation
    10,  // Nature
    11,  // Perception
    16,  // Stealth
    17,  // Survival
};

static const ClassFeature RANGER_FEATURES[] = {
    {
        "Bevorzugter Feind",
        "Waehle Feindtyp: Vorteil auf Ueberleben/Wissen ueber diesen Typ."
    },
    {
        "Natuerlicher Erkundungsraum",
        "Waehle Terrain: Gruppe verfaehrt sich nicht, Gruppen-Schrittlaerm gedaempft."
    },
};

static const StartItem RANGER_ITEMS_A[] = {
    { "Schuppenhemd",   1, 45.0f, 5000, ITEM_CAT_ARMOR,  1 },
    { "Kurzschwert",    2, 2.0f,  1000, ITEM_CAT_WEAPON, 1 },
    { "Dungeon-Gepaeck",1, 0.0f,  0,    ITEM_CAT_OTHER,  0 },
};
static const StartItem RANGER_ITEMS_B[] = {
    { "Lederruestung",  1, 10.0f, 1000, ITEM_CAT_ARMOR,  1 },
    { "Langbogen",      1, 2.0f,  7500, ITEM_CAT_WEAPON, 1 },
    { "20 Pfeile",      1, 1.0f,  100,  ITEM_CAT_WEAPON, 0 },
    { "Erkunder-Gepaeck",1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};

// ============================================================================
// HEXENMEISTER (Warlock, SRD 5.1)
// ============================================================================
static const int WARLOCK_SKILL_OPTS[] = {
    2,   // Arcana
    4,   // Deception
    5,   // History
    7,   // Intimidation
    10,  // Nature
    14,  // Religion
};

static const ClassFeature WARLOCK_FEATURES[] = {
    {
        "Pakt-Magie (Overchanneled)",
        "Wirke Zauber mit CHA. 1 Paktplatz (Grad 1), Neu nach kurzer oder langer Rast."
    },
    {
        "Eldritch Blast (Zaubertrick)",
        "1W10 Kraftschaden, Reichweite 120 Fuss. Skaliert bei Stufe 5/11/17."
    },
};

static const StartItem WARLOCK_ITEMS_A[] = {
    { "Leichte Armbruest",1, 5.0f, 2500, ITEM_CAT_WEAPON, 1 },
    { "20 Bolzen",        1, 1.5f, 100,  ITEM_CAT_WEAPON, 0 },
    { "Zauberfokus",      1, 1.0f, 0,    ITEM_CAT_OTHER,  0 },
    { "Dungeon-Gepaeck",  1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};
static const StartItem WARLOCK_ITEMS_B[] = {
    { "Einfache Waffe",   1, 2.0f, 200,  ITEM_CAT_WEAPON, 1 },
    { "Zauberfokus",      1, 1.0f, 0,    ITEM_CAT_OTHER,  0 },
    { "Erkunder-Gepaeck", 1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};

// ============================================================================
// ZAUBERWIRKER (Sorcerer, SRD 5.1)
// ============================================================================
static const int SORC_SKILL_OPTS[] = {
    2,   // Arcana
    4,   // Deception
    6,   // Insight
    7,   // Intimidation
    13,  // Persuasion
    14,  // Religion
};

static const ClassFeature SORCERER_FEATURES[] = {
    {
        "Zauberwirken",
        "Wirke Zauber mit CHA. 4 Zaubertricks, 2 bekannte Grad-1-Zauber, 2 Plaetze."
    },
    {
        "Zauberursprung",
        "Waehle Drachenahnlinie oder Wilde Magie: gibt Bonus-Features."
    },
};

static const StartItem SORC_ITEMS_A[] = {
    { "Leichte Armbruest",1, 5.0f, 2500, ITEM_CAT_WEAPON, 1 },
    { "20 Bolzen",        1, 1.5f, 100,  ITEM_CAT_WEAPON, 0 },
    { "Zauberfokus",      1, 1.0f, 0,    ITEM_CAT_OTHER,  0 },
    { "Dungeon-Gepaeck",  1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};
static const StartItem SORC_ITEMS_B[] = {
    { "Dolch",            2, 1.0f, 200,  ITEM_CAT_WEAPON, 1 },
    { "Zauberfokus",      1, 1.0f, 0,    ITEM_CAT_OTHER,  0 },
    { "Erkunder-Gepaeck", 1, 0.0f, 0,    ITEM_CAT_OTHER,  0 },
};

// ============================================================================
// GLOBALE KLASSEN-TABELLE
// ============================================================================
const SrdClass SRD_CLASSES[] = {
    // --- BARDE ---------------------------------------------------------------
    {
        "bard", "Barde",
        "Kuenstler und Magie-Wirker, inspiriert Alliierte.",
        8,                      // d8
        ABILITY_CHA,
        { ABILITY_DEX, ABILITY_CHA },
        3,                      // 3 Fertigkeiten
        1,                      // aus allen Skills
        NULL, 0,
        "Leichte Ruestung",
        "Einfache Waffen",
        BARD_FEATURES, 2,
        1, ABILITY_CHA,
        2, 4, 2,                // 2 Zaubertricks, 4 Zauber, 2 Grad-1-Plaetze
        { "Lederruestung + 2 Dolche + Laute + Unterhalt.-Gepaeck",
          BARD_ITEMS_A, 4 },
        { "90 Goldmuenzen",
          BARD_ITEMS_B, 1 },
    },
    // --- KAEMPFER ------------------------------------------------------------
    {
        "fighter", "Kaempfer",
        "Vielseitiger Krieger mit ueberlegenem Kampfstil.",
        10,                     // d10
        ABILITY_STR,
        { ABILITY_STR, ABILITY_CON },
        2,
        0,
        FIGHTER_SKILL_OPTS, 8,
        "Alle Ruestungen, Schilde",
        "Einfache und kriegerische Waffen",
        FIGHTER_FEATURES, 2,
        0, -1, 0, 0, 0,
        { "Kettenhemd + Langschwert + Schild + Rucksack",
          FIGHTER_ITEMS_A, 4 },
        { "Lederruestung + 2 Handaexte + Rucksack + 4 Wurfmesser",
          FIGHTER_ITEMS_B, 4 },
    },
    // --- ZAUBERER ------------------------------------------------------------
    {
        "wizard", "Magier",
        "Meister der arkanen Magie, maechtiger Zauberwirker.",
        6,                      // d6
        ABILITY_INT,
        { ABILITY_INT, ABILITY_WIS },
        2,
        0,
        WIZARD_SKILL_OPTS, 6,
        "Keine",
        "Dolche, Pfeile, Wurfpfeile, Stab, Leichte Armbrust",
        WIZARD_FEATURES, 2,
        1, ABILITY_INT,
        3, 6, 2,                // 3 ZT, 6 Zauberbuchzauber, 2 Grad-1-Plaetze
        { "Stab + Zauberbuch + Gelehrten-Gepaeck",
          WIZARD_ITEMS_A, 3 },
        { "Dolch + Zauberbuch + Erkunder-Gepaeck",
          WIZARD_ITEMS_B, 3 },
    },
    // --- SCHURKE -------------------------------------------------------------
    {
        "rogue", "Schurke",
        "Heimtueckischer Kaempfer mit Expertise und Heimtueckischem Angriff.",
        8,                      // d8
        ABILITY_DEX,
        { ABILITY_DEX, ABILITY_INT },
        4,                      // 4 Fertigkeiten!
        0,
        ROGUE_SKILL_OPTS, 12,
        "Leichte Ruestung",
        "Einfache Waffen, Handarmbrust, Langschwert, Rapier, Kurzschwert",
        ROGUE_FEATURES, 3,
        0, -1, 0, 0, 0,
        { "Rapier + Lederruestung + Diebeswerkzeug + Einbrecher-Gepaeck",
          ROGUE_ITEMS_A, 4 },
        { "2 Kurzschwerter + Lederruestung + Diebeswerkzeug",
          ROGUE_ITEMS_B, 4 },
    },
    // --- KLERIKER ------------------------------------------------------------
    {
        "cleric", "Kleriker",
        "Gottesdiener mit Heilungsmagie und goettlicher Macht.",
        8,                      // d8
        ABILITY_WIS,
        { ABILITY_WIS, ABILITY_CHA },
        2,
        0,
        CLERIC_SKILL_OPTS, 5,
        "Leichte und mittlere Ruestung, Schilde",
        "Einfache Waffen",
        CLERIC_FEATURES, 2,
        1, ABILITY_WIS,
        3, 0, 2,                // 3 ZT, vorbereitete Zauber (nicht 'bekannt'), 2 Plaetze
        { "Streitkolben + Schuppenhemd + Heiliges Symbol + Priester-Gepaeck",
          CLERIC_ITEMS_A, 4 },
        { "Streitkolben + Lederruestung + Heiliges Symbol + Entdecker-Gepaeck",
          CLERIC_ITEMS_B, 4 },
    },
    // --- BARBAR --------------------------------------------------------------
    {
        "barbarian", "Barbar",
        "Wilder Kaempfer, der Schmerz ignoriert und rasend angreift.",
        12,                     // d12
        ABILITY_STR,
        { ABILITY_STR, ABILITY_CON },
        2,
        0,
        BARB_SKILL_OPTS, 6,
        "Leichte und mittlere Ruestung, Schilde (keine schwere)",
        "Einfache und kriegerische Waffen",
        BARBAR_FEATURES, 2,
        0, -1, 0, 0, 0,
        { "Grossaxt + 4 Wurfmesser + Erkunder-Gepaeck",
          BARBAR_ITEMS_A, 3 },
        { "2 Handaexte + 4 Wurfmesser + Erkunder-Gepaeck",
          BARBAR_ITEMS_B, 3 },
    },
    // --- DRUIDE --------------------------------------------------------------
    {
        "druid", "Druide",
        "Naturreligioener Zauberwirker, kann sich in Tiere verwandeln.",
        8,                      // d8
        ABILITY_WIS,
        { ABILITY_INT, ABILITY_WIS },
        2,
        0,
        DRUID_SKILL_OPTS, 8,
        "Leichte und mittlere Ruestung (kein Metall), Schilde",
        "Keulen, Dolche, Wurfpfeile, Speere, Stab, Sichel, Sling, Speer",
        DRUID_FEATURES, 2,
        1, ABILITY_WIS,
        2, 0, 2,                // 2 ZT, vorbereitete Zauber, 2 Grad-1-Plaetze
        { "Schild + Druidenfokus + Erkunder-Gepaeck",
          DRUID_ITEMS_A, 3 },
        { "Streitkolben + Lederruestung + Druidenfokus",
          DRUID_ITEMS_B, 3 },
    },
    // --- MOENCHE -------------------------------------------------------------
    {
        "monk", "Moenche",
        "Kampfkuenstler, der Ki-Energie nutzt.",
        8,                      // d8
        ABILITY_DEX,
        { ABILITY_STR, ABILITY_DEX },
        2,
        0,
        MONK_SKILL_OPTS, 6,
        "Keine (ungebundene Bewegung: AC = 10+DEX+WIS)",
        "Einfache Waffen, Kurzschwert",
        MONK_FEATURES, 3,
        0, -1, 0, 0, 0,
        { "Kurzschwert + Dungeon-Gepaeck + 10 Wurfpfeile",
          MONK_ITEMS_A, 3 },
        { "Einfache Waffe + Erkunder-Gepaeck + 10 Wurfpfeile",
          MONK_ITEMS_B, 3 },
    },
    // --- PALADIN -------------------------------------------------------------
    {
        "paladin", "Paladin",
        "Heiliger Krieger mit Eiden und Heilungshandauflegung.",
        10,                     // d10
        ABILITY_STR,
        { ABILITY_WIS, ABILITY_CHA },
        2,
        0,
        PALADIN_SKILL_OPTS, 6,
        "Alle Ruestungen, Schilde",
        "Einfache und kriegerische Waffen",
        PALADIN_FEATURES, 2,
        1, ABILITY_CHA,
        0, 0, 2,                // Kein ZT, keine bekannten Zauber, 2 Grad-1-Plaetze (ab Stufe 2 eigentlich)
        { "Kriegshammer + Kettenhemd + Schild + Heiliges Symbol",
          PALADIN_ITEMS_A, 4 },
        { "Langschwert + Schuppenhemd + Heiliges Symbol + Priester-Gepaeck",
          PALADIN_ITEMS_B, 4 },
    },
    // --- WALDLAEUFER ---------------------------------------------------------
    {
        "ranger", "Waldlaeufer",
        "Wildnisexperte mit Spurenverfolgung und Tierfreundschaft.",
        10,                     // d10
        ABILITY_DEX,
        { ABILITY_STR, ABILITY_DEX },
        3,
        0,
        RANGER_SKILL_OPTS, 8,
        "Leichte und mittlere Ruestung, Schilde",
        "Einfache und kriegerische Waffen",
        RANGER_FEATURES, 2,
        1, ABILITY_WIS,
        0, 0, 2,                // Zauber ab Stufe 2 (hier 0 bekannte Zauber bei Stufe 1)
        { "Schuppenhemd + 2 Kurzschwerter + Dungeon-Gepaeck",
          RANGER_ITEMS_A, 3 },
        { "Lederruestung + Langbogen + 20 Pfeile + Erkunder-Gepaeck",
          RANGER_ITEMS_B, 4 },
    },
    // --- HEXENMEISTER --------------------------------------------------------
    {
        "warlock", "Hexenmeister",
        "Durch einen Pakt mit einer maechtigen Wesenheit begabter Wirker.",
        8,                      // d8
        ABILITY_CHA,
        { ABILITY_WIS, ABILITY_CHA },
        2,
        0,
        WARLOCK_SKILL_OPTS, 6,
        "Leichte Ruestung",
        "Einfache Waffen",
        WARLOCK_FEATURES, 2,
        1, ABILITY_CHA,
        2, 2, 1,                // 2 ZT, 2 bekannte Zauber, 1 Paktplatz
        { "Leichte Armbruest + 20 Bolzen + Zauberfokus + Dungeon-Gepaeck",
          WARLOCK_ITEMS_A, 4 },
        { "Einfache Waffe + Zauberfokus + Erkunder-Gepaeck",
          WARLOCK_ITEMS_B, 3 },
    },
    // --- ZAUBERWIRKER (SORCERER) ---------------------------------------------
    {
        "sorcerer", "Zauberwirker",
        "Geborener Magiekanal mit metamagischen Faehigkeiten.",
        6,                      // d6
        ABILITY_CHA,
        { ABILITY_CON, ABILITY_CHA },
        2,
        0,
        SORC_SKILL_OPTS, 6,
        "Keine",
        "Dolche, Pfeile, Wurfpfeile, Stab, Leichte Armbrust",
        SORCERER_FEATURES, 2,
        1, ABILITY_CHA,
        4, 2, 2,                // 4 ZT, 2 bekannte Zauber, 2 Grad-1-Plaetze
        { "Leichte Armbruest + 20 Bolzen + Zauberfokus + Dungeon-Gepaeck",
          SORC_ITEMS_A, 4 },
        { "2 Dolche + Zauberfokus + Erkunder-Gepaeck",
          SORC_ITEMS_B, 3 },
    },
};

const int SRD_CLASS_COUNT = sizeof(SRD_CLASSES) / sizeof(SRD_CLASSES[0]);
