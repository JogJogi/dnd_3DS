#include "screen_manager.h"
#include "ui_core.h"
#include "../db/character_db.h"
#include "../models/character.h"
#include <stdio.h>
#include <string.h>

// ---- Zustand ---------------------------------------------------------------
#define MAX_CHARS 8

static int       s_char_ids[MAX_CHARS];
static char      s_char_names[MAX_CHARS][64];
static int       s_char_count = 0;
static Character s_char_preview;   // Vorschau des ausgewählten Chars
static int       s_preview_loaded = 0;

static UiButton s_btn_new;
static UiButton s_btn_delete;
static UiList   s_char_list;

static void load_characters(void) {
    s_char_count = character_db_list(s_char_ids, s_char_names, MAX_CHARS);
    if (s_char_count < 0) s_char_count = 0;

    ui_list_init(&s_char_list, 4, 28, 312, 164, 24);
    for (int i = 0; i < s_char_count; i++) {
        ui_list_set_item(&s_char_list, i, s_char_names[i], COLOR_TEXT);
    }
    s_char_list.selected = s_char_count > 0 ? 0 : -1;
    s_preview_loaded = 0;
}

static void load_preview(void) {
    if (s_char_list.selected >= 0 && s_char_list.selected < s_char_count) {
        s_preview_loaded = (character_db_load(s_char_ids[s_char_list.selected],
                                               &s_char_preview) == 0);
    } else {
        s_preview_loaded = 0;
    }
}

// ---- Screen Callbacks ------------------------------------------------------

static void on_enter(void) {
    load_characters();
    load_preview();

    // Buttons: untere Zeile
    s_btn_new    = (UiButton){  4, 196, 150, 26, "+ Neuer Charakter", COLOR_ACCENT,     COLOR_TEXT, 0};
    s_btn_delete = (UiButton){162, 196, 154, 26, "Loeschen",          COLOR_BTN_NORMAL, COLOR_TEXT, 0};
}

static void on_exit(void) {}

static void update(u32 keys_down, u32 keys_held, touchPosition* touch, int touch_down) {
    (void)keys_held;

    int prev_sel = s_char_list.selected;

    // D-Pad Navigation
    if (keys_down & KEY_DOWN)  { ui_list_scroll(&s_char_list, 1);  s_char_list.selected++; }
    if (keys_down & KEY_UP)    { ui_list_scroll(&s_char_list, -1); s_char_list.selected--; }
    if (s_char_list.selected < 0) s_char_list.selected = 0;
    if (s_char_list.selected >= s_char_count && s_char_count > 0)
        s_char_list.selected = s_char_count - 1;
    if (s_char_list.selected != prev_sel) load_preview();

    // A = Charakter laden
    if ((keys_down & KEY_A) && s_char_list.selected >= 0 && s_char_count > 0) {
        g_active_char_id = s_char_ids[s_char_list.selected];
        screen_push(SCREEN_CHARACTER);
        return;
    }

    // Y = Neuen Charakter erstellen (Builder-Wizard)
    if (keys_down & KEY_Y) {
        screen_push(SCREEN_BUILDER);
        return;
    }

    if (!touch_down) return;

    // Touch-Liste
    int old_sel = s_char_list.selected;
    ui_list_handle_touch(&s_char_list, touch, touch_down);
    if (s_char_list.selected != old_sel) load_preview();

    // Doppel-Tap auf Liste öffnet Charakter (beim touch_down = steigende Flanke öffnen bei selected)
    if (ui_point_in_rect((float)touch->px, (float)touch->py,
                          s_char_list.x, s_char_list.y, s_char_list.w, s_char_list.h)
        && s_char_list.selected >= 0 && s_char_count > 0) {
        g_active_char_id = s_char_ids[s_char_list.selected];
        screen_push(SCREEN_CHARACTER);
        return;
    }

    if (ui_button_touched(&s_btn_new, touch)) {
        // Gefuehrten Charakter-Erstellungs-Wizard starten
        screen_push(SCREEN_BUILDER);
        return;
    }
    if (ui_button_touched(&s_btn_delete, touch) && s_char_count > 0 && s_char_list.selected >= 0) {
        character_db_delete(s_char_ids[s_char_list.selected]);
        load_characters();
        load_preview();
        return;
    }
}

static void draw_top(void) {
    // Header-Panel
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_text(10, 5, 0.5f, 0.62f, COLOR_GOLD, "D&D Companion");
    ui_draw_text(200, 8, 0.5f, 0.38f, COLOR_TEXT_DIM, "Charakterauswahl");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    if (s_char_count == 0) {
        ui_draw_text(20, 90,  0.5f, 0.48f, COLOR_TEXT_DIM, "Noch keine Charaktere.");
        ui_draw_text(20, 115, 0.5f, 0.42f, COLOR_TEXT_DIM, "Erstelle deinen ersten Helden!");
        return;
    }

    // Charakter-Vorschau (ausgewählter Charakter)
    if (s_preview_loaded) {
        Character* c = &s_char_preview;

        // Name groß
        ui_draw_textf(10, 35, 0.5f, 0.62f, COLOR_TEXT, "%s", c->name);

        // Klasse / Volk / Stufe
        ui_draw_textf(10, 58, 0.5f, 0.42f, COLOR_TEXT_DIM,
                      "Stufe %d  %s", c->level, c->class_name);
        if (c->race[0])
            ui_draw_textf(200, 58, 0.5f, 0.42f, COLOR_TEXT_DIM, "%s", c->race);

        // HP-Leiste
        ui_draw_separator(0, 76, SCREEN_TOP_W, COLOR_TEXT_DIM);
        ui_draw_textf(10, 80, 0.5f, 0.38f, COLOR_TEXT_DIM, "HP");
        ui_draw_hp_bar(40, 80, 200, 14, c->hp_current, c->hp_max);
        ui_draw_textf(248, 78, 0.5f, 0.42f, COLOR_TEXT,
                      "%d/%d", c->hp_current, c->hp_max);

        // Kern-Stats in einer Zeile
        ui_draw_separator(0, 100, SCREEN_TOP_W, COLOR_TEXT_DIM);
        ui_draw_textf( 10, 106, 0.5f, 0.42f, COLOR_TEXT, "AC %d",  c->armor_class);
        ui_draw_textf( 80, 106, 0.5f, 0.42f, COLOR_TEXT, "Spd %d", c->speed);
        ui_draw_textf(160, 106, 0.5f, 0.42f, COLOR_TEXT, "XP %d",  c->experience);
        if (c->inspiration)
            ui_draw_text(320, 106, 0.5f, 0.42f, COLOR_GOLD, "★ INSP");

        // Gesinnung / Hintergrund
        if (c->alignment[0])
            ui_draw_textf(10, 124, 0.5f, 0.38f, COLOR_TEXT_DIM, "%s", c->alignment);
        if (c->background[0])
            ui_draw_textf(200, 124, 0.5f, 0.38f, COLOR_TEXT_DIM, "%s", c->background);
    } else if (s_char_list.selected >= 0 && s_char_list.selected < s_char_count) {
        ui_draw_textf(10, 40, 0.5f, 0.52f, COLOR_TEXT, "%s", s_char_names[s_char_list.selected]);
    }

    ui_draw_separator(0, 224, SCREEN_TOP_W, COLOR_TEXT_DIM);
    ui_draw_text(10, 228, 0.5f, 0.34f, COLOR_TEXT_DIM,
                 "[A] Laden  [Y] Neu  [DPad] Nav");
}

static void draw_bottom(void) {
    // Header
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_text(8, 5, 0.5f, 0.45f, COLOR_TEXT_DIM, "Charaktere");
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

    // Charakter-Liste
    ui_list_draw(&s_char_list);

    // Separator vor Buttons
    ui_draw_separator(0, 194, SCREEN_BTM_W, COLOR_TEXT_DIM);

    // Buttons
    s_btn_delete.color_bg = (s_char_list.selected >= 0 && s_char_count > 0)
                            ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_new);
    ui_button_draw(&s_btn_delete);

    // Hilfetext
    ui_draw_text(8, 225, 0.5f, 0.33f, COLOR_TEXT_DIM, "[A] Oeffnen   Tippen zum Auswaehlen");
}

// ---- Screen-Objekt ---------------------------------------------------------
Screen g_screen_main_menu = {
    .id          = SCREEN_MAIN_MENU,
    .on_enter    = on_enter,
    .on_exit     = on_exit,
    .update      = update,
    .draw_top    = draw_top,
    .draw_bottom = draw_bottom
};
