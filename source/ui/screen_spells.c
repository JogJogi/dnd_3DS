#include "screen_manager.h"
#include "ui_core.h"
#include "../db/spell_db.h"
#include "../models/spell.h"
#include "../data/spell_reference.h"
#include <stdio.h>
#include <string.h>

// ---- State ------------------------------------------------------------------
static Spell      s_spells[SPELLS_MAX];
static int        s_spell_count = 0;
static SpellSlots s_slots;

// Tabs: 0=Vorbereitet, 1=Alle, 2=Slots, 3=Zauberbuch
static int  s_tab = 0;

// Gefilterte Liste (Tabs 0 & 1): Indizes in s_spells
static int  s_filt[SPELLS_MAX];
static int  s_filt_count = 0;
static UiList s_list;

// Zauberbuch (Tab 3)
#define BOOK_FILT_COUNT 8
static const int   s_bfilt_level[BOOK_FILT_COUNT] = { -1, 0, 1, 2, 3, 4, 5, 6 };
static const char* s_bfilt_label[BOOK_FILT_COUNT] = { "Alle", "T", "1", "2", "3", "4", "5", "6" };

static int    s_book_level = -1;   // -1=alle, 0=Trick, 1-6=Grad
static int    s_book_filt[UI_LIST_MAX_ITEMS];
static int    s_book_filt_count = 0;
static UiList s_book_list;
static UiButton s_btn_book_filt[BOOK_FILT_COUNT];
static UiButton s_btn_book_action;  // "+ Lernen" oder "- Vergessen"

// Buttons (geteilt)
static UiButton s_btn_tab[4];
static UiButton s_btn_back;
static UiButton s_btn_add;
static UiButton s_btn_prepare;
static UiButton s_btn_delete;
static UiButton s_btn_long_rest;
static UiButton s_btn_slot_minus[9];
static UiButton s_btn_slot_plus[9];
static UiButton s_btn_slot_max[9];

// ---- Hilfsfunktionen --------------------------------------------------------

static void rebuild_filter(void) {
    s_filt_count = 0;
    for (int i = 0; i < s_spell_count && s_filt_count < SPELLS_MAX; i++) {
        if (s_tab == 0) {
            if (s_spells[i].spell_level == 0 || s_spells[i].prepared)
                s_filt[s_filt_count++] = i;
        } else {
            s_filt[s_filt_count++] = i;
        }
    }
}

static void rebuild_list(void) {
    s_list.count = 0;
    for (int i = 0; i < s_filt_count; i++) {
        const Spell* sp = &s_spells[s_filt[i]];
        char flags[6] = "";
        if (sp->concentration) strncat(flags, "K", sizeof(flags) - 1);
        if (sp->ritual)        strncat(flags, "R", sizeof(flags) - 1);

        char buf[128];
        if (sp->spell_level == 0)
            snprintf(buf, sizeof(buf), "[T] %-22s %s", sp->name, flags);
        else
            snprintf(buf, sizeof(buf), "[%d] %-22s %s", sp->spell_level, sp->name, flags);

        u32 col;
        if (sp->spell_level == 0)  col = COLOR_GOLD;
        else if (sp->prepared)     col = COLOR_TEXT;
        else                       col = COLOR_TEXT_DIM;

        ui_list_set_item(&s_list, i, buf, col);
    }
    s_list.count = s_filt_count;
}

static void rebuild_book_filter(void) {
    s_book_filt_count = 0;
    for (int i = 0; i < SPELL_REF_COUNT && s_book_filt_count < UI_LIST_MAX_ITEMS; i++) {
        if (s_book_level < 0 || SPELL_REFS[i].level == (uint8_t)s_book_level)
            s_book_filt[s_book_filt_count++] = i;
    }

    s_book_list.count = 0;
    for (int i = 0; i < s_book_filt_count; i++) {
        const SpellRef* sr = &SPELL_REFS[s_book_filt[i]];

        // Prüfe ob schon bekannt
        int known = 0;
        for (int j = 0; j < s_spell_count; j++) {
            if (strcmp(s_spells[j].name, sr->name) == 0) { known = 1; break; }
        }

        char lvl_str[4];
        if (sr->level == 0) snprintf(lvl_str, sizeof(lvl_str), "T");
        else                snprintf(lvl_str, sizeof(lvl_str), "%d", sr->level);

        char buf[80];
        snprintf(buf, sizeof(buf), "[%s] %-24s%s%s",
                 lvl_str, sr->name,
                 sr->concentration ? "K" : "",
                 sr->ritual        ? "R" : "");

        u32 col = known ? COLOR_HP_GREEN : COLOR_TEXT;
        ui_list_set_item(&s_book_list, i, buf, col);
    }
    s_book_list.count = s_book_filt_count;
}

static void reload_data(void) {
    if (g_active_char_id <= 0) return;
    s_spell_count = spells_db_load(g_active_char_id, s_spells, SPELLS_MAX);
    spell_slots_db_load(g_active_char_id, &s_slots);
    s_slots.character_id = g_active_char_id;
    rebuild_filter();
    rebuild_list();
    if (s_list.selected >= s_filt_count)
        s_list.selected = s_filt_count > 0 ? s_filt_count - 1 : -1;
    rebuild_book_filter();
}

// ---- Pips zeichnen ----------------------------------------------------------
static void draw_slot_bar(float x, float y, int avail, int total) {
    if (total == 0) {
        ui_draw_text(x, y, 0.5f, 0.38f, COLOR_TEXT_DIM, "--");
        return;
    }
    char buf[11] = {0};
    int  show = total < 10 ? total : 10;
    for (int i = 0; i < show; i++) buf[i] = (i < avail) ? 'O' : '-';
    if (total > 10) buf[9] = '+';
    u32 col = avail > 0 ? COLOR_HP_GREEN : COLOR_HP_RED;
    ui_draw_text(x, y, 0.5f, 0.40f, col, buf);
}

// ---- on_enter / on_exit -----------------------------------------------------
static void on_enter(void) {
    // 4 Tab-Buttons (je 73px, 3px Abstand, Start bei x=4)
    float tw = 73.0f, tg = 3.0f, tx = 4.0f;
    s_btn_tab[0] = (UiButton){ tx + 0*(tw+tg), 4, tw, 22, "Vorbereitet",  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_tab[1] = (UiButton){ tx + 1*(tw+tg), 4, tw, 22, "Alle",          COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_tab[2] = (UiButton){ tx + 2*(tw+tg), 4, tw, 22, "Slots",         COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_tab[3] = (UiButton){ tx + 3*(tw+tg), 4, tw, 22, "Zauberbuch",    COLOR_BTN_NORMAL, COLOR_TEXT, 0 };

    // Spell-Liste (Tabs 0 & 1)
    ui_list_init(&s_list, 4, 30, 312, 170, 18);

    // Aktionsbuttons (Tabs 0 & 1)
    s_btn_back    = (UiButton){   4, 210,  70, 22, "< Zurueck",   COLOR_BTN_NORMAL, COLOR_TEXT,     0 };
    s_btn_add     = (UiButton){  80, 210,  50, 22, "+ Neu",        COLOR_ACCENT,     COLOR_TEXT,     0 };
    s_btn_prepare = (UiButton){ 136, 210,  84, 22, "Vorb.an/aus",  COLOR_BTN_NORMAL, COLOR_HP_GREEN, 0 };
    s_btn_delete  = (UiButton){ 226, 210,  84, 22, "Loeschen",     COLOR_ACCENT,     COLOR_TEXT,     0 };

    // Slots-Tab
    s_btn_long_rest = (UiButton){ 4, 210, 130, 22, "Langer Rast",  COLOR_BTN_NORMAL, COLOR_GOLD, 0 };
    for (int i = 0; i < 9; i++) {
        float y = 30.0f + i * 20.0f;
        s_btn_slot_minus[i] = (UiButton){ 158, y, 22, 16, "-",   COLOR_ACCENT,     COLOR_TEXT,     0 };
        s_btn_slot_plus[i]  = (UiButton){ 183, y, 22, 16, "+",   COLOR_HP_GREEN,   COLOR_TEXT,     0 };
        s_btn_slot_max[i]   = (UiButton){ 210, y, 40, 16, "Max", COLOR_BTN_NORMAL, COLOR_TEXT_DIM, 0 };
    }

    // Zauberbuch-Tab: 8 Level-Filter-Buttons (je 37px, 2px Abstand, Start x=2)
    for (int i = 0; i < BOOK_FILT_COUNT; i++) {
        float bx = 2.0f + i * (37.0f + 2.0f);
        s_btn_book_filt[i] = (UiButton){ bx, 30, 37, 16,
                                          s_bfilt_label[i],
                                          COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    }
    // Liste und Aktion-Button
    ui_list_init(&s_book_list, 4, 50, 312, 148, 16);
    s_btn_book_action = (UiButton){ 218, 206, 98, 22, "+ Lernen", COLOR_HP_GREEN, COLOR_TEXT, 0 };

    s_tab        = 0;
    s_book_level = -1;
    reload_data();
}

static void on_exit(void) {}

// ---- update -----------------------------------------------------------------
static void do_toggle_prepared(void) {
    if (s_list.selected < 0 || s_list.selected >= s_filt_count) return;
    Spell* sp = &s_spells[s_filt[s_list.selected]];
    if (sp->spell_level == 0) return;
    sp->prepared = !sp->prepared;
    spells_db_set_prepared(sp->id, sp->prepared);
    rebuild_filter();
    rebuild_list();
}

static void update(u32 kd, u32 kh, touchPosition* t, int td) {
    (void)kh;
    if (kd & KEY_B) { screen_pop(); return; }

    // L/R: Tabs wechseln
    if (kd & KEY_L) {
        if (s_tab > 0) {
            s_tab--;
            rebuild_filter(); rebuild_list();
            s_list.selected = -1; s_list.scroll_offset = 0;
        }
    }
    if (kd & KEY_R) {
        if (s_tab < 3) {
            s_tab++;
            rebuild_filter(); rebuild_list();
            s_list.selected = -1; s_list.scroll_offset = 0;
        }
    }

    // D-Pad Scroll
    if (s_tab < 2) {
        if (kd & KEY_DUP)   ui_list_scroll(&s_list, -1);
        if (kd & KEY_DDOWN) ui_list_scroll(&s_list,  1);
        if (kd & KEY_A)     do_toggle_prepared();
    }
    if (s_tab == 3) {
        if (kd & KEY_DUP)   ui_list_scroll(&s_book_list, -1);
        if (kd & KEY_DDOWN) ui_list_scroll(&s_book_list,  1);
    }

    if (!td) return;

    // Tab-Buttons (Touch)
    for (int i = 0; i < 4; i++) {
        if (ui_button_touched(&s_btn_tab[i], t)) {
            if (s_tab != i) {
                s_tab = i;
                rebuild_filter(); rebuild_list();
                s_list.selected = -1; s_list.scroll_offset = 0;
            }
            return;
        }
    }

    if (ui_button_touched(&s_btn_back, t)) { screen_pop(); return; }

    if (s_tab < 2) {
        // Spell-Liste Touch
        ui_list_handle_touch(&s_list, t, td);

        if (ui_button_touched(&s_btn_add, t)) {
            if (g_active_char_id <= 0) return;
            Spell ns;
            memset(&ns, 0, sizeof(ns));
            ns.character_id = g_active_char_id;
            if (!ui_keyboard_input("Zauber-Name:", ns.name, SPELL_NAME_MAX)) return;
            ui_keyboard_input_numeric("Grad (0=Trick, 1-9):", &ns.spell_level, 0, 9);
            ui_keyboard_input("Schule:", ns.school, SPELL_TEXT_MAX);
            ui_keyboard_input("Wirkzeit:", ns.casting_time, SPELL_TEXT_MAX);
            ui_keyboard_input("Reichweite:", ns.range, SPELL_TEXT_MAX);
            ui_keyboard_input("Dauer:", ns.duration, SPELL_TEXT_MAX);
            ui_keyboard_input("Beschreibung:", ns.description, SPELL_DESC_MAX);
            spells_db_save(&ns);
            reload_data();
            return;
        }

        if (ui_button_touched(&s_btn_prepare, t)) {
            do_toggle_prepared();
            return;
        }

        if (ui_button_touched(&s_btn_delete, t)) {
            if (s_list.selected >= 0 && s_list.selected < s_filt_count) {
                spells_db_delete(s_spells[s_filt[s_list.selected]].id);
                reload_data();
                if (s_list.selected >= s_filt_count)
                    s_list.selected = s_filt_count - 1;
            }
            return;
        }

    } else if (s_tab == 2) {
        // Slots-Tab
        if (ui_button_touched(&s_btn_long_rest, t)) {
            spellslots_restore_all(&s_slots);
            spell_slots_db_save(&s_slots);
            return;
        }
        for (int i = 0; i < 9; i++) {
            if (ui_button_touched(&s_btn_slot_minus[i], t)) {
                if (s_slots.used[i] < s_slots.total[i]) {
                    s_slots.used[i]++;
                    spell_slots_db_save(&s_slots);
                }
                return;
            }
            if (ui_button_touched(&s_btn_slot_plus[i], t)) {
                if (s_slots.used[i] > 0) {
                    s_slots.used[i]--;
                    spell_slots_db_save(&s_slots);
                }
                return;
            }
            if (ui_button_touched(&s_btn_slot_max[i], t)) {
                int val = s_slots.total[i];
                if (ui_keyboard_input_numeric("Max. Slots (Grad):", &val, 0, 9)) {
                    s_slots.total[i] = val;
                    if (s_slots.used[i] > s_slots.total[i])
                        s_slots.used[i] = s_slots.total[i];
                    spell_slots_db_save(&s_slots);
                }
                return;
            }
        }

    } else {
        // Zauberbuch-Tab
        // Level-Filter-Buttons
        for (int i = 0; i < BOOK_FILT_COUNT; i++) {
            if (ui_button_touched(&s_btn_book_filt[i], t)) {
                s_book_level = s_bfilt_level[i];
                s_book_list.selected     = -1;
                s_book_list.scroll_offset = 0;
                rebuild_book_filter();
                return;
            }
        }

        ui_list_handle_touch(&s_book_list, t, td);

        // Lernen / Vergessen
        if (ui_button_touched(&s_btn_book_action, t)) {
            int sel = s_book_list.selected;
            if (sel >= 0 && sel < s_book_filt_count && g_active_char_id > 0) {
                const SpellRef* sr = &SPELL_REFS[s_book_filt[sel]];

                // Finde DB-ID falls bekannt
                int spell_id = 0;
                for (int i = 0; i < s_spell_count; i++) {
                    if (strcmp(s_spells[i].name, sr->name) == 0) {
                        spell_id = s_spells[i].id;
                        break;
                    }
                }

                if (spell_id > 0) {
                    // Bekannt -> Vergessen
                    spells_db_delete(spell_id);
                } else {
                    // Unbekannt -> Lernen
                    Spell ns;
                    memset(&ns, 0, sizeof(ns));
                    ns.character_id = g_active_char_id;
                    strncpy(ns.name,         sr->name,        SPELL_NAME_MAX - 1);
                    ns.spell_level = sr->level;
                    strncpy(ns.school,       sr->school,      SPELL_TEXT_MAX - 1);
                    strncpy(ns.casting_time, sr->cast_time,   SPELL_TEXT_MAX - 1);
                    strncpy(ns.range,        sr->range,       SPELL_TEXT_MAX - 1);
                    strncpy(ns.duration,     sr->duration,    SPELL_TEXT_MAX - 1);
                    strncpy(ns.description,  sr->description, SPELL_DESC_MAX - 1);
                    ns.ritual        = sr->ritual;
                    ns.concentration = sr->concentration;
                    ns.prepared      = (sr->level == 0) ? 1 : 0; // Tricks immer aktiv
                    spells_db_save(&ns);
                }
                reload_data();
            }
            return;
        }
    }
}

// ---- draw_top ---------------------------------------------------------------
static void draw_top(void) {
    ui_draw_text(10, 4, 0.5f, 0.55f, COLOR_GOLD, "Zauber");

    // Kompakte Slot-Übersicht (3 Spalten)
    ui_draw_rect(0, 21, SCREEN_TOP_W, 1, COLOR_ACCENT);
    ui_draw_text(6, 24, 0.5f, 0.34f, COLOR_TEXT_DIM, "Slots:");
    for (int lv = 1; lv <= 9; lv++) {
        int   col_i = (lv - 1) % 3;
        int   row_i = (lv - 1) / 3;
        float sx    = 50.0f + col_i * 117.0f;
        float sy    = 24.0f + row_i * 13.0f;
        int   avail = s_slots.total[lv-1] - s_slots.used[lv-1];
        if (avail < 0) avail = 0;
        u32 lc = s_slots.total[lv-1] > 0 ? COLOR_TEXT : COLOR_TEXT_DIM;
        ui_draw_textf(sx,      sy, 0.5f, 0.35f, lc, "L%d:", lv);
        draw_slot_bar(sx + 24, sy, avail, s_slots.total[lv-1]);
    }
    ui_draw_rect(0, 64, SCREEN_TOP_W, 1, COLOR_ACCENT);

    if (s_tab < 2) {
        // ---- Ausgewählter Zauber ----
        int sel = s_list.selected;
        if (sel >= 0 && sel < s_filt_count) {
            const Spell* sp = &s_spells[s_filt[sel]];

            ui_draw_text(10, 68, 0.5f, 0.55f, COLOR_GOLD, sp->name);

            char sub[96];
            if (sp->spell_level == 0)
                snprintf(sub, sizeof(sub), "Zaubertrick%s%s",
                         sp->school[0] ? " * " : "", sp->school);
            else
                snprintf(sub, sizeof(sub), "Grad %d%s%s", sp->spell_level,
                         sp->school[0] ? " * " : "", sp->school);
            ui_draw_text(10, 87, 0.5f, 0.38f, COLOR_TEXT_DIM, sub);

            float y = 102.0f;
            if (sp->casting_time[0]) { ui_draw_textf(10, y, 0.5f, 0.37f, COLOR_TEXT, "Zeit: %s",    sp->casting_time); y += 13; }
            if (sp->range[0])        { ui_draw_textf(10, y, 0.5f, 0.37f, COLOR_TEXT, "Reichw.: %s", sp->range);        y += 13; }
            if (sp->duration[0])     { ui_draw_textf(10, y, 0.5f, 0.37f, COLOR_TEXT, "Dauer: %s",   sp->duration);     y += 13; }
            if (sp->components[0])   { ui_draw_textf(10, y, 0.5f, 0.37f, COLOR_TEXT, "Komp.: %s",   sp->components);   y += 13; }

            char flags[64] = "";
            if (sp->concentration)                strncat(flags, "[Konz.]",   sizeof(flags) - 1);
            if (sp->ritual)                       strncat(flags, " [Ritual]", sizeof(flags) - 1);
            if (sp->prepared && sp->spell_level > 0) strncat(flags, " [Vorb.]", sizeof(flags) - 1);
            if (flags[0]) { ui_draw_text(10, y, 0.5f, 0.35f, COLOR_ACCENT2, flags); y += 12; }

            if (sp->description[0] && y < 220.0f) {
                char desc[140];
                snprintf(desc, sizeof(desc), "%.128s%s", sp->description,
                         strlen(sp->description) > 128 ? "..." : "");
                ui_draw_text(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, desc);
            }
        } else {
            ui_draw_text(100, 140, 0.5f, 0.42f, COLOR_TEXT_DIM,
                         s_filt_count > 0 ? "Zauber auswaehlen" : "Keine Zauber");
        }

    } else if (s_tab == 2) {
        // ---- Slots-Hilfe ----
        ui_draw_text(10,  72, 0.5f, 0.42f, COLOR_TEXT,     "Zauberschlitze verwalten");
        ui_draw_text(10,  90, 0.5f, 0.37f, COLOR_TEXT_DIM, "[-] Slot benutzen (Zauber wirken)");
        ui_draw_text(10, 103, 0.5f, 0.37f, COLOR_TEXT_DIM, "[+] Slot wiederherst. (kurze Rast)");
        ui_draw_text(10, 116, 0.5f, 0.37f, COLOR_TEXT_DIM, "[Max] Maximum fuer diesen Grad setzen");
        ui_draw_text(10, 129, 0.5f, 0.37f, COLOR_TEXT_DIM, "Langer Rast: alle Slots zurueck");
        ui_draw_text(10, 148, 0.5f, 0.35f, COLOR_TEXT_DIM, "O = verfuegbar    - = benutzt");

    } else {
        // ---- Zauberbuch: Referenz-Detail ----
        int sel = s_book_list.selected;
        if (sel >= 0 && sel < s_book_filt_count) {
            const SpellRef* sr = &SPELL_REFS[s_book_filt[sel]];

            int known = 0;
            for (int i = 0; i < s_spell_count; i++) {
                if (strcmp(s_spells[i].name, sr->name) == 0) { known = 1; break; }
            }

            u32 name_col = known ? COLOR_HP_GREEN : COLOR_GOLD;
            ui_draw_text(10, 68, 0.5f, 0.55f, name_col, sr->name);
            if (known)
                ui_draw_text(280, 68, 0.5f, 0.33f, COLOR_HP_GREEN, "[Bekannt]");

            char sub[80];
            if (sr->level == 0)
                snprintf(sub, sizeof(sub), "Zaubertrick * %s", sr->school);
            else
                snprintf(sub, sizeof(sub), "Grad %d * %s", sr->level, sr->school);
            ui_draw_text(10, 87, 0.5f, 0.38f, COLOR_TEXT_DIM, sub);

            float y = 102.0f;
            if (sr->cast_time[0]) { ui_draw_textf(10, y, 0.5f, 0.37f, COLOR_TEXT, "Zeit: %s",    sr->cast_time); y += 13; }
            if (sr->range[0])     { ui_draw_textf(10, y, 0.5f, 0.37f, COLOR_TEXT, "Reichw.: %s", sr->range);     y += 13; }
            if (sr->duration[0])  { ui_draw_textf(10, y, 0.5f, 0.37f, COLOR_TEXT, "Dauer: %s",   sr->duration);  y += 13; }

            char flags[64] = "";
            if (sr->concentration) strncat(flags, "[Konz.]",  sizeof(flags) - 1);
            if (sr->ritual)        strncat(flags, " [Ritual]", sizeof(flags) - 1);
            if (flags[0]) { ui_draw_text(10, y, 0.5f, 0.35f, COLOR_ACCENT2, flags); y += 12; }

            if (sr->description[0] && y < 220.0f) {
                // Wrap auf 2 Zeilen bei langer Beschreibung
                int len = (int)strlen(sr->description);
                if (len <= 60) {
                    ui_draw_text(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, sr->description);
                } else {
                    // Erste Zeile (bis ~60 Zeichen am nächsten Leerzeichen)
                    char line[80];
                    int cut = 60;
                    while (cut > 0 && sr->description[cut] != ' ') cut--;
                    if (cut == 0) cut = 60;
                    snprintf(line, sizeof(line), "%.*s", cut, sr->description);
                    ui_draw_text(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, line);
                    y += 12;
                    // Zweite Zeile
                    if (y < 220.0f) {
                        snprintf(line, sizeof(line), "%.72s%s",
                                 sr->description + cut + 1,
                                 len - cut - 1 > 72 ? "..." : "");
                        ui_draw_text(10, y, 0.5f, 0.35f, COLOR_TEXT_DIM, line);
                    }
                }
            }
        } else {
            ui_draw_text(80, 140, 0.5f, 0.42f, COLOR_TEXT_DIM,
                         s_book_filt_count > 0 ? "Zauber auswaehlen" : "Keine Treffer");
        }
    }

    ui_draw_text(10, 225, 0.5f, 0.35f, COLOR_TEXT_DIM,
                 "[L/R] Tab  [A] Vorb.  [Up/Dn] Scroll  [B] Zurueck");
}

// ---- draw_bottom ------------------------------------------------------------
static void draw_bottom(void) {
    // Tab-Buttons
    for (int i = 0; i < 4; i++) {
        s_btn_tab[i].color_bg = (s_tab == i) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
        ui_button_draw(&s_btn_tab[i]);
    }

    if (s_tab < 2) {
        // Spell-Liste + Aktionsbuttons
        ui_list_draw(&s_list);
        ui_button_draw(&s_btn_back);
        ui_button_draw(&s_btn_add);
        ui_button_draw(&s_btn_prepare);
        ui_button_draw(&s_btn_delete);

    } else if (s_tab == 2) {
        // Slot-Verwaltung: 9 Zeilen
        for (int i = 0; i < 9; i++) {
            float y     = 30.0f + i * 20.0f;
            int   avail = s_slots.total[i] - s_slots.used[i];
            if (avail < 0) avail = 0;
            u32 lc = s_slots.total[i] > 0 ? COLOR_TEXT : COLOR_TEXT_DIM;

            ui_draw_textf(10,  y + 2, 0.5f, 0.40f, lc, "L%d", i + 1);
            draw_slot_bar(35,  y + 2, avail, s_slots.total[i]);
            ui_draw_textf(138, y + 2, 0.5f, 0.38f, lc, "%d/%d", avail, s_slots.total[i]);

            ui_button_draw(&s_btn_slot_minus[i]);
            ui_button_draw(&s_btn_slot_plus[i]);
            ui_button_draw(&s_btn_slot_max[i]);
        }
        ui_button_draw(&s_btn_back);
        ui_button_draw(&s_btn_long_rest);

    } else {
        // Zauberbuch-Tab
        // Level-Filter-Buttons
        for (int i = 0; i < BOOK_FILT_COUNT; i++) {
            s_btn_book_filt[i].color_bg =
                (s_bfilt_level[i] == s_book_level) ? COLOR_ACCENT : COLOR_BTN_NORMAL;
            ui_button_draw(&s_btn_book_filt[i]);
        }

        ui_list_draw(&s_book_list);
        ui_button_draw(&s_btn_back);

        // Lernen/Vergessen-Button: Farbe + Label je nach Status
        int sel = s_book_list.selected;
        int known = 0;
        if (sel >= 0 && sel < s_book_filt_count) {
            const SpellRef* sr = &SPELL_REFS[s_book_filt[sel]];
            for (int i = 0; i < s_spell_count; i++) {
                if (strcmp(s_spells[i].name, sr->name) == 0) { known = 1; break; }
            }
        }
        s_btn_book_action.color_bg = known ? COLOR_ACCENT  : COLOR_HP_GREEN;
        s_btn_book_action.label    = known ? "- Vergessen" : "+ Lernen";
        ui_button_draw(&s_btn_book_action);
    }
}

Screen g_screen_spells = {
    .id          = SCREEN_SPELLS,
    .on_enter    = on_enter,
    .on_exit     = on_exit,
    .update      = update,
    .draw_top    = draw_top,
    .draw_bottom = draw_bottom
};
