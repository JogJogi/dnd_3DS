// screen_builder_spells.c – Zauber- und Cantrip-Auswahl im Charakter-Builder
// Erscheint nur fuer Zauberwirker-Klassen nach der Skill-Auswahl.

#include "screen_builder.h"
#include "ui_core.h"
#include "../data/srd_classes.h"
#include "../data/srd_spells_bard.h"
#include <stdio.h>
#include <string.h>

// ---- Tab: 0 = Zaubertricks, 1 = Bekannte Zauber ----------------------------
static int s_tab;  // 0 = Cantrips, 1 = Zauber Grad 1

// ---- Listen und Buttons -----------------------------------------------------
static UiList   s_spell_list;
static UiButton s_btn_cantrip_tab;
static UiButton s_btn_spell_tab;
static UiButton s_btn_back;
static UiButton s_btn_next;

// ---- Hilfsfunktionen --------------------------------------------------------

// Gibt Anzahl erlaubter Cantrips fuer die aktuelle Klasse zurueck
static int allowed_cantrips(void) {
    if (g_builder.class_idx < 0) return 0;
    return SRD_CLASSES[g_builder.class_idx].cantrips_known;
}

// Gibt Anzahl erlaubter bekannter Zauber fuer die aktuelle Klasse zurueck
static int allowed_spells(void) {
    if (g_builder.class_idx < 0) return 0;
    return SRD_CLASSES[g_builder.class_idx].spells_known;
}

// Prueft ob ein Cantrip-Index gewaehlt ist
static int cantrip_chosen(int idx) {
    for (int i = 0; i < g_builder.chosen_cantrip_count; i++)
        if (g_builder.chosen_cantrip_idx[i] == idx) return 1;
    return 0;
}

// Prueft ob ein Zauber-Index gewaehlt ist
static int spell_chosen(int idx) {
    for (int i = 0; i < g_builder.chosen_spell_count; i++)
        if (g_builder.chosen_spell_idx[i] == idx) return 1;
    return 0;
}

// Liefert Anzahl Eintraege fuer aktuelle Liste (je nach Klasse)
static int list_entry_count(void) {
    if (g_builder.class_idx < 0) return 0;
    const char* class_id = SRD_CLASSES[g_builder.class_idx].id;
    if (strcmp(class_id, "bard") == 0) {
        return (s_tab == 0) ? BARD_CANTRIP_COUNT : BARD_SPELL_COUNT;
    }
    // Weitere Klassen folgen hier
    return 0;
}

// Gibt Name des Zaubers/Cantrips fuer Index i zurueck
static const char* entry_name(int i) {
    if (g_builder.class_idx < 0) return "";
    const char* class_id = SRD_CLASSES[g_builder.class_idx].id;
    if (strcmp(class_id, "bard") == 0) {
        if (s_tab == 0)
            return (i < BARD_CANTRIP_COUNT) ? BARD_CANTRIP_LIST[i].name : "";
        else
            return (i < BARD_SPELL_COUNT)   ? BARD_SPELL_LIST[i].name   : "";
    }
    return "";
}

// Gibt Kurzbeschreibung zurueck
static const char* entry_desc(int i) {
    if (g_builder.class_idx < 0) return "";
    const char* class_id = SRD_CLASSES[g_builder.class_idx].id;
    if (strcmp(class_id, "bard") == 0) {
        if (s_tab == 0)
            return (i < BARD_CANTRIP_COUNT) ? BARD_CANTRIP_LIST[i].description : "";
        else
            return (i < BARD_SPELL_COUNT)   ? BARD_SPELL_LIST[i].description   : "";
    }
    return "";
}

static int entry_level(int i) {
    if (g_builder.class_idx < 0) return 0;
    const char* class_id = SRD_CLASSES[g_builder.class_idx].id;
    if (strcmp(class_id, "bard") == 0) {
        if (s_tab == 0) return 0;
        return (i < BARD_SPELL_COUNT) ? BARD_SPELL_LIST[i].level : 0;
    }
    return 0;
}

static int entry_conc(int i) {
    if (g_builder.class_idx < 0) return 0;
    const char* class_id = SRD_CLASSES[g_builder.class_idx].id;
    if (strcmp(class_id, "bard") == 0) {
        if (s_tab == 0)
            return (i < BARD_CANTRIP_COUNT) ? BARD_CANTRIP_LIST[i].concentration : 0;
        else
            return (i < BARD_SPELL_COUNT)   ? BARD_SPELL_LIST[i].concentration   : 0;
    }
    return 0;
}

// Rebuild der Liste
static void refresh_list(void) {
    int n = list_entry_count();
    int max_chosen = (s_tab == 0) ? allowed_cantrips() : allowed_spells();

    ui_list_init(&s_spell_list, 4, 28, 312, 172, 22);

    for (int i = 0; i < n && i < UI_LIST_MAX_ITEMS; i++) {
        int chosen = (s_tab == 0) ? cantrip_chosen(i) : spell_chosen(i);
        int lvl    = entry_level(i);
        char row[128];
        if (s_tab == 0)
            snprintf(row, sizeof(row), "[%s] %s", chosen ? "X" : " ", entry_name(i));
        else
            snprintf(row, sizeof(row), "[%s] Gr.%d %s", chosen ? "X" : " ", lvl, entry_name(i));

        u32 col = chosen ? COLOR_GOLD : COLOR_TEXT;
        ui_list_set_item(&s_spell_list, i, row, col);
    }

    (void)max_chosen;
}

// Cantrip umschalten
static void toggle_cantrip(int idx) {
    if (idx < 0 || idx >= BARD_CANTRIP_COUNT) return;
    int max = allowed_cantrips();
    if (cantrip_chosen(idx)) {
        // Abwaehlen: Index aus Array entfernen
        for (int i = 0; i < g_builder.chosen_cantrip_count; i++) {
            if (g_builder.chosen_cantrip_idx[i] == idx) {
                g_builder.chosen_cantrip_idx[i] =
                    g_builder.chosen_cantrip_idx[--g_builder.chosen_cantrip_count];
                break;
            }
        }
    } else if (g_builder.chosen_cantrip_count < max &&
               g_builder.chosen_cantrip_count < BUILDER_MAX_CANTRIPS) {
        g_builder.chosen_cantrip_idx[g_builder.chosen_cantrip_count++] = idx;
    }
    refresh_list();
}

// Zauber umschalten
static void toggle_spell(int idx) {
    if (idx < 0 || idx >= BARD_SPELL_COUNT) return;
    int max = allowed_spells();
    if (spell_chosen(idx)) {
        for (int i = 0; i < g_builder.chosen_spell_count; i++) {
            if (g_builder.chosen_spell_idx[i] == idx) {
                g_builder.chosen_spell_idx[i] =
                    g_builder.chosen_spell_idx[--g_builder.chosen_spell_count];
                break;
            }
        }
    } else if (g_builder.chosen_spell_count < max &&
               g_builder.chosen_spell_count < BUILDER_MAX_SPELLS) {
        g_builder.chosen_spell_idx[g_builder.chosen_spell_count++] = idx;
    }
    refresh_list();
}

// Prueft ob Auswahl vollstaendig ist
static int selection_complete(void) {
    return (g_builder.chosen_cantrip_count >= allowed_cantrips() &&
            g_builder.chosen_spell_count   >= allowed_spells());
}

// ============================================================================
// OEFFENTLICHE SCHRITT-FUNKTIONEN (aufgerufen von screen_builder.c)
// ============================================================================

void step_spells_init(void) {
    s_tab = 0;  // Beginne mit Cantrips

    // Buttons
    s_btn_cantrip_tab = (UiButton){ 4,   2,  152, 22, "Zaubertricks",
                                    COLOR_ACCENT, COLOR_TEXT, 0 };
    s_btn_spell_tab   = (UiButton){ 162, 2,  152, 22, "Bekannte Zauber",
                                    COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_back = (UiButton){ 4,   206, 100, 26, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_next = (UiButton){ 210, 206, 106, 26, "Weiter >",  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };

    refresh_list();
}

void step_spells_draw_top(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);

    int can_done   = (g_builder.chosen_cantrip_count >= allowed_cantrips());
    int spell_done = (g_builder.chosen_spell_count   >= allowed_spells());

    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD,
                  "Schritt 5/%d: Zauber waehlen", BUILDER_VISIBLE_STEPS);
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    // Status-Zeile
    ui_draw_textf(10, 32, 0.5f, 0.40f, can_done ? COLOR_HP_GREEN : COLOR_ACCENT2,
                  "Zaubertricks: %d/%d gewaehlt",
                  g_builder.chosen_cantrip_count, allowed_cantrips());
    ui_draw_textf(10, 46, 0.5f, 0.40f, spell_done ? COLOR_HP_GREEN : COLOR_ACCENT2,
                  "Grad-1-Zauber: %d/%d gewaehlt",
                  g_builder.chosen_spell_count, allowed_spells());

    ui_draw_separator(0, 62, SCREEN_TOP_W, COLOR_TEXT_DIM);

    // Detailansicht des ausgewaehlten Eintrags
    int sel = s_spell_list.selected;
    int n   = list_entry_count();
    if (sel >= 0 && sel < n) {
        const char* name = entry_name(sel);
        const char* desc = entry_desc(sel);
        int conc         = entry_conc(sel);

        ui_draw_textf(10, 66, 0.5f, 0.48f, COLOR_TEXT, "%s", name);
        if (conc)
            ui_draw_text(280, 70, 0.5f, 0.34f, COLOR_ACCENT2, "[K]");

        // Beschreibung umbrechen
        int y = 84;
        int maxw = 76;  // ca. Zeichen pro Zeile bei Size 0.35
        int dlen = (int)strlen(desc);
        int start = 0;
        while (start < dlen && y < 230) {
            int end = start + maxw;
            if (end >= dlen) {
                ui_draw_textf(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, "%s", desc + start);
                break;
            }
            while (end > start && desc[end] != ' ') end--;
            if (end == start) end = start + maxw;
            char line[128];
            int len = end - start;
            if (len >= (int)sizeof(line)) len = (int)sizeof(line) - 1;
            snprintf(line, sizeof(line), "%.*s", len, desc + start);
            ui_draw_textf(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, "%s", line);
            y += 13;
            start = end + 1;
        }
    } else {
        ui_draw_text(10, 80, 0.5f, 0.40f, COLOR_TEXT_DIM, "Waehle einen Eintrag fuer Details.");
    }
}

void step_spells_draw_bottom(void) {
    // Tab-Buttons oben
    s_btn_cantrip_tab.color_bg = (s_tab == 0) ? COLOR_ACCENT     : COLOR_BTN_NORMAL;
    s_btn_spell_tab.color_bg   = (s_tab == 1) ? COLOR_ACCENT     : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_cantrip_tab);
    ui_button_draw(&s_btn_spell_tab);

    // Scrollbare Liste
    ui_list_draw(&s_spell_list);

    // Fortschritts-Text
    int max_chosen = (s_tab == 0) ? allowed_cantrips() : allowed_spells();
    int cur_chosen = (s_tab == 0) ? g_builder.chosen_cantrip_count : g_builder.chosen_spell_count;
    ui_draw_textf(4, 200, 0.5f, 0.36f, COLOR_TEXT_DIM,
                  "Gewaehlt: %d / %d", cur_chosen, max_chosen);

    // Navigations-Buttons
    s_btn_next.color_bg = selection_complete() ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_back);
    ui_button_draw(&s_btn_next);

    ui_draw_text(4, 232, 0.5f, 0.30f, COLOR_TEXT_DIM,
                 "[A] Wählen  [L/R] Tab  [B] Zurück");
}

void step_spells_update(u32 keys_down, u32 keys_held,
                        touchPosition* touch, int touch_down) {
    (void)keys_held;

    // Tab wechseln
    if (keys_down & KEY_L) {
        s_tab = 0;
        s_spell_list.selected = 0;
        s_spell_list.scroll_offset = 0;
        refresh_list();
    }
    if (keys_down & KEY_R) {
        s_tab = 1;
        s_spell_list.selected = 0;
        s_spell_list.scroll_offset = 0;
        refresh_list();
    }

    // Liste scrollen
    if (keys_down & KEY_UP)   ui_list_scroll(&s_spell_list, -1);
    if (keys_down & KEY_DOWN) ui_list_scroll(&s_spell_list,  1);

    // Auswahl umschalten
    if (keys_down & KEY_A) {
        int sel = s_spell_list.selected;
        if (sel >= 0) {
            if (s_tab == 0) toggle_cantrip(sel);
            else            toggle_spell(sel);
        }
    }

    // Touch-Behandlung (Liste + Buttons)
    if (touch_down) {
        // Tab-Buttons
        if (ui_button_touched(&s_btn_cantrip_tab, touch)) {
            s_tab = 0;
            s_spell_list.selected = 0;
            s_spell_list.scroll_offset = 0;
            refresh_list();
        } else if (ui_button_touched(&s_btn_spell_tab, touch)) {
            s_tab = 1;
            s_spell_list.selected = 0;
            s_spell_list.scroll_offset = 0;
            refresh_list();
        } else if (ui_button_touched(&s_btn_back, touch)) {
            builder_prev_step();
        } else if (ui_button_touched(&s_btn_next, touch) && selection_complete()) {
            builder_next_step();
        } else {
            // Liste-Touch
            int prev = s_spell_list.selected;
            ui_list_handle_touch(&s_spell_list, touch, touch_down);
            if (s_spell_list.selected != prev && s_spell_list.selected >= 0) {
                // Einzelner Touch: Element selektieren
                // Doppelklick (Touch auf bereits selektiertes): umschalten
                if (s_spell_list.selected == prev) {
                    if (s_tab == 0) toggle_cantrip(prev);
                    else            toggle_spell(prev);
                }
            }
        }
    }

    // Weiter/Zurueck via Buttons
    if (keys_down & KEY_START && selection_complete())
        builder_next_step();
    if (keys_down & KEY_B)
        builder_prev_step();
}
