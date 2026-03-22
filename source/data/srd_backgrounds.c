#include "srd_backgrounds.h"

// Skill-Indizes aus character.h:
// SKILL_INSIGHT=6, SKILL_RELIGION=14, SKILL_PERCEPTION=11,
// SKILL_DECEPTION=4, SKILL_STEALTH=16, SKILL_PERSUASION=13,
// SKILL_ATHLETICS=3, SKILL_SURVIVAL=17, SKILL_ARCANA=2,
// SKILL_HISTORY=5, SKILL_INVESTIGATION=8, SKILL_MEDICINE=9,
// SKILL_NATURE=10, SKILL_PERFORMANCE=12, SKILL_INTIMIDATION=7,
// SKILL_ANIMAL_HANDLING=1, SKILL_SLEIGHT_OF_HAND=15, SKILL_ACROBATICS=0

const SrdBackground SRD_BACKGROUNDS[] = {
    // --- AKOLYTH (SRD 5.1 – vollstaendig) -----------------------------------
    {
        "acolyte",
        "Akolyth",
        "Diener einer Kirche oder Tempelgemeinschaft.",
        "Schutzbefohlener der Glaeubigen",
        "Du kannst dich an Tempel deiner Gottheit um kostenlose Heilung wenden."
        " Du und deine Gruppe erhalten einfache Unterkunft und Verpflegung.",
        { 6, 14 },              // Insight, Religion
        2,
        "Keine",
        "Zwei Sprachen deiner Wahl",
        "Heiliges Symbol, Gebetsbuch, 5 Raeuchersticks, Robe, 15 GP",
        1500                    // 15 GP in Kupfer
    },
    // --- KRIMINELLER (Criminal, SRD-nah) ------------------------------------
    {
        "criminal",
        "Krimineller",
        "Jemand, der jenseits der Gesetze gelebt hat.",
        "Kriminelle Kontakte",
        "Du hast zuverlässige Kontakte zu kriminellen Netzwerken in Staedten.",
        { 4, 16 },              // Deception, Stealth
        2,
        "Gluecksspiel-Set oder Diebeswerkzeug",
        "Keine",
        "Brecheisen, Dunkelkleidung mit Kapuze, 15 GP",
        1500
    },
    // --- VOLKSHELD (Folk Hero, SRD 5.1) -------------------------------------
    {
        "folk_hero",
        "Volksheld",
        "Aus einfachem Volk, zum Helden durch aussergewoehnliche Tat.",
        "Gesicht des Volkes",
        "Einfache Leute helfen dir: Unterkunft, Essen, Infos aus der Gemeinschaft.",
        { 3, 17 },              // Athletics, Survival
        2,
        "Ein Handwerker-Werkzeug deiner Wahl, Reittiere",
        "Keine",
        "Handwerker-Werkzeug, Schaufel, Eisentopf, Bauernskleidung, 10 GP",
        1000
    },
    // --- ADELIGER (Noble, SRD 5.1) ------------------------------------------
    {
        "noble",
        "Adeliger",
        "Geboren in Reichtum und Privilegien der Oberschicht.",
        "Adeliger Rang",
        "Andere nehmen deine Anweisungen ernst und bieten bevorzugte Behandlung.",
        { 5, 13 },              // History, Persuasion
        2,
        "Ein Gluecksspiel-Set deiner Wahl",
        "Eine Sprache deiner Wahl",
        "Feine Kleidung, Siegelring, Schuldschein (25 GP), Goldmuenzen (25 GP)",
        2500
    },
    // --- WEISER (Sage, SRD 5.1) ---------------------------------------------
    {
        "sage",
        "Weiser",
        "Gelehrter, der Jahre in Bibliotheken und Schulen verbracht hat.",
        "Forscher",
        "Weisst wo du Informationen findest. Weigert sich jemand zu helfen, weisst du wo du sonst suchen kannst.",
        { 2, 5 },               // Arcana, History
        2,
        "Keine",
        "Zwei Sprachen deiner Wahl",
        "Flasche Tinte, Feder, kleines Messer, Briefe von ehemaligem Schieler, Gelehrtenskleidung, 10 GP",
        1000
    },
    // --- SOLDAT (Soldier, SRD 5.1) ------------------------------------------
    {
        "soldier",
        "Soldat",
        "Hat als Teil einer Armee oder Miliz gedient.",
        "Militaerischer Rang",
        "Andere Soldaten kennen deinen Rang und erweisen Respekt. Zugang zu Militaereinrichtungen.",
        { 3, 7 },               // Athletics, Intimidation
        2,
        "Spiele-Set (Wuerfel oder Karten)",
        "Keine",
        "Rangabzeichen, Kriegstrophae, Spielset, Gemeinschaftskleidung, 10 GP",
        1000
    },
    // --- WILDNISBEWOHNER (Outlander, SRD 5.1) -------------------------------
    {
        "outlander",
        "Wildnisbewohner",
        "Hat ausserhalb der Zivilisation in der Wildnis gelebt.",
        "Wanderer",
        "Kannst in der Wildnis fuer dich und die Gruppe Nahrung und Wasser finden (nicht in Wueste/Tundra etc.).",
        { 1, 17 },              // Animal Handling, Survival
        2,
        "Ein Musikinstrument",
        "Eine Sprache deiner Wahl",
        "Stab, Jagdfalle, Trophae eines Tieres, Reisekleidung, 10 GP",
        1000
    },
    // --- UNTERHALTUNGSKUENSTLER (Entertainer, SRD 5.1) ----------------------
    {
        "entertainer",
        "Unterhaltungskuenstler",
        "Aufgewachsen mit Auftritten vor Publikum.",
        "Popular",
        "Du bekommst in einem Ort wo du aufgetreten bist kostenloses Essen und Unterkunft. Kannst bei Adligen auftreten.",
        { 0, 12 },              // Acrobatics, Performance
        2,
        "Musikinstrument",
        "Keine",
        "Musikinstrument, Gunstbeweis eines Bewunderers, Kostuem, 15 GP",
        1500
    },
};

const int SRD_BACKGROUND_COUNT = sizeof(SRD_BACKGROUNDS) / sizeof(SRD_BACKGROUNDS[0]);
