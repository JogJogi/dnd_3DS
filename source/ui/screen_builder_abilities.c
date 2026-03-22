#include "screen_builder.h"
#include "ui_core.h"
#include "../utils/dnd_rules.h"
#include <stdio.h>
#include <string.h>

// ---- Point-Buy Kosten-Tabelle (Score - 8 = Index) --------------------------
// Score:  8  9 10 11 12 13 14 15
// Cost:   0  1  2  3  4  5  7  9  (kumulativ)
static const int PB_COST[8] = { 0, 1, 2, 3, 4, 5, 7, 9 };

static int pb_cost(int score) {
    if (score < POINTBUY_MIN) return 0;
    if (score > POINTBUY_MAX) return PB_COST[7];
    return PB_COST[score - POINTBUY_MIN];
}

static int pb_total_spent(void) {
    int total = 0;
    for (int i = 0; i < ABILITY_COUNT; i++)
        total += pb_cost(g_builder.abilities[i]);
    return total;
}

// Standard-Array Werte
static const int STD_ARRAY[ABILITY_COUNT] = { 15, 14, 13, 12, 10, 8 };

// ---- Zustand ---------------------------------------------------------------
static int s_cursor      = 0;  // 0-5, welches Attribut ausgewaehlt
static int s_grab_active = 0;  // 1 wenn ein Wert "gehalten" wird (Standard-Array)

static UiButton s_btn_std;
static UiButton s_btn_pb;
static UiButton s_btn_man;
static UiButton s_btn_next;
static UiButton s_btn_back;

// ---- Initialisierung -------------------------------------------------------

void step_abilities_init(void) {
    s_cursor     = 0;
    s_grab_active = 0;

    // Wenn Standard-Array: Initialwerte setzen falls noch nicht vergeben
    if (g_builder.ability_method == ABILITY_METHOD_STANDARD) {
        // Pruefe ob Werte noch dem STD-Array entsprechen
        int already_set = 0;
        for (int i = 0; i < ABILITY_COUNT; i++) {
            if (g_builder.abilities[i] != STD_ARRAY[i]) { already_set = 1; break; }
            if (g_builder.abilities[i] != 10) { already_set = 1; break; }
        }
        if (!already_set) {
            for (int i = 0; i < ABILITY_COUNT; i++)
                g_builder.abilities[i] = STD_ARRAY[i];
        }
    }

    // Methoden-Buttons
    s_btn_std = (UiButton){ 4,   28, 98, 22, "Standard",  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_pb  = (UiButton){ 108, 28, 98, 22, "Punkt-Kauf",COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_man = (UiButton){ 214, 28, 98, 22, "Manuell",   COLOR_BTN_NORMAL, COLOR_TEXT, 0 };

    s_btn_back = (UiButton){ 4,   202, 100, 26, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_next = (UiButton){ 210, 202, 106, 26, "Weiter >",  COLOR_ACCENT,     COLOR_TEXT, 0 };
}

// ---- Methoden-Wechsel -------------------------------------------------------
static void switch_method(int method) {
    if (g_builder.ability_method == method) return;
    g_builder.ability_method = method;

    if (method == ABILITY_METHOD_STANDARD) {
        for (int i = 0; i < ABILITY_COUNT; i++)
            g_builder.abilities[i] = STD_ARRAY[i];
    } else if (method == ABILITY_METHOD_POINTBUY) {
        for (int i = 0; i < ABILITY_COUNT; i++)
            g_builder.abilities[i] = POINTBUY_MIN; // alle auf 8
    } else {
        // Manuell: unveraendert lassen, nur falls alle noch 8 sind → auf 10
        int all_eight = 1;
        for (int i = 0; i < ABILITY_COUNT; i++)
            if (g_builder.abilities[i] != 8) { all_eight = 0; break; }
        if (all_eight)
            for (int i = 0; i < ABILITY_COUNT; i++)
                g_builder.abilities[i] = 10;
    }
    s_grab_active = 0;
}

// ---- Standard-Array: Wert an Position `cursor` tauschen mit nächstem -------
static void std_shift(int direction) {
    // Finde welche Ability den naechsten Wert hat
    int cur_val = g_builder.abilities[s_cursor];
    int cur_arr_idx = -1;
    for (int i = 0; i < ABILITY_COUNT; i++)
        if (STD_ARRAY[i] == cur_val) { cur_arr_idx = i; break; }
    if (cur_arr_idx < 0) return;

    // Naechster Wert im Standard-Array (zirkulaer)
    int next_arr_idx = (cur_arr_idx + direction + ABILITY_COUNT) % ABILITY_COUNT;
    int next_val = STD_ARRAY[next_arr_idx];

    // Suche die Ability die diesen Wert haelt
    int swap_with = -1;
    for (int i = 0; i < ABILITY_COUNT; i++) {
        if (g_builder.abilities[i] == next_val) { swap_with = i; break; }
    }
    if (swap_with < 0 || swap_with == s_cursor) return;

    // Tauschen
    g_builder.abilities[s_cursor] = next_val;
    g_builder.abilities[swap_with] = cur_val;
}

// ---- Point-Buy: Wert aendern ------------------------------------------------
static void pb_change(int delta) {
    int val     = g_builder.abilities[s_cursor];
    int new_val = val + delta;

    if (new_val < POINTBUY_MIN || new_val > POINTBUY_MAX) return;

    int spent = pb_total_spent();
    int cost_diff = pb_cost(new_val) - pb_cost(val);

    if (spent + cost_diff > POINTBUY_BUDGET) return; // kein Budget

    g_builder.abilities[s_cursor] = new_val;
}

// ---- Top-Screen: Vorschau aller Attributwerte ------------------------------

void step_abilities_draw_top(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Schritt 3/6: Attribute");
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    // Methoden-Label
    const char* method_label = "Standard-Array";
    if (g_builder.ability_method == ABILITY_METHOD_POINTBUY)  method_label = "Punkt-Kauf";
    if (g_builder.ability_method == ABILITY_METHOD_MANUAL)    method_label = "Manuelle Eingabe";
    ui_draw_textf(10, 34, 0.5f, 0.42f, COLOR_TEXT_DIM,
                  "Methode: %s", method_label);

    // Punkt-Kauf Restpunkte
    if (g_builder.ability_method == ABILITY_METHOD_POINTBUY) {
        int remaining = POINTBUY_BUDGET - pb_total_spent();
        u32 col = remaining > 0 ? COLOR_GOLD : COLOR_HP_GREEN;
        ui_draw_textf(230, 34, 0.5f, 0.42f, col,
                      "Budget: %d", remaining);
    }

    ui_draw_separator(0, 50, SCREEN_TOP_W, COLOR_TEXT_DIM);

    // Spalten-Header (kompakter wenn HG-Boni vorhanden)
    int has_bg_asi = 0;
    for (int i = 0; i < ABILITY_COUNT; i++)
        if (g_builder.bg_asi[i] != 0) { has_bg_asi = 1; break; }

    ui_draw_text(10,  56, 0.5f, 0.38f, COLOR_TEXT_DIM, "Attr");
    ui_draw_text(80,  56, 0.5f, 0.38f, COLOR_TEXT_DIM, "Basis");
    ui_draw_text(130, 56, 0.5f, 0.38f, COLOR_TEXT_DIM, "Rasse");
    if (has_bg_asi)
        ui_draw_text(180, 56, 0.5f, 0.38f, COLOR_HP_GREEN, "HG");
    ui_draw_text(has_bg_asi ? 225 : 195, 56, 0.5f, 0.38f, COLOR_TEXT_DIM, "Ges.");
    ui_draw_text(has_bg_asi ? 278 : 248, 56, 0.5f, 0.38f, COLOR_TEXT_DIM, "Mod");

    // Sechs Attributs-Zeilen
    for (int i = 0; i < ABILITY_COUNT; i++) {
        float y = 70 + i * 26;
        int base      = g_builder.abilities[i];
        int bg_bonus  = g_builder.bg_asi[i];
        int final_val = builder_final_ability(i);
        int race_bonus = final_val - base - bg_bonus;
        int mod       = dnd_modifier(final_val);

        if (i == s_cursor)
            ui_draw_rect(0, y - 2, SCREEN_TOP_W, 24, COLOR_PANEL2);

        u32 text_col = (i == s_cursor) ? COLOR_GOLD : COLOR_TEXT;

        ui_draw_textf(10,  y, 0.5f, 0.42f, text_col, "%s", ABILITY_SHORT[i]);
        ui_draw_textf(80,  y, 0.5f, 0.42f, text_col, "%2d", base);

        if (race_bonus != 0)
            ui_draw_textf(130, y, 0.5f, 0.42f, COLOR_ACCENT2, "%+d", race_bonus);
        else
            ui_draw_textf(130, y, 0.5f, 0.42f, COLOR_TEXT_DIM, "--");

        if (has_bg_asi) {
            if (bg_bonus > 0)
                ui_draw_textf(180, y, 0.5f, 0.42f, COLOR_HP_GREEN, "+%d", bg_bonus);
            else
                ui_draw_textf(180, y, 0.5f, 0.42f, COLOR_TEXT_DIM, "--");
        }

        ui_draw_textf(has_bg_asi ? 225 : 195, y, 0.5f, 0.48f, text_col, "%2d", final_val);
        ui_draw_textf(has_bg_asi ? 278 : 248, y, 0.5f, 0.42f,
                      mod >= 0 ? COLOR_HP_GREEN : COLOR_HP_RED, "%+d", mod);
    }

    // Hilfe unten
    ui_draw_separator(0, 232, SCREEN_TOP_W, COLOR_TEXT_DIM);
    if (g_builder.ability_method == ABILITY_METHOD_STANDARD)
        ui_draw_text(10, 234, 0.5f, 0.32f, COLOR_TEXT_DIM,
                     "[L/R] Wert tauschen  [DPad hoch/runter] Attribut waehlen");
    else if (g_builder.ability_method == ABILITY_METHOD_POINTBUY)
        ui_draw_text(10, 234, 0.5f, 0.32f, COLOR_TEXT_DIM,
                     "[L/R] Wert +/-  [DPad hoch/runter] Attribut waehlen");
    else
        ui_draw_text(10, 234, 0.5f, 0.32f, COLOR_TEXT_DIM,
                     "[A] Eingabe  [DPad hoch/runter] Attribut waehlen");
}

// ---- Bottom-Screen: Methoden-Buttons + Navigation --------------------------

void step_abilities_draw_bottom(void) {
    ui_draw_rect(0, 0, SCREEN_BTM_W, 28, COLOR_PANEL);
    ui_draw_textf(8, 6, 0.5f, 0.45f, COLOR_TEXT_DIM, "Methode:");
    ui_draw_separator(0, 28, SCREEN_BTM_W, COLOR_ACCENT);

    // Methoden-Buttons hervorheben
    s_btn_std.color_bg = (g_builder.ability_method == ABILITY_METHOD_STANDARD)
                         ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    s_btn_pb.color_bg  = (g_builder.ability_method == ABILITY_METHOD_POINTBUY)
                         ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    s_btn_man.color_bg = (g_builder.ability_method == ABILITY_METHOD_MANUAL)
                         ? COLOR_ACCENT : COLOR_BTN_NORMAL;

    ui_button_draw(&s_btn_std);
    ui_button_draw(&s_btn_pb);
    ui_button_draw(&s_btn_man);

    ui_draw_separator(0, 54, SCREEN_BTM_W, COLOR_TEXT_DIM);

    // Mini-Attributliste unten auf dem Touch-Screen
    // Zeigt Wert + Up/Down Tippmoegleichkeit
    for (int i = 0; i < ABILITY_COUNT; i++) {
        float y = 58 + i * 22;
        int base = g_builder.abilities[i];
        u32 row_bg = (i == s_cursor) ? COLOR_PANEL2 : COLOR_TRANSPARENT;
        if (i == s_cursor)
            ui_draw_rect(0, y - 1, SCREEN_BTM_W, 22, COLOR_PANEL2);

        u32 col = (i == s_cursor) ? COLOR_GOLD : COLOR_TEXT;
        ui_draw_textf(8,   y + 3, 0.5f, 0.42f, col, "%-3s", ABILITY_SHORT[i]);
        ui_draw_textf(56,  y + 3, 0.5f, 0.42f, col, "%2d", base);

        // Links/Rechts-Tasten-Hint bei ausgewaehltem Attribut
        if (i == s_cursor) {
            if (g_builder.ability_method != ABILITY_METHOD_MANUAL) {
                ui_draw_text(120, y + 3, 0.5f, 0.42f, COLOR_TEXT_DIM, "[ L -    R + ]");
            } else {
                ui_draw_text(120, y + 3, 0.5f, 0.42f, COLOR_TEXT_DIM, "[ A = Eingabe ]");
            }
        }
        (void)row_bg;
    }

    ui_draw_separator(0, 196, SCREEN_BTM_W, COLOR_TEXT_DIM);
    ui_button_draw(&s_btn_back);
    ui_button_draw(&s_btn_next);
    ui_draw_text(4, 230, 0.5f, 0.32f, COLOR_TEXT_DIM,
                 "[DPad] Nav  [L/R] Aendern  [A] Eingabe");
}

// ---- Update ----------------------------------------------------------------

void step_abilities_update(u32 keys_down, u32 keys_held,
                           touchPosition* touch, int touch_down) {
    (void)keys_held;

    if (keys_down & KEY_B) { builder_prev_step(); return; }

    // Cursor-Navigation
    if (keys_down & KEY_DOWN) {
        s_cursor = (s_cursor + 1) % ABILITY_COUNT;
    }
    if (keys_down & KEY_UP) {
        s_cursor = (s_cursor - 1 + ABILITY_COUNT) % ABILITY_COUNT;
    }

    // Wert aendern
    if (g_builder.ability_method == ABILITY_METHOD_STANDARD) {
        if (keys_down & KEY_R) std_shift(+1);
        if (keys_down & KEY_L) std_shift(-1);
        // DPad links/rechts auch
        if (keys_down & KEY_RIGHT) std_shift(+1);
        if (keys_down & KEY_LEFT)  std_shift(-1);
    } else if (g_builder.ability_method == ABILITY_METHOD_POINTBUY) {
        if (keys_down & KEY_R || keys_down & KEY_RIGHT) pb_change(+1);
        if (keys_down & KEY_L || keys_down & KEY_LEFT)  pb_change(-1);
    } else {
        // Manuell: A oeffnet Tastatur
        if (keys_down & KEY_A) {
            int val = g_builder.abilities[s_cursor];
            if (ui_keyboard_input_numeric(ABILITY_SHORT[s_cursor], &val, 1, 20))
                g_builder.abilities[s_cursor] = val;
        }
    }

    // Weiter
    if (keys_down & KEY_START) {
        builder_next_step();
        return;
    }

    if (!touch_down) return;

    // Methoden-Buttons
    if (ui_button_touched(&s_btn_std, touch)) { switch_method(ABILITY_METHOD_STANDARD); return; }
    if (ui_button_touched(&s_btn_pb,  touch)) { switch_method(ABILITY_METHOD_POINTBUY); return; }
    if (ui_button_touched(&s_btn_man, touch)) { switch_method(ABILITY_METHOD_MANUAL);   return; }

    // Attribut-Zeile antippen
    for (int i = 0; i < ABILITY_COUNT; i++) {
        float y = 58 + i * 22;
        if (ui_point_in_rect((float)touch->px, (float)touch->py,
                             0, y - 1, SCREEN_BTM_W, 22)) {
            if (s_cursor == i) {
                // Selbes Attribut: Eingabe starten
                if (g_builder.ability_method == ABILITY_METHOD_MANUAL) {
                    int val = g_builder.abilities[s_cursor];
                    if (ui_keyboard_input_numeric(ABILITY_SHORT[s_cursor], &val, 1, 20))
                        g_builder.abilities[s_cursor] = val;
                } else if (g_builder.ability_method == ABILITY_METHOD_POINTBUY) {
                    // Tap rechte Haelfte = +, linke = -
                    if (touch->px > SCREEN_BTM_W / 2)
                        pb_change(+1);
                    else
                        pb_change(-1);
                } else {
                    if (touch->px > SCREEN_BTM_W / 2)
                        std_shift(+1);
                    else
                        std_shift(-1);
                }
            } else {
                s_cursor = i;
            }
            return;
        }
    }

    if (ui_button_touched(&s_btn_next, touch)) { builder_next_step(); return; }
    if (ui_button_touched(&s_btn_back, touch)) { builder_prev_step(); return; }
}
