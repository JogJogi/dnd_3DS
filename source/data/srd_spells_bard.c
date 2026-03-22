#include "srd_spells_bard.h"

// ============================================================================
// BARDEN-ZAUBERTRICKS (Cantrips)
// ============================================================================
const BardSpellEntry BARD_CANTRIP_LIST[] = {
    {
        "Klingenabwehr",       "Blade Ward",
        0, "Abjuration",
        "1 Aktion", "Selbst", "V, S", "1 Runde",
        "Du erhaeltst Resistenz gegen Hieb-, Stich- und Wuchtschaden durch Waffen bis zum Ende deines naechsten Zuges.",
        0, 0
    },
    {
        "Tanzende Lichter",    "Dancing Lights",
        0, "Illusion",
        "1 Aktion", "36 m", "V, S, M", "Konzentration, bis zu 1 Min.",
        "Erschaffe bis zu 4 faustgrosse Lichtquellen (Fackel-Helligkeit, 3m Radius). Kannst sie als Bonus-Aktion bewegen (bis zu 18m).",
        1, 0
    },
    {
        "Freundschaft",        "Friends",
        0, "Verzauberung",
        "1 Aktion", "Selbst", "S, M", "Konzentration, bis zu 1 Min.",
        "Vorteil auf alle CHA-Proben gegen ein nicht-feindliches Ziel fuer die Dauer. Danach merkt das Ziel es und wird feindselig.",
        1, 0
    },
    {
        "Licht",               "Light",
        0, "Hervorrufung",
        "1 Aktion", "Beruehrung", "V, M", "1 Stunde",
        "Ein Objekt strahlt helles Licht in 6m Radius aus (weiteres truebes Licht in 6m). WIS-Rettungswurf verweigert die Wirkung.",
        0, 0
    },
    {
        "Magikerhand",         "Mage Hand",
        0, "Hervorrufung",
        "1 Aktion", "9 m", "V, S", "1 Min.",
        "Geisterhafte Hand erscheint: traegt bis zu 5 kg, bedient Objekte, oeffnet/schliesst Tueren/Behaelter. Nicht fuer Angriffe.",
        0, 0
    },
    {
        "Ausbessern",          "Mending",
        0, "Transmutation",
        "1 Min.", "Beruehrung", "V, S, M", "Sofort",
        "Repariert einen einfachen Riss oder Bruch in einem Gegenstand. Kann magische Gegenstaende nicht reparieren.",
        0, 0
    },
    {
        "Botschaft",           "Message",
        0, "Transmutation",
        "1 Aktion", "36 m", "V, S, M", "1 Runde",
        "Zeige auf ein Wesen und fluestere eine Botschaft. Nur das Ziel hoert sie und kann ebenso leise antworten.",
        0, 0
    },
    {
        "Kleine Illusion",     "Minor Illusion",
        0, "Illusion",
        "1 Aktion", "9 m", "S, M", "1 Min.",
        "Erzeuge einen Geraeusch-Effekt ODER ein visuelles Bild (max. 1,5m^3). Wesen koennen mit INT-Probe erkennen, dass es eine Illusion ist.",
        0, 0
    },
    {
        "Prestidigitation",    "Prestidigitation",
        0, "Transmutation",
        "1 Aktion", "3 m", "V, S", "bis zu 1 Stunde",
        "Kleine magische Tricks: Feuer entfachen/loeschen, reinigen oder beschmutzen, kleinen Gegenstand erschaffen, Farbe aendern u.v.m.",
        0, 0
    },
    {
        "Wahrer Treffer",      "True Strike",
        0, "Divination",
        "1 Aktion", "9 m", "S", "Konzentration, bis zu 1 Runde",
        "Gewinne Einblick in die Verteidigung eines Ziels. Vorteil auf deinen naechsten Angriffswurf gegen das Ziel.",
        1, 0
    },
    {
        "Boshaftes Spotten",   "Vicious Mockery",
        0, "Verzauberung",
        "1 Aktion", "18 m", "V", "Sofort",
        "Ein Wesen das dich hoeren kann: WIS-RW oder 1W4 psych. Schaden und Nachteil auf seinen naechsten Angriffswurf. "
        "Schaden: 2W4 (L5), 3W4 (L11), 4W4 (L17).",
        0, 0
    },
};
const int BARD_CANTRIP_COUNT = sizeof(BARD_CANTRIP_LIST) / sizeof(BARD_CANTRIP_LIST[0]);

// ============================================================================
// BARDEN-ZAUBER GRAD 1-9 (SRD-Auswahl)
// ============================================================================
const BardSpellEntry BARD_SPELL_LIST[] = {
    // ---- GRAD 1 ------------------------------------------------------------
    {
        "Bann",                "Bane",
        1, "Verzauberung",
        "1 Aktion", "9 m", "V, S, M", "Konzentration, bis zu 1 Min.",
        "Bis zu 3 Wesen: CHA-RW oder -1W4 auf Angriffswuerfe und Rettungswuerfe fuer die Dauer.",
        1, 0
    },
    {
        "Bezaubern",           "Charm Person",
        1, "Verzauberung",
        "1 Aktion", "9 m", "V, S", "1 Stunde",
        "Ein humanoidesr Wesen: WIS-RW oder bezaubert bis Zauber endet oder es geschaedigt wird. "
        "Danach weiss es, dass du es bezaubert hast.",
        0, 0
    },
    {
        "Sprachen verstehen",  "Comprehend Languages",
        1, "Divination",
        "1 Aktion", "Selbst", "V, S, M", "1 Stunde",
        "Du verstehst die wortwoertliche Bedeutung aller gesprochenen oder geschriebenen Sprachen fuer die Dauer.",
        0, 1
    },
    {
        "Wunden heilen",       "Cure Wounds",
        1, "Hervorrufung",
        "1 Aktion", "Beruehrung", "V, S", "Sofort",
        "Ein Wesen erhoeht seine HP um 1W8 + Zauberstatistik-Modifikator. Kein Effekt auf Untote oder Konstrukte.",
        0, 0
    },
    {
        "Magie entdecken",     "Detect Magic",
        1, "Divination",
        "1 Aktion", "Selbst", "V, S", "Konzentration, bis zu 10 Min.",
        "Spuere Magie in 9m Radius. Aktion: Siehst Aura um magische Gegenstaende/Wesen. "
        "Erkennst die Schule der Magie.",
        1, 1
    },
    {
        "Verkleidung",         "Disguise Self",
        1, "Illusion",
        "1 Aktion", "Selbst", "V, S", "1 Stunde",
        "Aendere dein Aussehen: Kleidung, Gesicht, Groesse (bis zu 30cm anders). "
        "Illusorisch: Beruehren oder INT-Probe entlarvt die Illusion.",
        0, 0
    },
    {
        "Feenfeuer",           "Faerie Fire",
        1, "Hervorrufung",
        "1 Aktion", "18 m", "V", "Konzentration, bis zu 1 Min.",
        "Objekte und Wesen in 6m^3-Wuerfel: DEX-RW oder von blauem, gruenen oder violettem Licht umhuellt. "
        "Angriffe haben Vorteil. Keine Unsichtbarkeit moeglich.",
        1, 0
    },
    {
        "Federfall",           "Feather Fall",
        1, "Transmutation",
        "1 Reaktion", "18 m", "V, M", "1 Min.",
        "Bis zu 5 fallende Wesen sinken 18m/Runde und landen sanft. Kein Fallschaden.",
        0, 0
    },
    {
        "Heilendes Wort",      "Healing Word",
        1, "Hervorrufung",
        "1 Bonus-Aktion", "18 m", "V", "Sofort",
        "Ein Wesen erhoeht seine HP um 1W4 + Zauberstatistik-Modifikator. "
        "Schneller als Wunden heilen, aber weniger kraftvoll.",
        0, 0
    },
    {
        "Heldenmut",           "Heroism",
        1, "Verzauberung",
        "1 Aktion", "Beruehrung", "V, S", "Konzentration, bis zu 1 Min.",
        "Ein williges Wesen wird immun gegen Furcht und erhaelt zu Beginn jedes Zuges "
        "temporaere HP in Hoehe deines Zaubermerkmal-Modifikators.",
        1, 0
    },
    {
        "Laecherliches Gelaechter", "Hideous Laughter",
        1, "Verzauberung",
        "1 Aktion", "9 m", "V, S, M", "Konzentration, bis zu 1 Min.",
        "Ein Wesen (INT > 4): WIS-RW oder lacht unkontrolliert und ist niedergeworfen + bewegungsunfaehig. "
        "Wdh. RW bei Schaden.",
        1, 0
    },
    {
        "Identifizieren",      "Identify",
        1, "Divination",
        "1 Min.", "Beruehrung", "V, S, M", "Sofort",
        "Erkenne magische Eigenschaften eines Gegenstands, oder einen aktiven Zauber auf einem Wesen "
        "(Zauberspruch-Name, Dauer, Wirkung).",
        0, 1
    },
    {
        "Schlafen",            "Sleep",
        1, "Verzauberung",
        "1 Aktion", "27 m", "V, S, M", "1 Min.",
        "5W8 HP Wesen in einem 6m Kreis schlafen ein (niedrigste HP zuerst). "
        "Wesen > max. verbleibende HP sind immun. Schaden weckt Schlafende.",
        0, 0
    },
    {
        "Mit Tieren sprechen", "Speak with Animals",
        1, "Divination",
        "1 Aktion", "Selbst", "V, S", "10 Min.",
        "Verstehe und kommuniziere mit Bestien. Sie koennen begrenzte Infos zu nahen Orten, Wesen usw. geben.",
        0, 1
    },
    {
        "Donnerwoge",          "Thunderwave",
        1, "Hervorrufung",
        "1 Aktion", "Selbst (4,5m Wuerfel)", "V, S", "Sofort",
        "Alle in 4,5m: CON-RW oder 2W8 Donner und weggestossen (3m). Erfolg: halber Schaden. "
        "Schaden: 3W8(L2), 4W8(L3) usw.",
        0, 0
    },
    {
        "Unsichtbarer Diener", "Unseen Servant",
        1, "Hervorrufung",
        "1 Aktion", "18 m", "V, S, M", "1 Stunde",
        "Erschaffe einen unsichtbaren Kraft-Diener mit 1 HP (AC 10). Fuehrt einfache Aufgaben aus. "
        "Kann max. 5 kg tragen.",
        0, 1
    },
    // ---- GRAD 2 ------------------------------------------------------------
    {
        "Blindheit/Taubheit",  "Blindness/Deafness",
        2, "Nekromantie",
        "1 Aktion", "9 m", "V", "1 Min.",
        "Ein Wesen: CON-RW oder blind oder taub (deine Wahl) fuer die Dauer. Neuer RW am Ende jedes Zuges.",
        0, 0
    },
    {
        "Gedanken erspueren",  "Detect Thoughts",
        2, "Divination",
        "1 Aktion", "Selbst", "V, S, M", "Konzentration, bis zu 1 Min.",
        "Erspuere Gedanken in 9m. Aktion: Lies Gedanken eines Wesens (WIS-RW verhindert tiefen Scan).",
        1, 0
    },
    {
        "Faehigkeiten staerken","Enhance Ability",
        2, "Transmutation",
        "1 Aktion", "Beruehrung", "V, S, M", "Konzentration, bis zu 1 Stunde",
        "Waehle ein Attribut: STR(Baerenkraft), DEX(Katzengnade), CON(Steinhaut), INT(Fuchsgeist), "
        "WIS(Eulenweisheit), CHA(Adlerpracht). Jeweils Vorteil auf Proben + Bonus.",
        1, 0
    },
    {
        "Fesselung",           "Hold Person",
        2, "Verzauberung",
        "1 Aktion", "18 m", "V, S, M", "Konzentration, bis zu 1 Min.",
        "Ein Humanoid: WIS-RW oder lahmgelegt fuer die Dauer. Neuer RW am Ende jedes Zuges. "
        "Angriffe auf Gelaehmte: autom. Treffer in Nahkampf.",
        1, 0
    },
    {
        "Unsichtbarkeit",      "Invisibility",
        2, "Illusion",
        "1 Aktion", "Beruehrung", "V, S, M", "Konzentration, bis zu 1 Stunde",
        "Ein Wesen wird unsichtbar. Endet wenn es angreift oder einen Zauber wirkt. "
        "Hoehere Zauberstufe: +1 Wesen pro Grad.",
        1, 0
    },
    {
        "Geringere Wiederherstellung", "Lesser Restoration",
        2, "Hervorrufung",
        "1 Aktion", "Beruehrung", "V, S", "Sofort",
        "Beende eine Krankheit, Verblindung, Betaeubung, Taubheit oder Vergiftung auf einem Wesen.",
        0, 0
    },
    {
        "Suggestion",          "Suggestion",
        2, "Verzauberung",
        "1 Aktion", "9 m", "V, M", "Konzentration, bis zu 8 Stunden",
        "Ein Wesen das du hoerst: WIS-RW oder befolgt eine sinnvolle Anweisung bis zum Abschluss "
        "oder Zauberende. Anweisung darf nicht direkt schaedlich sein.",
        1, 0
    },
    {
        "Erschuetterung",      "Shatter",
        2, "Hervorrufung",
        "1 Aktion", "18 m", "V, S, M", "Sofort",
        "Laerender Knall in 3m Radius: CON-RW oder 3W8 Donner. Unbelebte anorganische Objekte: auto. Schaden. "
        "Schaden: +1W8 pro Zusatzstufe.",
        0, 0
    },
    // ---- GRAD 3 ------------------------------------------------------------
    {
        "Fluch verhaften",     "Bestow Curse",
        3, "Nekromantie",
        "1 Aktion", "Beruehrung", "V, S", "Konzentration, bis zu 1 Min.",
        "Beruehre ein Wesen: WIS-RW oder verflucht. Waehle: Nachteil auf Proben eines Attributs, "
        "Nachteil auf Angriffe gegen dich, WIS-RW zu Beginn sonst kein Bonus-Aktion fuer dich, "
        "oder +1W8 Nekromantie-Schaden.",
        1, 0
    },
    {
        "Magie aufloesen",     "Dispel Magic",
        3, "Abjuration",
        "1 Aktion", "36 m", "V, S", "Sofort",
        "Beende automatisch alle Zauber von Grad 3 oder niedriger auf dem Ziel. "
        "Hoehere Zauber: Zauberstatistik-Probe (DC 10 + Zaubergrad).",
        0, 0
    },
    {
        "Furcht",              "Fear",
        3, "Illusion",
        "1 Aktion", "Selbst (9m Kegel)", "V, S, M", "Konzentration, bis zu 1 Min.",
        "Wesen im Bereich: WIS-RW oder veraeengstigt und muss fliehen. Kann nach dem Bewegen "
        "nur Ausweichen als Aktion machen. Neuer RW am Ende jedes Zuges.",
        1, 0
    },
    {
        "Hypnotisches Muster", "Hypnotic Pattern",
        3, "Illusion",
        "1 Aktion", "36 m", "S, M", "Konzentration, bis zu 1 Min.",
        "Wirbelnd leuchtendes Muster in 9m Wuerfel: WIS-RW oder fasziniert. Faszinierte Wesen sind "
        "gelähmt. Enden wenn Schaden oder jemand wachrüttelt.",
        1, 0
    },
    {
        "Groesseres Bild",     "Major Image",
        3, "Illusion",
        "1 Aktion", "36 m", "V, S, M", "Konzentration, bis zu 10 Min.",
        "Erschaffe ein Bild (max. 6m^3) mit Bild, Ton, Geruch und Temperatur. "
        "Kann mit Aktion angepasst werden. INT-Probe entlarvt als Illusion.",
        1, 0
    },
    {
        "Sende Botschaft",     "Sending",
        3, "Hervorrufung",
        "1 Aktion", "Unbegrenzt", "V, S, M", "1 Runde",
        "Sende eine 25-Wort-Nachricht an ein Wesen, das du kennst. Es kann kurz antworten. "
        "Wesen auf anderen Ebenen: 5% Chance des Scheiterns.",
        0, 0
    },
    {
        "Mit Toten sprechen",  "Speak with Dead",
        3, "Nekromantie",
        "1 Aktion", "3 m", "V, S, M", "10 Min.",
        "Ein Leichnam kann dir bis zu 5 Fragen beantworten (soweit er wusste). "
        "Untote, Wesen die < 10 Tage tot sind, und feindlich Gestorbene koennen luegen.",
        0, 0
    },
    // ---- GRAD 4 ------------------------------------------------------------
    {
        "Verwirrung",          "Confusion",
        4, "Verzauberung",
        "1 Aktion", "27 m", "V, S, M", "Konzentration, bis zu 1 Min.",
        "Wesen in 3m Radius: WIS-RW oder verwirrt. Verwirrt: Wuerfel am Anfang jedes Zuges "
        "fuer zuaellige Aktion (Angriff auf Verbündeten, bewegen, nichts tun). Neuer RW.",
        1, 0
    },
    {
        "Dimensionstor",       "Dimension Door",
        4, "Beschwörung",
        "1 Aktion", "150 m", "V", "Sofort",
        "Teleportiere dich an einen Ort innerhalb der Reichweite, den du siehst oder kennst. "
        "Kannst ein weiteres williges Wesen mitnehmen.",
        0, 0
    },
    {
        "Groessere Unsichtbarkeit", "Greater Invisibility",
        4, "Illusion",
        "1 Aktion", "Beruehrung", "V, S", "Konzentration, bis zu 1 Min.",
        "Wesen bleibt unsichtbar, auch waehrend Angriff oder Zauberwirken. "
        "Angreifer haben Nachteil, Ziel hat Vorteil.",
        1, 0
    },
    {
        "Polymorphose",        "Polymorph",
        4, "Transmutation",
        "1 Aktion", "18 m", "V, S, M", "Konzentration, bis zu 1 Stunde",
        "Verwandle ein Wesen in eine Bestie. Unwilliges Wesen: WIS-RW. Neue Statistiken der Bestie. "
        "Bei 0 HP zurueck. Wissen und Faehigkeiten weg bis Rueckverwandlung.",
        1, 0
    },
    // ---- GRAD 5 ------------------------------------------------------------
    {
        "Groessere Wiederherstellung", "Greater Restoration",
        5, "Abjuration",
        "1 Aktion", "Beruehrung", "V, S, M (100 GP Diamant)", "Sofort",
        "Entferne: eine Erschoepfungsstufe, oder: Bezaubern/Versteinern/Verfluchen/Attributsreduzierung "
        "oder HP-Maximum-Reduzierung.",
        0, 0
    },
    {
        "Masse Wunden heilen", "Mass Cure Wounds",
        5, "Hervorrufung",
        "1 Aktion", "18 m", "V, S", "Sofort",
        "Bis zu 6 Wesen in 9m Radius erhalten je 3W8 + Zaubermerkmal-Modifikator HP.",
        0, 0
    },
    {
        "Tote erwecken",       "Raise Dead",
        5, "Nekromantie",
        "1 Stunde", "Beruehrung", "V, S, M (500 GP Diamant)", "Sofort",
        "Erwecke einen Toten (bis zu 10 Tage tot), der willig ist. Lebt mit 1 HP. "
        "Sterblich (keine Untoten). 4 Erschoepfungsstufen. -4 auf alles, bis Stufe geheilt.",
        0, 0
    },
    // ---- GRAD 6 ------------------------------------------------------------
    {
        "Magie wahrnehmen",    "True Seeing",
        6, "Divination",
        "1 Aktion", "Beruehrung", "V, S, M", "1 Stunde",
        "Wesen sieht im Dunkeln (18m), durchschaut Illusionen, sieht Unsichtbares, erkennt "
        "Gestaltwandler und sieht in die Aethersphare (18m).",
        0, 0
    },
    // ---- GRAD 7 ------------------------------------------------------------
    {
        "Mythos und Legende",  "Mordenkainen's Magnificent Mansion",
        7, "Beschwörung",
        "1 Min.", "9 m", "V, S, M", "24 Stunden",
        "Erschaffe ein magisches Gebaeude auf einer anderen Ebene mit bis zu 100 Zimmern. "
        "Bis zu 100 willkommene Wesen koennen eintreten.",
        0, 0
    },
    // ---- GRAD 8 ------------------------------------------------------------
    {
        "Gesinnungsaenderung",  "Dominate Monster",
        8, "Verzauberung",
        "1 Aktion", "18 m", "V, S", "Konzentration, bis zu 1 Stunde",
        "Ein Wesen: WIS-RW oder faellt unter deine Kontrolle. Du gibst telepathische Befehle. "
        "Neuer RW bei Schaden.",
        1, 0
    },
    // ---- GRAD 9 ------------------------------------------------------------
    {
        "Echte Polymorph.",    "True Polymorph",
        9, "Transmutation",
        "1 Aktion", "9 m", "V, S, M", "Konzentration, bis zu 1 Stunde",
        "Verwandle ein Wesen oder einen Gegenstand in ein anderes Wesen oder Objekt. "
        "Nach 1 Stunde Konzentration: permanent.",
        1, 0
    },
};

const int BARD_SPELL_COUNT = sizeof(BARD_SPELL_LIST) / sizeof(BARD_SPELL_LIST[0]);

// ============================================================================
// HILFSFUNKTIONEN
// ============================================================================

const BardSpellEntry* bard_get_cantrip(int index) {
    if (index < 0 || index >= BARD_CANTRIP_COUNT) return NULL;
    return &BARD_CANTRIP_LIST[index];
}

const BardSpellEntry* bard_get_spell(int index) {
    if (index < 0 || index >= BARD_SPELL_COUNT) return NULL;
    return &BARD_SPELL_LIST[index];
}

int bard_spell_count_by_level(int level) {
    int count = 0;
    for (int i = 0; i < BARD_SPELL_COUNT; i++) {
        if (BARD_SPELL_LIST[i].level == level) count++;
    }
    return count;
}
