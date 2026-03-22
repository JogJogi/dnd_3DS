#include "srd_progression.h"
#include <string.h>
#include <stddef.h>

// Makro fuer leere Featliste
#define NO_FEATS NULL, 0

// ============================================================================
// ZAUBERSCHLITZ-TABELLEN
// ============================================================================
// Vollstaendige D&D 5e SRD Tabellen fuer Level 1-20 (Index 0 = Level 1)

// Volle Zauberwirker (Barde, Kleriker, Druide, Magier, Zauberwirker)
static const int SLOTS_FULL[PROG_LEVELS][PROG_SLOT_LEVELS] = {
//  L1  L2  L3  L4  L5  L6  L7  L8  L9
    { 2,  0,  0,  0,  0,  0,  0,  0,  0 },  // 1
    { 3,  0,  0,  0,  0,  0,  0,  0,  0 },  // 2
    { 4,  2,  0,  0,  0,  0,  0,  0,  0 },  // 3
    { 4,  3,  0,  0,  0,  0,  0,  0,  0 },  // 4
    { 4,  3,  2,  0,  0,  0,  0,  0,  0 },  // 5
    { 4,  3,  3,  0,  0,  0,  0,  0,  0 },  // 6
    { 4,  3,  3,  1,  0,  0,  0,  0,  0 },  // 7
    { 4,  3,  3,  2,  0,  0,  0,  0,  0 },  // 8
    { 4,  3,  3,  3,  1,  0,  0,  0,  0 },  // 9
    { 4,  3,  3,  3,  2,  0,  0,  0,  0 },  // 10
    { 4,  3,  3,  3,  2,  1,  0,  0,  0 },  // 11
    { 4,  3,  3,  3,  2,  1,  0,  0,  0 },  // 12
    { 4,  3,  3,  3,  2,  1,  1,  0,  0 },  // 13
    { 4,  3,  3,  3,  2,  1,  1,  0,  0 },  // 14
    { 4,  3,  3,  3,  2,  1,  1,  1,  0 },  // 15
    { 4,  3,  3,  3,  2,  1,  1,  1,  0 },  // 16
    { 4,  3,  3,  3,  2,  1,  1,  1,  1 },  // 17
    { 4,  3,  3,  3,  3,  1,  1,  1,  1 },  // 18
    { 4,  3,  3,  3,  3,  2,  1,  1,  1 },  // 19
    { 4,  3,  3,  3,  3,  2,  2,  1,  1 },  // 20
};

// Halbe Zauberwirker (Paladin, Waldlaeufer) – keine Zauber bei Stufe 1
static const int SLOTS_HALF[PROG_LEVELS][PROG_SLOT_LEVELS] = {
//  L1  L2  L3  L4  L5  L6  L7  L8  L9
    { 0,  0,  0,  0,  0,  0,  0,  0,  0 },  // 1
    { 2,  0,  0,  0,  0,  0,  0,  0,  0 },  // 2
    { 3,  0,  0,  0,  0,  0,  0,  0,  0 },  // 3
    { 3,  0,  0,  0,  0,  0,  0,  0,  0 },  // 4
    { 4,  2,  0,  0,  0,  0,  0,  0,  0 },  // 5
    { 4,  2,  0,  0,  0,  0,  0,  0,  0 },  // 6
    { 4,  3,  0,  0,  0,  0,  0,  0,  0 },  // 7
    { 4,  3,  0,  0,  0,  0,  0,  0,  0 },  // 8
    { 4,  3,  2,  0,  0,  0,  0,  0,  0 },  // 9
    { 4,  3,  2,  0,  0,  0,  0,  0,  0 },  // 10
    { 4,  3,  3,  0,  0,  0,  0,  0,  0 },  // 11
    { 4,  3,  3,  0,  0,  0,  0,  0,  0 },  // 12
    { 4,  3,  3,  1,  0,  0,  0,  0,  0 },  // 13
    { 4,  3,  3,  1,  0,  0,  0,  0,  0 },  // 14
    { 4,  3,  3,  2,  0,  0,  0,  0,  0 },  // 15
    { 4,  3,  3,  2,  0,  0,  0,  0,  0 },  // 16
    { 4,  3,  3,  3,  1,  0,  0,  0,  0 },  // 17
    { 4,  3,  3,  3,  1,  0,  0,  0,  0 },  // 18
    { 4,  3,  3,  3,  2,  0,  0,  0,  0 },  // 19
    { 4,  3,  3,  3,  2,  0,  0,  0,  0 },  // 20
};

// Hexenmeister: Pakt-Magie (Slots erholen nach kurzer Rast)
// slots[0] = Anzahl Pakt-Plaetze, slots[1] = Zauber-Grad der Pakt-Plaetze
static const int SLOTS_WARLOCK[PROG_LEVELS][PROG_SLOT_LEVELS] = {
//  S   Grad L3  L4  L5  L6  L7  L8  L9  (L3-L9 ungenutzt bei Warlock)
    { 1,  0,  0,  0,  0,  0,  0,  0,  0 },  // 1 (1 Grad-1-Platz)
    { 2,  0,  0,  0,  0,  0,  0,  0,  0 },  // 2 (2 Grad-1-Plaetze)
    { 0,  2,  0,  0,  0,  0,  0,  0,  0 },  // 3 (2 Grad-2-Plaetze)
    { 0,  2,  0,  0,  0,  0,  0,  0,  0 },  // 4
    { 0,  0,  2,  0,  0,  0,  0,  0,  0 },  // 5 (2 Grad-3-Plaetze)
    { 0,  0,  2,  0,  0,  0,  0,  0,  0 },  // 6
    { 0,  0,  0,  2,  0,  0,  0,  0,  0 },  // 7 (2 Grad-4-Plaetze)
    { 0,  0,  0,  2,  0,  0,  0,  0,  0 },  // 8
    { 0,  0,  0,  0,  2,  0,  0,  0,  0 },  // 9 (2 Grad-5-Plaetze)
    { 0,  0,  0,  0,  2,  0,  0,  0,  0 },  // 10
    { 0,  0,  0,  0,  3,  0,  0,  0,  0 },  // 11 (3 Grad-5-Plaetze)
    { 0,  0,  0,  0,  3,  0,  0,  0,  0 },  // 12
    { 0,  0,  0,  0,  3,  0,  0,  0,  0 },  // 13
    { 0,  0,  0,  0,  3,  0,  0,  0,  0 },  // 14
    { 0,  0,  0,  0,  3,  0,  0,  0,  0 },  // 15
    { 0,  0,  0,  0,  3,  0,  0,  0,  0 },  // 16
    { 0,  0,  0,  0,  4,  0,  0,  0,  0 },  // 17 (4 Grad-5-Plaetze)
    { 0,  0,  0,  0,  4,  0,  0,  0,  0 },  // 18
    { 0,  0,  0,  0,  4,  0,  0,  0,  0 },  // 19
    { 0,  0,  0,  0,  4,  0,  0,  0,  0 },  // 20
};

// Keine Zauberschlitze (Barbar, Kaempfer, Moenche, Schurke)
static const int SLOTS_NONE[PROG_LEVELS][PROG_SLOT_LEVELS] = {{ 0 }};

// ============================================================================
// FEATURES PER KLASSE PER STUFE (Stufen 2-10, Stufe 1 = Charaktererstellung)
// ============================================================================

// ---- BARDE ------------------------------------------------------------------
static const ProgFeat BARD_L2[] = {
    { "Alleskoenner",          "Halbierter Uebungsbonus auf alle Fertigkeitswuerfe ohne Uebung.", 0, "" },
    { "Lied der Erholung (W6)", "Verbündete stellen nach kurzer Rast einmal 1W6 + CON-Mod HP wieder her.", 0, "" },
};
static const ProgFeat BARD_L3[] = {
    { "Bardenschule",  "Waehle Bardenschule: Wissen (3 Fertigkeiten, Schneidende Worte) oder Wagemuet (Ruestung/Waffen, Kampfinspiration).", 0, "" },
    { "Expertise",    "Verdopple Uebungsbonus fuer 2 Fertigkeiten deiner Wahl.", 0, "" },
};
static const ProgFeat BARD_L5[] = {
    { "Bardische Inspiration (W8)", "Inspirations-Wuerfel steigt auf W8.", 0, "" },
    { "Quelle der Inspiration",     "Du erhältst alle verbrauchten Bardischen Inspirationen nach einer kurzen oder langen Rast zurück.", 0, "" },
};
static const ProgFeat BARD_L6[] = {
    { "Bannlied",              "Aktion: Verbündete in 30 Fuss erhalten Vorteil auf RW gegen Bezauberung/Angst bis zum Ende deines nächsten Zuges.", 0, "" },
    { "Merkmal: Bardenschule", "Wissen: Zusaetzliche Magische Geheimnisse (2 Zauber beliebiger Klasse). Wagemuet: Zusaetzlicher Angriff.", 0, "" },
};
static const ProgFeat BARD_L9[] = {
    { "Lied der Erholung (W8)", "Heilungswuerfel steigt auf W8.", 0, "" },
};
static const ProgFeat BARD_L10[] = {
    { "Bardische Inspiration (W10)", "Inspirations-Wuerfel steigt auf W10.", 0, "" },
    { "Expertise",                   "2 weitere Fertigkeiten erhalten doppelten Uebungsbonus.", 0, "" },
    { "Magische Geheimnisse",        "Lerne 2 Zauber beliebiger Klassen, die als Bardenzauber gelten.", 0, "" },
};
static const ProgFeat BARD_L13[] = {
    { "Lied der Erholung (W10)", "Heilungswuerfel steigt auf W10.", 0, "" },
};
static const ProgFeat BARD_L14[] = {
    { "Magische Geheimnisse",        "Lerne 2 weitere Zauber beliebiger Klassen.", 0, "" },
    { "Merkmal: Bardenschule",       "Wissen: Grenzenlose Begabung (Inspirationswuerfel auf Attributswurf). Wagemuet: Kampfmagie (Waffenang. als Bonus-Aktion bei Zaubern).", 0, "" },
};
static const ProgFeat BARD_L15[] = {
    { "Bardische Inspiration (W12)", "Inspirations-Wuerfel steigt auf W12.", 0, "" },
};
static const ProgFeat BARD_L17[] = {
    { "Lied der Erholung (W12)", "Heilungswuerfel steigt auf W12.", 0, "" },
};
static const ProgFeat BARD_L18[] = {
    { "Magische Geheimnisse", "Lerne 2 weitere Zauber beliebiger Klassen (insgesamt 8 durch Magische Geheimnisse).", 0, "" },
};
static const ProgFeat BARD_L20[] = {
    { "Ueberlegene Inspiration", "Wenn du die Initiative ergreifst und keine Bardische Inspiration mehr hast, erhältst du eine Anwendung.", 0, "" },
};

// ---- KAEMPFER ---------------------------------------------------------------
static const ProgFeat FIGHTER_L2[] = {
    { "Aktionsschub", "1x/kurze oder lange Rast: Nimm eine zusaetzliche Aktion in deinem Zug.", 1, "Kurze Rast" },
};
static const ProgFeat FIGHTER_L3[] = {
    { "Krieger-Archetyp", "Waehle: Meisterkampfsportler, Champion oder Arkanenritter.", 0, "" },
};
static const ProgFeat FIGHTER_L5[] = {
    { "Extra-Angriff", "Greifst du mit Angriffsaktion an, kannst du 2 Angriffe machen.", 0, "" },
};
static const ProgFeat FIGHTER_L7[] = {
    { "Archetyp-Feature", "Abhaengig vom gewaehlten Krieger-Archetyp.", 0, "" },
};
static const ProgFeat FIGHTER_L9[] = {
    { "Unbesiegbar", "1x/langer Rast: Wirf einen misslungenen Rettungswurf erneut.", 1, "Langer Rast" },
};
static const ProgFeat FIGHTER_L10[] = {
    { "Archetyp-Feature", "Abhaengig vom gewaehlten Krieger-Archetyp.", 0, "" },
};

// ---- MAGIER -----------------------------------------------------------------
static const ProgFeat WIZARD_L2[] = {
    { "Arkane Tradition", "Waehle eine Zauberschule als Spezialgebiet (Beschwörung, Illusion etc.).", 0, "" },
};
static const ProgFeat WIZARD_L6[] = {
    { "Traditions-Feature", "Abhaengig von der gewaehlten Arkanen Tradition.", 0, "" },
};
static const ProgFeat WIZARD_L10[] = {
    { "Traditions-Feature", "Weiteres Feature der gewaehlten Arkanen Tradition.", 0, "" },
};

// ---- SCHURKE ----------------------------------------------------------------
static const ProgFeat ROGUE_L2[] = {
    { "Gerissener Schlag", "Bonus-Aktion: Ausweichen (Dash), Zurueckziehen oder Verstecken.", 0, "" },
};
static const ProgFeat ROGUE_L3[] = {
    { "Schlingel-Archetyp", "Waehle: Dieb, Meuchler oder Arkaner Schurke.", 0, "" },
};
static const ProgFeat ROGUE_L5[] = {
    { "Unbegreifliches Ausweichen", "Wenn Angreifer sichtbar: halber Schaden wenn Rettungswurf gelingt.", 0, "" },
};
static const ProgFeat ROGUE_L6[] = {
    { "Expertise", "2 weitere Fertigkeiten erhalten doppelten Uebungsbonus.", 0, "" },
};
static const ProgFeat ROGUE_L7[] = {
    { "Ausweichen (Evasion)", "Wenn Rettungswurf gelingt: kein Schaden. Scheitert: halber Schaden.", 0, "" },
};
static const ProgFeat ROGUE_L9[] = {
    { "Archetyp-Feature", "Abhaengig vom gewaehlten Schlingel-Archetyp.", 0, "" },
};

// ---- KLERIKER ---------------------------------------------------------------
static const ProgFeat CLERIC_L2[] = {
    { "Kanalseiligkeit (1x/Rast)", "Kanal goettlicher Energie 1x/kurze Rast: Untote vertreiben oder Domain-Effekt.", 1, "Kurze Rast" },
};
static const ProgFeat CLERIC_L5[] = {
    { "Untote vernichten", "Kanal-Seiligkeit: Untote bis CR 1/2 werden vernichtet statt nur vertrieben.", 0, "" },
};
static const ProgFeat CLERIC_L6[] = {
    { "Kanalseiligkeit (2x/Rast)", "Kanal goettlicher Energie jetzt 2x pro Rast.", 2, "Kurze Rast" },
    { "Domaenen-Feature",          "Abhaengig von der gewaehlten goettlichen Domaene.", 0, "" },
};
static const ProgFeat CLERIC_L8[] = {
    { "Domaenen-Feature", "Weiteres Feature der gewaehlten goettlichen Domaene.", 0, "" },
};
static const ProgFeat CLERIC_L10[] = {
    { "Goettliche Intervention", "1x/Woche: Bitte deine Gottheit um Eingreifen (Wahrscheinlichkeit = Level%).", 1, "Langer Rast" },
};

// ---- BARBAR -----------------------------------------------------------------
static const ProgFeat BARB_L2[] = {
    { "Ruecksichtslos", "Greife mit Vorteil an. Angreifer haben bis zum naechsten Zug Vorteil gegen dich.", 0, "" },
    { "Gefahrensinn",   "Vorteil auf DEX-Rettungswuerfe gegen sichtbare Effekte.", 0, "" },
};
static const ProgFeat BARB_L3[] = {
    { "Barbarischer Urpfad", "Waehle: Berserker oder Totem-Krieger (Unterklasse).", 0, "" },
    { "Raserei x3",          "Raserei-Verwendungen steigen auf 3 pro langer Rast.", 3, "Langer Rast" },
};
static const ProgFeat BARB_L5[] = {
    { "Extra-Angriff",    "Greifst du mit Angriffsaktion an, kannst du 2 Angriffe machen.", 0, "" },
    { "Schnellbewegung",  "+10 Fuss Bewegungsreichweite (solange keine schwere Ruestung).", 0, "" },
};
static const ProgFeat BARB_L7[] = {
    { "Brutales Ueberma", "Vorteil auf Initiative-Wuerfe. Kann nicht uebersascht werden.", 0, "" },
};
static const ProgFeat BARB_L9[] = {
    { "Brutaler krit. Treffer (+1)", "Ein weiterer Schadenwuerfel bei krit. Nahkampftreffer.", 0, "" },
};
static const ProgFeat BARB_L10[] = {
    { "Urzeitliches Wissen", "Abhaengig vom gewaehlten Urpfad.", 0, "" },
};

// ---- DRUIDE -----------------------------------------------------------------
static const ProgFeat DRUID_L2[] = {
    { "Wilde Gestalt",  "Bonus-Aktion: Verwandle dich 2x/kurze Rast in ein Tier (max CR = Stufe/4).", 2, "Kurze Rast" },
    { "Druidenkreis",   "Waehle einen Druidenkreis (Land, Mond etc.) als Unterklasse.", 0, "" },
};
static const ProgFeat DRUID_L4[] = {
    { "Wilde Gestalt (verbessert)", "Jetzt auch Tiere mit Schwimmgeschwindigkeit (CR bis 1/4).", 0, "" },
};
static const ProgFeat DRUID_L6[] = {
    { "Kreis-Feature", "Abhaengig vom gewaehlten Druidenkreis.", 0, "" },
};
static const ProgFeat DRUID_L8[] = {
    { "Wilde Gestalt (CR 1)", "Kann sich jetzt in Tiere mit CR bis 1 verwandeln.", 0, "" },
};
static const ProgFeat DRUID_L10[] = {
    { "Kreis-Feature", "Weiteres Feature des gewaehlten Druidenkreises.", 0, "" },
};

// ---- MOENCHE ----------------------------------------------------------------
static const ProgFeat MONK_L2[] = {
    { "Ki (Punkte = Stufe)", "Ki-Punkte entsprechen jetzt der Stufe (2 Punkte). Neu nach kurzer Rast.", 2, "Kurze Rast" },
    { "Unarmierte Bewegung", "+10 Fuss Bewegungsreichweite ohne Ruestung.", 0, "" },
};
static const ProgFeat MONK_L3[] = {
    { "Moenchsweg",              "Waehle deinen Weg: Offene Hand, Schatten oder Vier Elemente.", 0, "" },
    { "Ablenkung von Geschossen","Als Reaktion: Wurfgeschoss ablenken oder fangen (Reflexangriff 1W10+DEX+Stufe).", 0, "" },
};
static const ProgFeat MONK_L4[] = {
    { "Langsamer Sturz", "Als Reaktion: Reduziere Sturz-Schaden um 5 x Stufe.", 0, "" },
};
static const ProgFeat MONK_L5[] = {
    { "Extra-Angriff",       "Greifst du mit Angriffsaktion an, kannst du 2 Angriffe machen.", 0, "" },
    { "Betaeubender Schlag", "Ki-Punkt ausgeben: Ziel muss KON-RW wuerfeln oder wird betaeubt.", 0, "" },
};
static const ProgFeat MONK_L6[] = {
    { "Ki-verstaerkter Schlag", "Unbewaffnete Angriffe gelten als magisch fuer Resistenz-Zwecke.", 0, "" },
    { "Weg-Feature",            "Abhaengig vom gewaehlten Moenchsweg.", 0, "" },
};
static const ProgFeat MONK_L7[] = {
    { "Ausweichen (Evasion)", "DEX-RW gelingt: kein Schaden. Scheitert: halber Schaden.", 0, "" },
    { "Geistesstille",        "Immunitet gegen Ablenken und Ueberraschen bei kurzer/langer Rast.", 0, "" },
};
static const ProgFeat MONK_L9[] = {
    { "Unarmierte Bewegung (+15)", "Bewegungsreichweite steigt auf +15 Fuss. Kann senkrechte Flaechen laufen.", 0, "" },
};
static const ProgFeat MONK_L10[] = {
    { "Reinheit des Koerpers", "Immun gegen Krankheiten und Gifte.", 0, "" },
};

// ---- PALADIN ----------------------------------------------------------------
static const ProgFeat PALADIN_L2[] = {
    { "Goettlicher Schlag", "Einen Zauberplatz ausgeben: Waffe macht + Zusatzschaden (1W8 pro Grad).", 0, "" },
    { "Kampfstil",          "Waehle: Abwehr, Duell, Grosswaffenkampf oder Schutz.", 0, "" },
    { "Zauberwirken",       "Erhalte erste Zauberplaetze (Grad 1). Zauber vorbereitend aus Paladin-Liste.", 0, "" },
};
static const ProgFeat PALADIN_L3[] = {
    { "Heilige Gesundheit",  "Immun gegen Krankheiten.", 0, "" },
    { "Heiliger Eid",        "Lege einen Heiligen Eid ab: Hingabe, Rache oder Antike Waelder.", 0, "" },
};
static const ProgFeat PALADIN_L5[] = {
    { "Extra-Angriff", "Greifst du mit Angriffsaktion an, kannst du 2 Angriffe machen.", 0, "" },
};
static const ProgFeat PALADIN_L6[] = {
    { "Aura der Schutzengel", "Verbündete in 10 Fuss erhalten deinen CHA-Bonus auf RW (nicht 0).", 0, "" },
};
static const ProgFeat PALADIN_L7[] = {
    { "Eid-Feature", "Abhaengig vom gewaehlten Heiligen Eid.", 0, "" },
};
static const ProgFeat PALADIN_L10[] = {
    { "Aura des Mutes", "Verbündete in 10 Fuss koennen nicht erschreckt werden.", 0, "" },
};

// ---- WALDLAEUFER ------------------------------------------------------------
static const ProgFeat RANGER_L2[] = {
    { "Kampfstil",   "Waehle: Bogenkampf, Verteidigung oder Zweiwaffen.", 0, "" },
    { "Zauberwirken","Erhalte erste Zauberplaetze (Grad 1). Bereite Waldlaeufer-Zauber vor.", 0, "" },
};
static const ProgFeat RANGER_L3[] = {
    { "Primale Wahrnehmung", "Kannst magische Wesen sehen, die durch Illusion verborgen sind.", 0, "" },
    { "Waldlaeufer-Archetyp","Waehle Archetyp: Jaeger oder Bestienmeister.", 0, "" },
};
static const ProgFeat RANGER_L5[] = {
    { "Extra-Angriff", "Greifst du mit Angriffsaktion an, kannst du 2 Angriffe machen.", 0, "" },
};
static const ProgFeat RANGER_L6[] = {
    { "Bevorzugter Feind II",      "Waehle einen weiteren Feindtyp + lerne eine weitere Sprache.", 0, "" },
    { "Natuerlicher Forscher II",  "Waehle ein weiteres Terrain-Gebiet.", 0, "" },
};
static const ProgFeat RANGER_L7[] = {
    { "Archetyp-Feature", "Abhaengig vom gewaehlten Waldlaeufer-Archetyp.", 0, "" },
};
static const ProgFeat RANGER_L8[] = {
    { "Landschritt", "Kein Nachteil durch magisches schwieriges Gelaende. Kein Schaden von Pflanzenmagie.", 0, "" },
};
static const ProgFeat RANGER_L10[] = {
    { "Tier verstecken", "Kannst Gefaehrten (inkl. Tier-Begleiter) verstecken.", 0, "" },
};

// ---- HEXENMEISTER -----------------------------------------------------------
static const ProgFeat WARLOCK_L2[] = {
    { "Teuflische Beschwörungen (2)", "Lerne 2 Beschwörungen: Passive Boni oder neue Faehigkeiten.", 0, "" },
};
static const ProgFeat WARLOCK_L3[] = {
    { "Pakts-Gabe", "Waehle: Klinge (Begleitwaffe), Kette (Vertrauter) oder Tome (Zauberbuch).", 0, "" },
};
static const ProgFeat WARLOCK_L5[] = {
    { "Teuflische Beschwörung (+1)", "Lerne eine weitere Beschwörung (jetzt 4 insgesamt).", 0, "" },
};
static const ProgFeat WARLOCK_L6[] = {
    { "Goennerfaehigkeit", "Abhaengig vom gewaehlten uebernatuerlichen Goenner.", 0, "" },
};
static const ProgFeat WARLOCK_L7[] = {
    { "Teuflische Beschwörung (+1)", "Lerne eine weitere Beschwörung (jetzt 5 insgesamt).", 0, "" },
};
static const ProgFeat WARLOCK_L9[] = {
    { "Teuflische Beschwörung (+1)", "Lerne eine weitere Beschwörung (jetzt 6 insgesamt).", 0, "" },
};
static const ProgFeat WARLOCK_L10[] = {
    { "Goennerfaehigkeit", "Weiteres Feature des gewaehlten uebernatuerlichen Goenners.", 0, "" },
};

// ---- ZAUBERWIRKER -----------------------------------------------------------
static const ProgFeat SORC_L2[] = {
    { "Magischer Font",  "Erhalte Zauberkraft-Punkte = Stufe. Fur Metamagie oder Zauberplatze.", 2, "Langer Rast" },
};
static const ProgFeat SORC_L3[] = {
    { "Metamagie (2 Optionen)", "Waehle 2 Metamagie-Optionen (z.B. Beschleunigt, Weitreichend, Zwilling).", 0, "" },
};
static const ProgFeat SORC_L6[] = {
    { "Ursprungs-Feature", "Abhaengig vom gewaehlten Zauber-Ursprung (z.B. Drachenahnlinie).", 0, "" },
};
static const ProgFeat SORC_L10[] = {
    { "Ursprungs-Feature", "Weiteres Feature des gewaehlten Zauber-Ursprungs.", 0, "" },
};

// ============================================================================
// GLOBALE KLASSENTABELLE
// ============================================================================
// Hilfs-Makros fuer haeufige Muster
#define L(sl, sk, asi, feats) { sl, sk, asi, feats, (int)(sizeof(feats)/sizeof(feats[0])) }
#define L0(sl)                { sl,  0,  0, NULL, 0 }
#define LA(sl)                { sl,  0,  1, NULL, 0 }  // ASI, keine neuen Features
#define LF(sl, feats)         { sl,  0,  0, feats, (int)(sizeof(feats)/sizeof(feats[0])) }
#define LFA(sl, feats)        { sl,  0,  1, feats, (int)(sizeof(feats)/sizeof(feats[0])) }
// Makros mit bekannte-Zauber-Gewinn (sk = Delta-Anzahl neuer Zauber)
#define LS(sl, sk)            { sl, sk,  0, NULL, 0 }
#define LSA(sl, sk)           { sl, sk,  1, NULL, 0 }  // ASI + Zauber
#define LSF(sl, sk, feats)    { sl, sk,  0, feats, (int)(sizeof(feats)/sizeof(feats[0])) }
#define LSFA(sl, sk, feats)   { sl, sk,  1, feats, (int)(sizeof(feats)/sizeof(feats[0])) }

const ClassProg SRD_CLASS_PROG[] = {

// ============================================================================
// BARDE
// ============================================================================
{
    "bard",
    {
        L0(SLOTS_FULL[0]),                            // Stufe 1: 4 bekannte Zauber (in SrdClass)
        LSF(SLOTS_FULL[1],  1, BARD_L2),             // Stufe 2: +1 Zauber (5 gesamt)
        LSF(SLOTS_FULL[2],  1, BARD_L3),             // Stufe 3: +1 Zauber (6 gesamt), Schule+Expertise
        LSA(SLOTS_FULL[3],  1),                       // Stufe 4: ASI, +1 Zauber (7 gesamt)
        LSF(SLOTS_FULL[4],  1, BARD_L5),             // Stufe 5: +1 Zauber (8 gesamt)
        LSF(SLOTS_FULL[5],  1, BARD_L6),             // Stufe 6: +1 Zauber (9 gesamt)
        LS(SLOTS_FULL[6],   1),                       // Stufe 7: +1 Zauber (10 gesamt)
        LSA(SLOTS_FULL[7],  1),                       // Stufe 8: ASI, +1 Zauber (11 gesamt)
        LSF(SLOTS_FULL[8],  1, BARD_L9),             // Stufe 9: +1 Zauber (12 gesamt)
        LSF(SLOTS_FULL[9],  2, BARD_L10),            // Stufe 10: +2 Zauber (14 gesamt, Mag. Geheimnisse)
        LS(SLOTS_FULL[10],  1),                       // Stufe 11: +1 Zauber (15 gesamt)
        LSA(SLOTS_FULL[11], 0),                       // Stufe 12: ASI, kein neuer Zauber
        LSF(SLOTS_FULL[12], 1, BARD_L13),            // Stufe 13: +1 Zauber (16 gesamt)
        LSF(SLOTS_FULL[13], 2, BARD_L14),            // Stufe 14: +2 Zauber (18, Mag. Geheimnisse)
        LSF(SLOTS_FULL[14], 1, BARD_L15),            // Stufe 15: +1 Zauber (19 gesamt)
        LSA(SLOTS_FULL[15], 0),                       // Stufe 16: ASI, kein neuer Zauber
        LSF(SLOTS_FULL[16], 1, BARD_L17),            // Stufe 17: +1 Zauber (20 gesamt)
        LSF(SLOTS_FULL[17], 2, BARD_L18),            // Stufe 18: +2 Zauber (22, Mag. Geheimnisse)
        LSA(SLOTS_FULL[18], 0),                       // Stufe 19: ASI, kein neuer Zauber
        LSF(SLOTS_FULL[19], 0, BARD_L20),            // Stufe 20: Ueberlegene Inspiration
    }
},

// ============================================================================
// KAEMPFER
// ============================================================================
{
    "fighter",
    {
        L0(SLOTS_NONE[0]),                           // Stufe 1
        LF(SLOTS_NONE[1],  FIGHTER_L2),             // Stufe 2
        LF(SLOTS_NONE[2],  FIGHTER_L3),             // Stufe 3
        LA(SLOTS_NONE[3]),                           // Stufe 4: ASI
        LF(SLOTS_NONE[4],  FIGHTER_L5),             // Stufe 5
        LA(SLOTS_NONE[5]),                           // Stufe 6: ASI
        LF(SLOTS_NONE[6],  FIGHTER_L7),             // Stufe 7
        LA(SLOTS_NONE[7]),                           // Stufe 8: ASI
        LF(SLOTS_NONE[8],  FIGHTER_L9),             // Stufe 9
        LF(SLOTS_NONE[9],  FIGHTER_L10),            // Stufe 10
        L0(SLOTS_NONE[10]),                          // Stufe 11
        LA(SLOTS_NONE[11]),                          // Stufe 12: ASI
        L0(SLOTS_NONE[12]),                          // Stufe 13
        LA(SLOTS_NONE[13]),                          // Stufe 14: ASI
        L0(SLOTS_NONE[14]),                          // Stufe 15
        LA(SLOTS_NONE[15]),                          // Stufe 16: ASI
        L0(SLOTS_NONE[16]),                          // Stufe 17
        L0(SLOTS_NONE[17]),                          // Stufe 18
        LA(SLOTS_NONE[18]),                          // Stufe 19: ASI
        L0(SLOTS_NONE[19]),                          // Stufe 20
    }
},

// ============================================================================
// MAGIER
// ============================================================================
{
    "wizard",
    {
        L0(SLOTS_FULL[0]),                           // Stufe 1
        LF(SLOTS_FULL[1],  WIZARD_L2),              // Stufe 2
        L0(SLOTS_FULL[2]),                           // Stufe 3
        LA(SLOTS_FULL[3]),                           // Stufe 4: ASI
        L0(SLOTS_FULL[4]),                           // Stufe 5
        LF(SLOTS_FULL[5],  WIZARD_L6),              // Stufe 6
        L0(SLOTS_FULL[6]),                           // Stufe 7
        LA(SLOTS_FULL[7]),                           // Stufe 8: ASI
        L0(SLOTS_FULL[8]),                           // Stufe 9
        LF(SLOTS_FULL[9],  WIZARD_L10),             // Stufe 10
        L0(SLOTS_FULL[10]),                          // Stufe 11
        LA(SLOTS_FULL[11]),                          // Stufe 12: ASI
        L0(SLOTS_FULL[12]),                          // Stufe 13
        L0(SLOTS_FULL[13]),                          // Stufe 14
        L0(SLOTS_FULL[14]),                          // Stufe 15
        LA(SLOTS_FULL[15]),                          // Stufe 16: ASI
        L0(SLOTS_FULL[16]),                          // Stufe 17
        L0(SLOTS_FULL[17]),                          // Stufe 18
        LA(SLOTS_FULL[18]),                          // Stufe 19: ASI
        L0(SLOTS_FULL[19]),                          // Stufe 20
    }
},

// ============================================================================
// SCHURKE
// ============================================================================
{
    "rogue",
    {
        L0(SLOTS_NONE[0]),                           // Stufe 1
        LF(SLOTS_NONE[1],  ROGUE_L2),               // Stufe 2
        LF(SLOTS_NONE[2],  ROGUE_L3),               // Stufe 3
        LA(SLOTS_NONE[3]),                           // Stufe 4: ASI
        LF(SLOTS_NONE[4],  ROGUE_L5),               // Stufe 5
        LF(SLOTS_NONE[5],  ROGUE_L6),               // Stufe 6
        LF(SLOTS_NONE[6],  ROGUE_L7),               // Stufe 7
        LA(SLOTS_NONE[7]),                           // Stufe 8: ASI
        LF(SLOTS_NONE[8],  ROGUE_L9),               // Stufe 9
        LA(SLOTS_NONE[9]),                           // Stufe 10: ASI
        L0(SLOTS_NONE[10]),                          // Stufe 11
        LA(SLOTS_NONE[11]),                          // Stufe 12: ASI
        L0(SLOTS_NONE[12]),                          // Stufe 13
        L0(SLOTS_NONE[13]),                          // Stufe 14
        L0(SLOTS_NONE[14]),                          // Stufe 15
        LA(SLOTS_NONE[15]),                          // Stufe 16: ASI
        L0(SLOTS_NONE[16]),                          // Stufe 17
        L0(SLOTS_NONE[17]),                          // Stufe 18
        LA(SLOTS_NONE[18]),                          // Stufe 19: ASI
        L0(SLOTS_NONE[19]),                          // Stufe 20
    }
},

// ============================================================================
// KLERIKER
// ============================================================================
{
    "cleric",
    {
        L0(SLOTS_FULL[0]),                           // Stufe 1
        LF(SLOTS_FULL[1],  CLERIC_L2),              // Stufe 2
        L0(SLOTS_FULL[2]),                           // Stufe 3
        LA(SLOTS_FULL[3]),                           // Stufe 4: ASI
        LF(SLOTS_FULL[4],  CLERIC_L5),              // Stufe 5
        LF(SLOTS_FULL[5],  CLERIC_L6),              // Stufe 6
        L0(SLOTS_FULL[6]),                           // Stufe 7
        LFA(SLOTS_FULL[7], CLERIC_L8),              // Stufe 8: ASI + Feature
        L0(SLOTS_FULL[8]),                           // Stufe 9
        LF(SLOTS_FULL[9],  CLERIC_L10),             // Stufe 10
        L0(SLOTS_FULL[10]),                          // Stufe 11
        LA(SLOTS_FULL[11]),                          // Stufe 12: ASI
        L0(SLOTS_FULL[12]),                          // Stufe 13
        L0(SLOTS_FULL[13]),                          // Stufe 14
        L0(SLOTS_FULL[14]),                          // Stufe 15
        LA(SLOTS_FULL[15]),                          // Stufe 16: ASI
        L0(SLOTS_FULL[16]),                          // Stufe 17
        L0(SLOTS_FULL[17]),                          // Stufe 18
        LA(SLOTS_FULL[18]),                          // Stufe 19: ASI
        L0(SLOTS_FULL[19]),                          // Stufe 20
    }
},

// ============================================================================
// BARBAR
// ============================================================================
{
    "barbarian",
    {
        L0(SLOTS_NONE[0]),                           // Stufe 1
        LF(SLOTS_NONE[1],  BARB_L2),                // Stufe 2
        LF(SLOTS_NONE[2],  BARB_L3),                // Stufe 3
        LA(SLOTS_NONE[3]),                           // Stufe 4: ASI
        LF(SLOTS_NONE[4],  BARB_L5),                // Stufe 5
        L0(SLOTS_NONE[5]),                           // Stufe 6
        LF(SLOTS_NONE[6],  BARB_L7),                // Stufe 7
        LA(SLOTS_NONE[7]),                           // Stufe 8: ASI
        LF(SLOTS_NONE[8],  BARB_L9),                // Stufe 9
        LF(SLOTS_NONE[9],  BARB_L10),               // Stufe 10
        L0(SLOTS_NONE[10]),                          // Stufe 11
        LA(SLOTS_NONE[11]),                          // Stufe 12: ASI
        L0(SLOTS_NONE[12]),                          // Stufe 13
        L0(SLOTS_NONE[13]),                          // Stufe 14
        L0(SLOTS_NONE[14]),                          // Stufe 15
        LA(SLOTS_NONE[15]),                          // Stufe 16: ASI
        L0(SLOTS_NONE[16]),                          // Stufe 17
        L0(SLOTS_NONE[17]),                          // Stufe 18
        LA(SLOTS_NONE[18]),                          // Stufe 19: ASI
        L0(SLOTS_NONE[19]),                          // Stufe 20
    }
},

// ============================================================================
// DRUIDE
// ============================================================================
{
    "druid",
    {
        L0(SLOTS_FULL[0]),                           // Stufe 1
        LF(SLOTS_FULL[1],  DRUID_L2),               // Stufe 2
        L0(SLOTS_FULL[2]),                           // Stufe 3
        LFA(SLOTS_FULL[3], DRUID_L4),               // Stufe 4: ASI + Feature
        L0(SLOTS_FULL[4]),                           // Stufe 5
        LF(SLOTS_FULL[5],  DRUID_L6),               // Stufe 6
        L0(SLOTS_FULL[6]),                           // Stufe 7
        LFA(SLOTS_FULL[7], DRUID_L8),               // Stufe 8: ASI + Feature
        L0(SLOTS_FULL[8]),                           // Stufe 9
        LF(SLOTS_FULL[9],  DRUID_L10),              // Stufe 10
        L0(SLOTS_FULL[10]),                          // Stufe 11
        LA(SLOTS_FULL[11]),                          // Stufe 12: ASI
        L0(SLOTS_FULL[12]),                          // Stufe 13
        L0(SLOTS_FULL[13]),                          // Stufe 14
        L0(SLOTS_FULL[14]),                          // Stufe 15
        LA(SLOTS_FULL[15]),                          // Stufe 16: ASI
        L0(SLOTS_FULL[16]),                          // Stufe 17
        L0(SLOTS_FULL[17]),                          // Stufe 18
        LA(SLOTS_FULL[18]),                          // Stufe 19: ASI
        L0(SLOTS_FULL[19]),                          // Stufe 20
    }
},

// ============================================================================
// MOENCHE
// ============================================================================
{
    "monk",
    {
        L0(SLOTS_NONE[0]),                           // Stufe 1
        LF(SLOTS_NONE[1],  MONK_L2),                // Stufe 2
        LF(SLOTS_NONE[2],  MONK_L3),                // Stufe 3
        LFA(SLOTS_NONE[3], MONK_L4),                // Stufe 4: ASI + Feature
        LF(SLOTS_NONE[4],  MONK_L5),                // Stufe 5
        LF(SLOTS_NONE[5],  MONK_L6),                // Stufe 6
        LF(SLOTS_NONE[6],  MONK_L7),                // Stufe 7
        LA(SLOTS_NONE[7]),                           // Stufe 8: ASI
        LF(SLOTS_NONE[8],  MONK_L9),                // Stufe 9
        LF(SLOTS_NONE[9],  MONK_L10),               // Stufe 10
        L0(SLOTS_NONE[10]),                          // Stufe 11
        LA(SLOTS_NONE[11]),                          // Stufe 12: ASI
        L0(SLOTS_NONE[12]),                          // Stufe 13
        L0(SLOTS_NONE[13]),                          // Stufe 14
        L0(SLOTS_NONE[14]),                          // Stufe 15
        LA(SLOTS_NONE[15]),                          // Stufe 16: ASI
        L0(SLOTS_NONE[16]),                          // Stufe 17
        L0(SLOTS_NONE[17]),                          // Stufe 18
        LA(SLOTS_NONE[18]),                          // Stufe 19: ASI
        L0(SLOTS_NONE[19]),                          // Stufe 20
    }
},

// ============================================================================
// PALADIN
// ============================================================================
{
    "paladin",
    {
        L0(SLOTS_HALF[0]),                           // Stufe 1 (keine Zauber)
        LF(SLOTS_HALF[1],  PALADIN_L2),             // Stufe 2
        LF(SLOTS_HALF[2],  PALADIN_L3),             // Stufe 3
        LA(SLOTS_HALF[3]),                           // Stufe 4: ASI
        LF(SLOTS_HALF[4],  PALADIN_L5),             // Stufe 5
        LF(SLOTS_HALF[5],  PALADIN_L6),             // Stufe 6
        LF(SLOTS_HALF[6],  PALADIN_L7),             // Stufe 7
        LA(SLOTS_HALF[7]),                           // Stufe 8: ASI
        L0(SLOTS_HALF[8]),                           // Stufe 9
        LF(SLOTS_HALF[9],  PALADIN_L10),            // Stufe 10
        L0(SLOTS_HALF[10]),                          // Stufe 11
        LA(SLOTS_HALF[11]),                          // Stufe 12: ASI
        L0(SLOTS_HALF[12]),                          // Stufe 13
        L0(SLOTS_HALF[13]),                          // Stufe 14
        L0(SLOTS_HALF[14]),                          // Stufe 15
        LA(SLOTS_HALF[15]),                          // Stufe 16: ASI
        L0(SLOTS_HALF[16]),                          // Stufe 17
        L0(SLOTS_HALF[17]),                          // Stufe 18
        LA(SLOTS_HALF[18]),                          // Stufe 19: ASI
        L0(SLOTS_HALF[19]),                          // Stufe 20
    }
},

// ============================================================================
// WALDLAEUFER
// ============================================================================
{
    "ranger",
    {
        L0(SLOTS_HALF[0]),                           // Stufe 1 (keine Zauber)
        LF(SLOTS_HALF[1],  RANGER_L2),              // Stufe 2
        LF(SLOTS_HALF[2],  RANGER_L3),              // Stufe 3
        LA(SLOTS_HALF[3]),                           // Stufe 4: ASI
        LF(SLOTS_HALF[4],  RANGER_L5),              // Stufe 5
        LF(SLOTS_HALF[5],  RANGER_L6),              // Stufe 6
        LF(SLOTS_HALF[6],  RANGER_L7),              // Stufe 7
        LFA(SLOTS_HALF[7], RANGER_L8),              // Stufe 8: ASI + Feature
        L0(SLOTS_HALF[8]),                           // Stufe 9
        LF(SLOTS_HALF[9],  RANGER_L10),             // Stufe 10
        L0(SLOTS_HALF[10]),                          // Stufe 11
        LA(SLOTS_HALF[11]),                          // Stufe 12: ASI
        L0(SLOTS_HALF[12]),                          // Stufe 13
        L0(SLOTS_HALF[13]),                          // Stufe 14
        L0(SLOTS_HALF[14]),                          // Stufe 15
        LA(SLOTS_HALF[15]),                          // Stufe 16: ASI
        L0(SLOTS_HALF[16]),                          // Stufe 17
        L0(SLOTS_HALF[17]),                          // Stufe 18
        LA(SLOTS_HALF[18]),                          // Stufe 19: ASI
        L0(SLOTS_HALF[19]),                          // Stufe 20
    }
},

// ============================================================================
// HEXENMEISTER
// ============================================================================
{
    "warlock",
    {
        L0(SLOTS_WARLOCK[0]),                        // Stufe 1
        LF(SLOTS_WARLOCK[1],  WARLOCK_L2),           // Stufe 2
        LF(SLOTS_WARLOCK[2],  WARLOCK_L3),           // Stufe 3
        LA(SLOTS_WARLOCK[3]),                         // Stufe 4: ASI
        LF(SLOTS_WARLOCK[4],  WARLOCK_L5),           // Stufe 5
        LF(SLOTS_WARLOCK[5],  WARLOCK_L6),           // Stufe 6
        LF(SLOTS_WARLOCK[6],  WARLOCK_L7),           // Stufe 7
        LA(SLOTS_WARLOCK[7]),                         // Stufe 8: ASI
        LF(SLOTS_WARLOCK[8],  WARLOCK_L9),           // Stufe 9
        LF(SLOTS_WARLOCK[9],  WARLOCK_L10),          // Stufe 10
        L0(SLOTS_WARLOCK[10]),                        // Stufe 11
        LA(SLOTS_WARLOCK[11]),                        // Stufe 12: ASI
        L0(SLOTS_WARLOCK[12]),                        // Stufe 13
        L0(SLOTS_WARLOCK[13]),                        // Stufe 14
        L0(SLOTS_WARLOCK[14]),                        // Stufe 15
        LA(SLOTS_WARLOCK[15]),                        // Stufe 16: ASI
        L0(SLOTS_WARLOCK[16]),                        // Stufe 17
        L0(SLOTS_WARLOCK[17]),                        // Stufe 18
        LA(SLOTS_WARLOCK[18]),                        // Stufe 19: ASI
        L0(SLOTS_WARLOCK[19]),                        // Stufe 20
    }
},

// ============================================================================
// ZAUBERWIRKER (SORCERER)
// ============================================================================
{
    "sorcerer",
    {
        L0(SLOTS_FULL[0]),                           // Stufe 1
        LF(SLOTS_FULL[1],  SORC_L2),                // Stufe 2
        LF(SLOTS_FULL[2],  SORC_L3),                // Stufe 3
        LA(SLOTS_FULL[3]),                           // Stufe 4: ASI
        L0(SLOTS_FULL[4]),                           // Stufe 5
        LF(SLOTS_FULL[5],  SORC_L6),                // Stufe 6
        L0(SLOTS_FULL[6]),                           // Stufe 7
        LA(SLOTS_FULL[7]),                           // Stufe 8: ASI
        L0(SLOTS_FULL[8]),                           // Stufe 9
        LF(SLOTS_FULL[9],  SORC_L10),               // Stufe 10
        L0(SLOTS_FULL[10]),                          // Stufe 11
        LA(SLOTS_FULL[11]),                          // Stufe 12: ASI
        L0(SLOTS_FULL[12]),                          // Stufe 13
        L0(SLOTS_FULL[13]),                          // Stufe 14
        L0(SLOTS_FULL[14]),                          // Stufe 15
        LA(SLOTS_FULL[15]),                          // Stufe 16: ASI
        L0(SLOTS_FULL[16]),                          // Stufe 17
        L0(SLOTS_FULL[17]),                          // Stufe 18
        LA(SLOTS_FULL[18]),                          // Stufe 19: ASI
        L0(SLOTS_FULL[19]),                          // Stufe 20
    }
},

};

const int SRD_CLASS_PROG_COUNT = sizeof(SRD_CLASS_PROG) / sizeof(SRD_CLASS_PROG[0]);

// ============================================================================
// HILFSFUNKTIONEN
// ============================================================================

const ClassProg* srd_prog_get(const char* class_id) {
    for (int i = 0; i < SRD_CLASS_PROG_COUNT; i++)
        if (strcmp(SRD_CLASS_PROG[i].class_id, class_id) == 0)
            return &SRD_CLASS_PROG[i];
    return NULL;
}

void srd_prog_spell_slots(const char* class_id, int level,
                          int out_slots[PROG_SLOT_LEVELS]) {
    if (level < 1 || level > PROG_LEVELS) {
        memset(out_slots, 0, PROG_SLOT_LEVELS * sizeof(int));
        return;
    }
    const ClassProg* p = srd_prog_get(class_id);
    if (!p) {
        memset(out_slots, 0, PROG_SLOT_LEVELS * sizeof(int));
        return;
    }
    const int* s = p->levels[level - 1].slots;
    if (!s) { memset(out_slots, 0, PROG_SLOT_LEVELS * sizeof(int)); return; }
    for (int i = 0; i < PROG_SLOT_LEVELS; i++)
        out_slots[i] = s[i];
}

int srd_prog_is_asi(const char* class_id, int level) {
    if (level < 1 || level > PROG_LEVELS) return 0;
    const ClassProg* p = srd_prog_get(class_id);
    if (!p) return 0;
    return p->levels[level - 1].is_asi;
}

int srd_prog_feats(const char* class_id, int level, const ProgFeat** out_feats) {
    if (level < 1 || level > PROG_LEVELS) { *out_feats = NULL; return 0; }
    const ClassProg* p = srd_prog_get(class_id);
    if (!p) { *out_feats = NULL; return 0; }
    *out_feats = p->levels[level - 1].feats;
    return p->levels[level - 1].feat_count;
}

int srd_prog_hp_average(int hit_die, int con_mod) {
    int gain = hit_die / 2 + 1 + con_mod;
    return gain < 1 ? 1 : gain;
}

int srd_prog_hp_max(int hit_die, int con_mod) {
    int gain = hit_die + con_mod;
    return gain < 1 ? 1 : gain;
}
