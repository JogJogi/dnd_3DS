#include "screen_builder.h"
#include "ui_core.h"
#include "../data/srd_races.h"
#include "../data/srd_classes.h"
#include "../data/srd_backgrounds.h"
#include "../utils/dnd_rules.h"
#include <stdio.h>
#include <string.h>

// ---- Gesinnung-Optionen ----------------------------------------------------
static const char* ALIGNMENTS[] = {
    "Rechtschaffen Gut",
    "Neutral Gut",
    "Chaotisch Gut",
    "Rechtschaffen Neutral",
    "Neutral",
    "Chaotisch Neutral",
    "Rechtschaffen Boese",
    "Neutral Boese",
    "Chaotisch Boese",
};
#define ALIGNMENT_COUNT 9

// ---- Zustand ---------------------------------------------------------------
static int s_cursor = 0;    // 0 = Name, 1 = Gesinnung
static int s_align_idx = 4; // Standard: Neutral

static UiButton s_btn_name;
static UiButton s_btn_next;
static UiButton s_btn_back;

static UiList   s_align_list;

// ---- Initialisierung -------------------------------------------------------

void step_details_init(void) {
    s_cursor = 0;

    // Name-Button (zeigt aktuellen Namen)
    s_btn_name = (UiButton){ 4, 54, 312, 30, g_builder.name[0] ? g_builder.name : "[ Name eingeben ]",
                             COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_name.label = g_builder.name[0] ? g_builder.name : "[ Name eingeben ]";

    // Gesinnung-Auswahl vorbefuellen aus Builder-Zustand
    s_align_idx = 4; // Default: Neutral
    if (g_builder.alignment[0]) {
        for (int i = 0; i < ALIGNMENT_COUNT; i++) {
            if (strncmp(g_builder.alignment, ALIGNMENTS[i], CHAR_NAME_MAX) == 0) {
                s_align_idx = i;
                break;
            }
        }
    } else {
        // Standard setzen
        strncpy(g_builder.alignment, ALIGNMENTS[4], CHAR_NAME_MAX - 1);
    }

    // Gesinnung-Liste
    ui_list_init(&s_align_list, 4, 100, 312, 90, 22);
    for (int i = 0; i < ALIGNMENT_COUNT; i++)
        ui_list_set_item(&s_align_list, i, ALIGNMENTS[i],
                         i == s_align_idx ? COLOR_GOLD : COLOR_TEXT);
    s_align_list.selected = s_align_idx;

    s_btn_back = (UiButton){ 4,   202, 100, 26, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_next = (UiButton){ 210, 202, 106, 26, "Weiter >",  COLOR_ACCENT,     COLOR_TEXT, 0 };
}

// ---- Top-Screen: Zusammenfassung bisheriger Auswahl ------------------------

void step_details_draw_top(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Schritt 5/6: Details");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    int y = 36;

    // Name
    ui_draw_textf(10, y, 0.5f, 0.42f, COLOR_TEXT_DIM, "Name:");
    if (g_builder.name[0])
        ui_draw_textf(70, y, 0.5f, 0.50f, COLOR_TEXT, "%s", g_builder.name);
    else
        ui_draw_textf(70, y, 0.5f, 0.42f, COLOR_TEXT_DIM, "(nicht gesetzt)");
    y += 18;

    // Gesinnung
    ui_draw_textf(10, y, 0.5f, 0.42f, COLOR_TEXT_DIM, "Gesinnung:");
    ui_draw_textf(110, y, 0.5f, 0.42f, COLOR_GOLD, "%s", g_builder.alignment);
    y += 18;

    ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM);
    y += 6;

    // Rasse + Klasse
    if (g_builder.race_idx >= 0) {
        const SrdRace* r = &SRD_RACES[g_builder.race_idx];
        const Subrace* sub = NULL;
        if (g_builder.subrace_idx >= 0 && g_builder.subrace_idx < r->subrace_count)
            sub = &r->subraces[g_builder.subrace_idx];
        char asi[48];
        srd_race_asi_summary(r, sub, asi, sizeof(asi));
        ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_TEXT,
                      "Rasse: %s   %s",
                      sub ? sub->name : r->name, asi);
    } else {
        ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_TEXT_DIM, "Rasse: (nicht gewaehlt)");
    }
    y += 16;

    if (g_builder.class_idx >= 0) {
        const SrdClass* cls = &SRD_CLASSES[g_builder.class_idx];
        ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_TEXT,
                      "Klasse: %s  W%d  HP: %d",
                      cls->name, cls->hit_die, builder_initial_hp());
    } else {
        ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_TEXT_DIM, "Klasse: (nicht gewaehlt)");
    }
    y += 16;

    if (g_builder.bg_idx >= 0)
        ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_TEXT,
                      "Hintergrund: %s", SRD_BACKGROUNDS[g_builder.bg_idx].name);
    else
        ui_draw_textf(10, y, 0.5f, 0.40f, COLOR_TEXT_DIM, "Hintergrund: (nicht gewaehlt)");
    y += 16;

    ui_draw_separator(0, y, SCREEN_TOP_W, COLOR_TEXT_DIM);
    y += 6;

    // Attributs-Vorschau (kompakt)
    ui_draw_text(10, y, 0.5f, 0.38f, COLOR_TEXT, "Attribute:");
    y += 14;
    char attr_line[128];
    attr_line[0] = '\0';
    for (int i = 0; i < ABILITY_COUNT; i++) {
        char tmp[24];
        int final = builder_final_ability(i);
        int mod   = dnd_modifier(final);
        snprintf(tmp, sizeof(tmp), "%s %d(%+d)  ", ABILITY_SHORT[i], final, mod);
        strncat(attr_line, tmp, sizeof(attr_line) - strlen(attr_line) - 1);
        if (i == 2) {
            ui_draw_textf(10, y, 0.5f, 0.38f, COLOR_TEXT, "%s", attr_line);
            attr_line[0] = '\0';
            y += 14;
        }
    }
    if (attr_line[0])
        ui_draw_textf(10, y, 0.5f, 0.38f, COLOR_TEXT, "%s", attr_line);
}

// ---- Bottom-Screen ---------------------------------------------------------

void step_details_draw_bottom(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_textf(8, 5, 0.5f, 0.45f, COLOR_TEXT_DIM, "Name & Gesinnung");
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

    // Name-Eingabe-Button
    ui_draw_text(8, 32, 0.5f, 0.40f, COLOR_TEXT_DIM, "Name:");
    // Button-Label dynamisch aktualisieren
    s_btn_name.label = g_builder.name[0] ? g_builder.name : "[ Tippen um Namen einzugeben ]";
    s_btn_name.color_bg = (s_cursor == 0) ? COLOR_PANEL2 : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_name);

    // Gesinnung-Label
    ui_draw_text(8, 88, 0.5f, 0.40f, COLOR_TEXT_DIM, "Gesinnung:");

    // Gesinnung-Farben aktualisieren
    for (int i = 0; i < ALIGNMENT_COUNT; i++)
        s_align_list.item_colors[i] = (i == s_align_idx) ? COLOR_GOLD : COLOR_TEXT;
    s_align_list.selected = s_align_idx;
    ui_list_draw(&s_align_list);

    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);

    s_btn_next.color_bg = g_builder.name[0] ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_back);
    ui_button_draw(&s_btn_next);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM,
                 "[A] Name eingeben  [DPad] Gesinnung");
}

// ---- Update ----------------------------------------------------------------

void step_details_update(u32 keys_down, u32 keys_held,
                         touchPosition* touch, int touch_down) {
    (void)keys_held;

    if (keys_down & KEY_B) { builder_prev_step(); return; }

    // A = Name eingeben
    if (keys_down & KEY_A) {
        char buf[CHAR_NAME_MAX];
        strncpy(buf, g_builder.name, CHAR_NAME_MAX);
        if (ui_keyboard_input("Charaktername", buf, CHAR_NAME_MAX))
            snprintf(g_builder.name, CHAR_NAME_MAX, "%s", buf);
        return;
    }

    // DPad navigiert Gesinnungsliste
    if (keys_down & KEY_DOWN) {
        s_align_idx = (s_align_idx + 1) % ALIGNMENT_COUNT;
        strncpy(g_builder.alignment, ALIGNMENTS[s_align_idx], CHAR_NAME_MAX - 1);
    }
    if (keys_down & KEY_UP) {
        s_align_idx = (s_align_idx - 1 + ALIGNMENT_COUNT) % ALIGNMENT_COUNT;
        strncpy(g_builder.alignment, ALIGNMENTS[s_align_idx], CHAR_NAME_MAX - 1);
    }

    if (!touch_down) return;

    // Name-Button
    if (ui_button_touched(&s_btn_name, touch)) {
        char buf[CHAR_NAME_MAX];
        strncpy(buf, g_builder.name, CHAR_NAME_MAX);
        if (ui_keyboard_input("Charaktername", buf, CHAR_NAME_MAX))
            snprintf(g_builder.name, CHAR_NAME_MAX, "%s", buf);
        return;
    }

    // Gesinnung Liste
    int old = s_align_list.selected;
    ui_list_handle_touch(&s_align_list, touch, touch_down);
    if (s_align_list.selected != old && s_align_list.selected >= 0) {
        s_align_idx = s_align_list.selected;
        strncpy(g_builder.alignment, ALIGNMENTS[s_align_idx], CHAR_NAME_MAX - 1);
    }

    if (ui_button_touched(&s_btn_next, touch) && g_builder.name[0]) {
        builder_next_step();
        return;
    }
    if (ui_button_touched(&s_btn_back, touch)) { builder_prev_step(); return; }
}
