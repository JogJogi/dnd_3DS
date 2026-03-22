#include "srd_feats.h"
#include "../models/character.h"

// ============================================================================
// SRD FEATS (D&D 5e System Reference Document)
// ============================================================================

const SrdFeat SRD_FEATS[] = {
    // ---- Kampf-Feats --------------------------------------------------------
    {
        "alert",
        "Alarmbereit",
        "+5 auf Initiative. Du kannst nicht ueberrascht werden, waehrend du bei Bewusstsein bist. "
        "Unsichtbare Wesen haben keinen Vorteil auf Angriffswuerfe gegen dich.",
        -1, NULL
    },
    {
        "crossbow_expert",
        "Armbrustexperte",
        "Du ignorierst die Eigenschaft 'Laden' bei Armbruesten. Nahkampf hindert dich nicht an "
        "Fernkampfangriffen mit Armbruesten. Mit Bonus-Aktion kannst du mit Handarmbrust angreifen.",
        -1, NULL
    },
    {
        "defensive_duelist",
        "Defensiver Duellant",
        "Reaktion: Wenn du mit einer Finesse-Waffe angegriffen wirst, die du haeltst, "
        "addiere deinen Uebungsbonus zu deiner RK fuer diesen Angriff.",
        -1, "Geschick 13+"
    },
    {
        "dual_wielder",
        "Beidhaendiger Kaempfer",
        "+1 auf RK, wenn du in beiden Haenden je eine Waffe haeltst. Du kannst zwei nicht-leichte "
        "Waffen gleichzeitig fuehren. Zwei-Waffen-Kampf auch ohne freie Hand moeglich.",
        -1, NULL
    },
    {
        "great_weapon_master",
        "Grosswaffenmeister",
        "Wenn du mit einer schweren Waffe einen kritischen Treffer erzielst oder ein Wesen auf "
        "0 HP bringst, kannst du als Bonus-Aktion einen Waffenangriff machen. "
        "Optional: -5 auf Angriff fuer +10 Schaden.",
        -1, NULL
    },
    {
        "mage_slayer",
        "Magierjaeger",
        "Reaktion: Wenn ein Wesen in Nahkampfreichweite einen Zauber wirkt, kannst du angreifen. "
        "Das Wesen hat Nachteil auf KON-Rettungswuerfe fuer Konzentration, wenn du es verletzt. "
        "Vorteil auf Rettungswuerfe gegen Zauber von Wesen in Nahkampfreichweite.",
        -1, NULL
    },
    {
        "polearm_master",
        "Stangenwaffen-Meister",
        "Bonus-Aktion: Angriff mit dem anderen Ende deiner Stangenwaffe (1W4 Wuchtschaden). "
        "Wenn ein Wesen in deine Reichweite eintritt, kannst du als Reaktion angreifen.",
        -1, NULL
    },
    {
        "sentinel",
        "Wachposten",
        "Wenn du einen Gelegenheitsangriff triffst, sinkt die Bewegungsrate auf 0. "
        "Gelegenheitsangriffe auch wenn Wesen mit Rueckzug-Aktion abzieht. "
        "Reaktion: Wenn Wesen in Nahkampfreichweite ein anderes Ziel angreift, kannst du angreifen.",
        -1, NULL
    },
    {
        "sharpshooter",
        "Scharfschuetze",
        "Fernkampfangriffe ignorieren halbe und dreiviertel Deckung. Kein Nachteil auf "
        "Fernkampfangriffe auf lange Distanz. Optional: -5 auf Angriff fuer +10 Schaden.",
        -1, NULL
    },
    {
        "war_caster",
        "Kampfzauberer",
        "Vorteil auf KON-Rettungswuerfe zur Aufrechterhaltung von Konzentration. "
        "Du kannst Zauberwirkbewegungen auch mit Waffen/Schild in den Haenden ausfuehren. "
        "Reaktion: Wenn ein Wesen eine Bewegung ausloest, kannst du einen Zauber mit "
        "Wirkzeit 'Aktion' als Gelegenheitsangriff wirken.",
        -1, NULL
    },
    // ---- Zauber-Feats -------------------------------------------------------
    {
        "magic_initiate",
        "Zauberanfaenger",
        "Waehle eine Klasse. Lerne 2 Zaubertricks und einen Grad-1-Zauber dieser Klasse. "
        "Den Grad-1-Zauber kannst du 1x/Tag ohne Zauberplatz wirken. "
        "Zaubermerkmal: das dieser Klasse zugehoerige.",
        -1, NULL
    },
    {
        "ritual_caster",
        "Ritualzauberer",
        "Lerne 2 Ritualzauber deiner Wahl (max. Grad 1) aus einer Klasse. "
        "Kannst diese nur als Ritual wirken. Kannst weitere Rituale in einem Ritualbuch aufschreiben.",
        -1, "INT oder WIS 13+"
    },
    {
        "spell_sniper",
        "Zauberschuetze",
        "Verdoppelt die Reichweite von Zaubern, die einen Angriffswurf erfordern. "
        "Deine Zaubertreffer ignorieren halbe und dreiviertel Deckung. "
        "Lerne einen Zaubertrick mit Angriffswurf aus einer beliebigen Klassenliste.",
        -1, "Zauberwirker-Eigenschaft"
    },
    // ---- Allgemeine Feats ---------------------------------------------------
    {
        "actor",
        "Schauspieler",
        "+1 auf CHA (Half-Feat). Vorteil auf Tauschung und Auftreten, wenn du jemanden imitierst. "
        "Kannst Stimme und Sprechweise einer anderen Person perfekt imitieren.",
        ABILITY_CHA, NULL
    },
    {
        "athlete",
        "Athlet",
        "+1 auf STR oder DEX (Half-Feat). Aufstehen kostet nur 5 Fuss Bewegung. "
        "Klettern kostet keine zusaetzliche Bewegung. Anlauf aus dem Stand mit nur 1,5m Anlauf.",
        ABILITY_STR, NULL  // ABILITY_STR als Default, DEX auch gueltig
    },
    {
        "charger",
        "Stuermender Angreifer",
        "Wenn du mindestens 3m in einer geraden Linie laeuft und dann angreifst, "
        "+5 Schaden oder schiebe Ziel bis zu 3m zurueck (als Bonus-Aktion).",
        -1, NULL
    },
    {
        "durable",
        "Ausgehaertet",
        "+1 auf CON (Half-Feat). Wenn du Trefferwuerfel bei kurzer Rast wuerfelst, "
        "ist das Minimum deines Wurfes 2x dein CON-Modifikator (mind. 1).",
        ABILITY_CON, NULL
    },
    {
        "grappler",
        "Ringkaempfer",
        "Vorteil auf Angriffswuerfe gegen Wesen, das du festhaeltst. "
        "Aktion: Kannst ein festgehaltenes Wesen auch selbst bewegungsunfaehig machen "
        "(du bist dann ebenfalls bewegungsunfaehig).",
        -1, "STR 13+"
    },
    {
        "healer",
        "Heiler",
        "Mit Heiler-Kit: Stabilisiere Wesen auf 0 HP ohne Heilungswurf. "
        "Aktion + 1 Kit-Nutzung: Stelle 1W6+4+Charakterstufe HP wieder her (1x pro Wesen/Rast).",
        -1, NULL
    },
    {
        "inspiring_leader",
        "Inspirierender Anfuehrer",
        "10 Min. Ansprache: Bis zu 6 Wesen, die du siehst und hoerst, erhalten temporaere HP "
        "in Hoehe von deiner Stufe + CHA-Modifikator.",
        -1, "CHA 13+"
    },
    {
        "keen_mind",
        "Scharfsinnig",
        "+1 auf INT (Half-Feat). Du weisst immer, welche Richtung Norden ist. "
        "Du weisst immer, wann zuletzt Sonnenauf- oder -untergang war. "
        "Du erinnerst dich genau an alles, was du in den letzten 30 Tagen erlebt hast.",
        ABILITY_INT, NULL
    },
    {
        "lucky",
        "Glueck",
        "3 Glueckspunkte/Langrast. Ausgeben, um bei Angriff/Fertigkeit/Rettungswurf "
        "zusaetzlichen W20 zu wuerfeln und das bessere Ergebnis zu nehmen. "
        "Auch gegen dich: nimm das schlechtere Ergebnis des Angreifers.",
        -1, NULL
    },
    {
        "mobile",
        "Beweglich",
        "+3m Bewegungsrate. Bei Dash-Aktion kein schwieriges Gelaende. "
        "Wenn du ein Wesen angreifst (ob Treffer oder nicht), loest das Wesen keinen "
        "Gelegenheitsangriff gegen dich fuer diese Runde aus.",
        -1, NULL
    },
    {
        "observant",
        "Aufmerksam",
        "+1 auf INT oder WIS (Half-Feat). +5 auf passive Wahrnehmung und passive Ermittlung. "
        "Kannst Lippen lesen, wenn Sprache bekannt.",
        ABILITY_WIS, NULL  // WIS als Default, INT auch gueltig
    },
    {
        "resilient",
        "Widerstandsfaehig",
        "+1 auf ein Attribut nach Wahl (Half-Feat). Erlange Uebung mit Rettungswuerfen "
        "des gewahlten Attributs.",
        ABILITY_CON, NULL  // CON als Default, beliebig gueltig
    },
    {
        "savage_attacker",
        "Wilder Angreifer",
        "1x/Zug: Wenn du mit einer Nahkampfwaffe Schaden machst, wuerfelst du den Schadenswuerfel "
        "zweimal und nimmst das bessere Ergebnis.",
        -1, NULL
    },
    {
        "skilled",
        "Geschickt",
        "Erhalte Uebung in 3 Fertigkeiten oder Werkzeugen deiner Wahl.",
        -1, NULL
    },
    {
        "tavern_brawler",
        "Kneipenschlaeger",
        "+1 auf STR oder CON (Half-Feat). Uebung mit improvisierten Waffen und unbewaffnetem Kampf. "
        "Unbewaffneter Angriff: 1W4 Wuchtschaden. Bonus-Aktion: Ringkampf nach Treffer.",
        ABILITY_STR, NULL  // STR als Default, CON auch gueltig
    },
    {
        "tough",
        "Zaeh",
        "Maximale HP erhoehen sich um 2x Charakterstufe sofort und um 2 bei jedem weiteren Stufenaufstieg.",
        -1, NULL
    },
    {
        "weapon_master",
        "Waffentechniker",
        "+1 auf STR oder DEX (Half-Feat). Erlange Uebung mit 4 Waffen deiner Wahl, "
        "fuer die du noch keine Uebung hast.",
        ABILITY_STR, NULL  // STR als Default, DEX auch gueltig
    },
};

const int SRD_FEAT_COUNT = sizeof(SRD_FEATS) / sizeof(SRD_FEATS[0]);
