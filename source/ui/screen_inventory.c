#include "screen_manager.h"
#include "ui_core.h"
#include "../db/inventory_db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // atof

// ---- Zustand ---------------------------------------------------------------
static Item     s_items[INVENTORY_MAX];
static int      s_item_count = 0;
static Currency s_currency;
static UiList   s_list;
static float    s_total_weight = 0.0f;

typedef enum { MODE_LIST = 0, MODE_EDIT } InvMode;
static InvMode s_mode     = MODE_LIST;
static int     s_edit_idx = -1;

// ---- Buttons: Listen-Modus -------------------------------------------------
static UiButton s_btn_back;
static UiButton s_btn_add;
static UiButton s_btn_delete;
static UiButton s_btn_edit_open;
static UiButton s_btn_toggle_equip;

// Währungs-Buttons (Labels dynamisch)
static char     s_lbl_cp[12], s_lbl_sp[12], s_lbl_ep[12], s_lbl_gp[12], s_lbl_pp[12];
static UiButton s_btn_cur_cp, s_btn_cur_sp, s_btn_cur_ep, s_btn_cur_gp, s_btn_cur_pp;

// ---- Buttons: Edit-Modus ---------------------------------------------------
static char     s_lbl_en[64];   // Name (gekürzt)
static char     s_lbl_eq[16];   // "Anz: 3"
static char     s_lbl_ew[20];   // "Gew: 1.0 lbs"
static char     s_lbl_ev[20];   // "Wert: 200 KP"
static char     s_lbl_ec[24];   // "Kat: Waffe"
static UiButton s_btn_en, s_btn_eq, s_btn_ew, s_btn_ev, s_btn_ec;
static UiButton s_btn_eequip, s_btn_edesc, s_btn_edone;

// ---- Hilfsfunktionen -------------------------------------------------------
static void refresh_currency_labels(void) {
    snprintf(s_lbl_cp, sizeof(s_lbl_cp), "KP:%d",  s_currency.copper);
    snprintf(s_lbl_sp, sizeof(s_lbl_sp), "SP:%d",  s_currency.silver);
    snprintf(s_lbl_ep, sizeof(s_lbl_ep), "EP:%d",  s_currency.electrum);
    snprintf(s_lbl_gp, sizeof(s_lbl_gp), "GP:%d",  s_currency.gold);
    snprintf(s_lbl_pp, sizeof(s_lbl_pp), "PP:%d",  s_currency.platinum);
    s_btn_cur_cp.label = s_lbl_cp;
    s_btn_cur_sp.label = s_lbl_sp;
    s_btn_cur_ep.label = s_lbl_ep;
    s_btn_cur_gp.label = s_lbl_gp;
    s_btn_cur_pp.label = s_lbl_pp;
}

static void refresh_edit_labels(void) {
    if (s_edit_idx < 0 || s_edit_idx >= s_item_count) return;
    Item* it = &s_items[s_edit_idx];
    snprintf(s_lbl_en, sizeof(s_lbl_en), "%.60s", it->name);
    snprintf(s_lbl_eq, sizeof(s_lbl_eq), "Anz: %d",        it->quantity);
    snprintf(s_lbl_ew, sizeof(s_lbl_ew), "Gew: %.1f lbs",  it->weight);
    snprintf(s_lbl_ev, sizeof(s_lbl_ev), "Wert: %d KP",    it->value_cp);
    snprintf(s_lbl_ec, sizeof(s_lbl_ec), "Kat: %.12s",     ITEM_CAT_NAMES[it->category]);
    s_btn_en.label    = s_lbl_en;
    s_btn_eq.label    = s_lbl_eq;
    s_btn_ew.label    = s_lbl_ew;
    s_btn_ev.label    = s_lbl_ev;
    s_btn_ec.label    = s_lbl_ec;
    s_btn_eequip.label = it->equipped ? "Equip: Ja" : "Equip: Nein";
}

static void reload(void) {
    s_item_count = inventory_db_load(g_active_char_id, s_items, INVENTORY_MAX);
    if (s_item_count < 0) s_item_count = 0;
    currency_db_load(g_active_char_id, &s_currency);
    s_total_weight = inventory_db_total_weight(g_active_char_id);

    // Liste: y=28, h=128 → 5 Items à 24px sichtbar
    ui_list_init(&s_list, 4, 28, 312, 128, 24);
    for (int i = 0; i < s_item_count; i++) {
        char label[128];
        snprintf(label, sizeof(label), "%s x%d (%.1f lbs)",
                 s_items[i].name, s_items[i].quantity, s_items[i].weight);
        u32 color = s_items[i].equipped ? COLOR_GOLD : COLOR_TEXT;
        ui_list_set_item(&s_list, i, label, color);
    }
    refresh_currency_labels();
}

static void init_buttons(void) {
    // Aktions-Zeile (y=162, h=24): [< Zurück | + Neu | Löschen | Bearb. | Equip]
    s_btn_back         = (UiButton){   4, 162, 60, 24, "< Zur.",   COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_add          = (UiButton){  68, 162, 56, 24, "+ Neu",    COLOR_ACCENT,     COLOR_TEXT, 0 };
    s_btn_delete       = (UiButton){ 128, 162, 64, 24, "Loschen",  COLOR_ACCENT,     COLOR_TEXT, 0 };
    s_btn_edit_open    = (UiButton){ 196, 162, 58, 24, "Bearb.",   COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_toggle_equip = (UiButton){ 258, 162, 58, 24, "Equip",    COLOR_BTN_NORMAL, COLOR_GOLD, 0 };

    // Währungs-Buttons (y=190, h=22): 5 × 58px + 4 × 3px gap = 302px, start x=9
    float cx = 9.0f, cy = 190.0f, cw = 58.0f, ch = 22.0f, cg = 3.0f;
    s_btn_cur_cp = (UiButton){ cx + 0*(cw+cg), cy, cw, ch, s_lbl_cp, COLOR_BTN_NORMAL, COLOR_GOLD, 0 };
    s_btn_cur_sp = (UiButton){ cx + 1*(cw+cg), cy, cw, ch, s_lbl_sp, COLOR_BTN_NORMAL, COLOR_GOLD, 0 };
    s_btn_cur_ep = (UiButton){ cx + 2*(cw+cg), cy, cw, ch, s_lbl_ep, COLOR_BTN_NORMAL, COLOR_GOLD, 0 };
    s_btn_cur_gp = (UiButton){ cx + 3*(cw+cg), cy, cw, ch, s_lbl_gp, COLOR_BTN_NORMAL, COLOR_GOLD, 0 };
    s_btn_cur_pp = (UiButton){ cx + 4*(cw+cg), cy, cw, ch, s_lbl_pp, COLOR_BTN_NORMAL, COLOR_GOLD, 0 };

    // Edit-Modus-Buttons (y=26..180): volle Breite, halbe Breite
    float fw = 312.0f, hw = 150.0f, gap = 8.0f, ex = 4.0f;
    s_btn_en    = (UiButton){ ex,         26, fw, 22, s_lbl_en, COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_eq    = (UiButton){ ex,         52, hw, 22, s_lbl_eq, COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_ew    = (UiButton){ ex+hw+gap,  52, hw, 22, s_lbl_ew, COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_ev    = (UiButton){ ex,         78, hw, 22, s_lbl_ev, COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_ec    = (UiButton){ ex+hw+gap,  78, hw, 22, s_lbl_ec, COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_eequip= (UiButton){ ex,        104, hw, 22, "Equip",  COLOR_BTN_NORMAL, COLOR_GOLD, 0 };
    s_btn_edesc = (UiButton){ ex,        130, fw, 22, "Beschreibung bearbeiten",
                               COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_edone = (UiButton){ ex,        162, 110, 24, "Fertig", COLOR_HP_GREEN,  COLOR_TEXT, 0 };
}

// ---- Callbacks -------------------------------------------------------------
static void on_enter(void) {
    s_mode     = MODE_LIST;
    s_edit_idx = -1;
    reload();
    init_buttons();
}

static void inv_on_exit(void) {}

static void update(u32 keys_down, u32 keys_held, touchPosition* touch, int touch_down) {
    (void)keys_held;

    // ---- LISTEN-MODUS -------------------------------------------------------
    if (s_mode == MODE_LIST) {
        if (keys_down & KEY_B) { screen_pop(); return; }
        if (keys_down & KEY_DOWN) ui_list_scroll(&s_list,  1);
        if (keys_down & KEY_UP)   ui_list_scroll(&s_list, -1);

        // [A] = ausgewähltes Item bearbeiten
        if ((keys_down & KEY_A) && s_list.selected >= 0) {
            s_mode     = MODE_EDIT;
            s_edit_idx = s_list.selected;
            refresh_edit_labels();
            return;
        }

        if (!touch_down) return;
        ui_list_handle_touch(&s_list, touch, touch_down);

        if (ui_button_touched(&s_btn_back, touch)) { screen_pop(); return; }

        if (ui_button_touched(&s_btn_add, touch)) {
            Item new_item = {0};
            new_item.character_id = g_active_char_id;
            new_item.quantity = 1;
            if (ui_keyboard_input("Item-Name:", new_item.name, ITEM_NAME_MAX)) {
                inventory_db_save(&new_item);
                reload();
            }
            return;
        }

        if (ui_button_touched(&s_btn_delete, touch) && s_list.selected >= 0) {
            inventory_db_delete(s_items[s_list.selected].id);
            if (s_list.selected >= s_item_count - 1) s_list.selected--;
            reload();
            return;
        }

        if (ui_button_touched(&s_btn_edit_open, touch) && s_list.selected >= 0) {
            s_mode     = MODE_EDIT;
            s_edit_idx = s_list.selected;
            refresh_edit_labels();
            return;
        }

        if (ui_button_touched(&s_btn_toggle_equip, touch) && s_list.selected >= 0) {
            Item* it = &s_items[s_list.selected];
            it->equipped ^= 1;
            inventory_db_save(it);
            reload();
            return;
        }

        // --- Währungs-Buttons ---
        if (ui_button_touched(&s_btn_cur_cp, touch)) {
            int val = s_currency.copper;
            if (ui_keyboard_input_numeric("Kupfermuenzen (KP):", &val, 0, 999999)) {
                s_currency.copper = val;
                currency_db_save(&s_currency);
                refresh_currency_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_cur_sp, touch)) {
            int val = s_currency.silver;
            if (ui_keyboard_input_numeric("Silbermuenzen (SP):", &val, 0, 999999)) {
                s_currency.silver = val;
                currency_db_save(&s_currency);
                refresh_currency_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_cur_ep, touch)) {
            int val = s_currency.electrum;
            if (ui_keyboard_input_numeric("Elektrummuenzen (EP):", &val, 0, 999999)) {
                s_currency.electrum = val;
                currency_db_save(&s_currency);
                refresh_currency_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_cur_gp, touch)) {
            int val = s_currency.gold;
            if (ui_keyboard_input_numeric("Goldmuenzen (GP):", &val, 0, 999999)) {
                s_currency.gold = val;
                currency_db_save(&s_currency);
                refresh_currency_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_cur_pp, touch)) {
            int val = s_currency.platinum;
            if (ui_keyboard_input_numeric("Platinmuenzen (PP):", &val, 0, 999999)) {
                s_currency.platinum = val;
                currency_db_save(&s_currency);
                refresh_currency_labels();
            }
            return;
        }

    // ---- EDIT-MODUS ---------------------------------------------------------
    } else {
        if (keys_down & KEY_B) {
            s_mode = MODE_LIST;
            reload();
            return;
        }

        if (!touch_down) return;
        if (s_edit_idx < 0 || s_edit_idx >= s_item_count) return;
        Item* it = &s_items[s_edit_idx];

        if (ui_button_touched(&s_btn_edone, touch)) {
            s_mode = MODE_LIST;
            reload();
            return;
        }

        if (ui_button_touched(&s_btn_en, touch)) {
            char buf[ITEM_NAME_MAX];
            snprintf(buf, sizeof(buf), "%s", it->name);
            if (ui_keyboard_input("Item-Name:", buf, ITEM_NAME_MAX)) {
                snprintf(it->name, ITEM_NAME_MAX, "%s", buf);
                inventory_db_save(it);
                refresh_edit_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_eq, touch)) {
            int val = it->quantity;
            if (ui_keyboard_input_numeric("Anzahl:", &val, 0, 9999)) {
                it->quantity = val;
                inventory_db_save(it);
                refresh_edit_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_ew, touch)) {
            // Gewicht als Text eingeben (Float)
            char buf[16];
            snprintf(buf, sizeof(buf), "%.1f", it->weight);
            if (ui_keyboard_input("Gewicht (lbs):", buf, sizeof(buf))) {
                it->weight = (float)atof(buf);
                inventory_db_save(it);
                refresh_edit_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_ev, touch)) {
            int val = it->value_cp;
            if (ui_keyboard_input_numeric("Wert (Kupfermuenzen):", &val, 0, 999999)) {
                it->value_cp = val;
                inventory_db_save(it);
                refresh_edit_labels();
            }
            return;
        }
        if (ui_button_touched(&s_btn_ec, touch)) {
            // Kategorie zyklisch wechseln
            it->category = (ItemCategory)((it->category + 1) % ITEM_CAT_COUNT);
            inventory_db_save(it);
            refresh_edit_labels();
            return;
        }
        if (ui_button_touched(&s_btn_eequip, touch)) {
            it->equipped ^= 1;
            inventory_db_save(it);
            refresh_edit_labels();
            return;
        }
        if (ui_button_touched(&s_btn_edesc, touch)) {
            char buf[ITEM_DESC_MAX];
            snprintf(buf, sizeof(buf), "%s", it->description);
            if (ui_keyboard_input("Beschreibung:", buf, ITEM_DESC_MAX)) {
                snprintf(it->description, ITEM_DESC_MAX, "%s", buf);
                inventory_db_save(it);
            }
            return;
        }
    }
}

// ---- Zeichnen Top-Screen ---------------------------------------------------
static void draw_top(void) {
    // Header
    ui_draw_rect(0, 0, SCREEN_TOP_W, 26, COLOR_PANEL);
    ui_draw_text(10, 5, 0.5f, 0.58f, COLOR_GOLD, "Inventar");
    ui_draw_textf(220, 7, 0.5f, 0.40f, COLOR_TEXT_DIM,
                  "Gewicht: %.1f lbs", s_total_weight);
    ui_draw_separator(0, 26, SCREEN_TOP_W, COLOR_ACCENT);

    // Währungs-Panel (immer sichtbar, prominent)
    ui_draw_panel(0, 28, SCREEN_TOP_W, 38, COLOR_PANEL, COLOR_TEXT_DIM);
    ui_draw_textf(10, 34, 0.5f, 0.42f, COLOR_GOLD,
                  "KP:%d   SP:%d   EP:%d",
                  s_currency.copper, s_currency.silver, s_currency.electrum);
    ui_draw_textf(10, 50, 0.5f, 0.42f, COLOR_GOLD,
                  "GP:%d   PP:%d", s_currency.gold, s_currency.platinum);
    // Gesamtwert
    int total_gp = s_currency.copper / 100
                 + s_currency.silver / 10
                 + s_currency.electrum / 2
                 + s_currency.gold
                 + s_currency.platinum * 10;
    ui_draw_textf(280, 34, 0.5f, 0.36f, COLOR_TEXT_DIM, "~%d GP", total_gp);
    ui_draw_separator(0, 66, SCREEN_TOP_W, COLOR_ACCENT);

    // Item-Details
    int sel = (s_mode == MODE_EDIT) ? s_edit_idx : s_list.selected;
    if (sel >= 0 && sel < s_item_count) {
        Item* it = &s_items[sel];
        u32 name_col = s_mode == MODE_EDIT ? COLOR_ACCENT2 : COLOR_TEXT;
        if (it->equipped) {
            ui_draw_rect(0, 70, 4, 22, COLOR_GOLD);  // Equip-Indikator links
        }
        ui_draw_textf(10, 70, 0.5f, 0.56f, name_col, "%s", it->name);
        if (it->equipped)
            ui_draw_text(320, 74, 0.5f, 0.38f, COLOR_GOLD, "[Ang.]");
        ui_draw_textf(10, 94, 0.5f, 0.40f, COLOR_TEXT_DIM,
                      "x%d   %.1f lbs   %d KP   %s",
                      it->quantity, it->weight, it->value_cp,
                      ITEM_CAT_NAMES[it->category]);
        if (it->description[0]) {
            ui_draw_separator(0, 112, SCREEN_TOP_W, COLOR_TEXT_DIM);
            ui_draw_text(10, 116, 0.5f, 0.36f, COLOR_TEXT, it->description);
        }
    } else {
        ui_draw_text(10, 80, 0.5f, 0.42f, COLOR_TEXT_DIM,
                     s_item_count == 0 ? "Inventar ist leer."
                                       : "Item auswaehlen...");
    }

    ui_draw_separator(0, 224, SCREEN_TOP_W, COLOR_TEXT_DIM);
    ui_draw_text(10, 228, 0.5f, 0.33f, COLOR_TEXT_DIM,
                 s_mode == MODE_LIST ? "[A] Bearbeiten  [DPad] Nav  [B] Zurueck"
                                     : "[B] oder Fertig = zurueck");
}

// ---- Zeichnen Bottom-Screen ------------------------------------------------
static void draw_bottom(void) {
    if (s_mode == MODE_LIST) {
        // Header
        ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
        ui_draw_textf(8, 5, 0.5f, 0.42f, COLOR_TEXT_DIM, "Items (%d)", s_item_count);
        ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

        // Inventar-Liste
        ui_list_draw(&s_list);

        // Separator vor Aktions-Buttons
        ui_draw_separator(0, 158, SCREEN_BTM_W, COLOR_TEXT_DIM);

        // Aktions-Buttons
        s_btn_edit_open.color_bg = (s_list.selected >= 0) ? COLOR_BTN_NORMAL : COLOR_TEXT_DIM;
        s_btn_delete.color_bg    = (s_list.selected >= 0) ? COLOR_ACCENT     : COLOR_TEXT_DIM;
        int is_equipped = (s_list.selected >= 0 && s_items[s_list.selected].equipped);
        s_btn_toggle_equip.color_bg  = is_equipped ? COLOR_GOLD       : COLOR_BTN_NORMAL;
        s_btn_toggle_equip.color_text = is_equipped ? COLOR_BLACK : COLOR_GOLD;
        s_btn_toggle_equip.label     = is_equipped ? "Ablegen" : "Equip";
        ui_button_draw(&s_btn_back);
        ui_button_draw(&s_btn_add);
        ui_button_draw(&s_btn_delete);
        ui_button_draw(&s_btn_edit_open);
        ui_button_draw(&s_btn_toggle_equip);

        // Separator vor Währung
        ui_draw_separator(0, 188, SCREEN_BTM_W, COLOR_TEXT_DIM);

        // Währungs-Buttons (tippen zum Bearbeiten)
        refresh_currency_labels();
        ui_button_draw(&s_btn_cur_cp);
        ui_button_draw(&s_btn_cur_sp);
        ui_button_draw(&s_btn_cur_ep);
        ui_button_draw(&s_btn_cur_gp);
        ui_button_draw(&s_btn_cur_pp);

        ui_draw_text(4, 216, 0.5f, 0.32f, COLOR_TEXT_DIM,
                     "[A] Bearbeiten  Waehrung: antippen");

    } else { // MODE_EDIT
        if (s_edit_idx < 0 || s_edit_idx >= s_item_count) return;
        Item* it = &s_items[s_edit_idx];

        // Header
        ui_draw_rect(0, 0, SCREEN_BTM_W, 24, COLOR_PANEL);
        ui_draw_textf(8, 4, 0.5f, 0.44f, COLOR_GOLD, "Bearbeit.: %.22s", it->name);
        ui_draw_separator(0, 24, SCREEN_BTM_W, COLOR_ACCENT);

        refresh_edit_labels();

        ui_button_draw(&s_btn_en);
        ui_button_draw(&s_btn_eq);
        ui_button_draw(&s_btn_ew);
        ui_button_draw(&s_btn_ev);
        ui_button_draw(&s_btn_ec);
        s_btn_eequip.color_bg = it->equipped ? COLOR_HP_GREEN : COLOR_BTN_NORMAL;
        ui_button_draw(&s_btn_eequip);
        ui_button_draw(&s_btn_edesc);
        ui_button_draw(&s_btn_edone);

        ui_draw_separator(0, 188, SCREEN_BTM_W, COLOR_TEXT_DIM);
        ui_draw_text(4, 192, 0.5f, 0.32f, COLOR_TEXT_DIM,
                     "Kategorie: antippen zum Wechseln   [B] Fertig");
    }
}

// ---- Screen-Objekt ---------------------------------------------------------
Screen g_screen_inventory = {
    .id          = SCREEN_INVENTORY,
    .on_enter    = on_enter,
    .on_exit     = inv_on_exit,
    .update      = update,
    .draw_top    = draw_top,
    .draw_bottom = draw_bottom
};
