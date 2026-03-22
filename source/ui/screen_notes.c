#include "screen_manager.h"
#include "ui_core.h"
#include "../db/notes_db.h"
#include <stdio.h>
#include <string.h>

#define NOTES_DISPLAY_MAX 32

static Note   s_notes[NOTES_DISPLAY_MAX];
static int    s_note_count = 0;
static UiList s_list;
static int    s_view_mode = 0;   // 0 = Liste, 1 = Detail

static UiButton s_btn_back;
static UiButton s_btn_add;
static UiButton s_btn_delete;
static UiButton s_btn_view;

static void reload(void) {
    s_note_count = notes_db_load(g_active_char_id, -1, s_notes, NOTES_DISPLAY_MAX);
    if (s_note_count < 0) s_note_count = 0;

    ui_list_init(&s_list, 4, 28, 312, 174, 24);
    for (int i = 0; i < s_note_count; i++) {
        char label[128];
        snprintf(label, sizeof(label), "[%s] %s",
                 NOTE_CAT_NAMES[s_notes[i].category], s_notes[i].title);
        ui_list_set_item(&s_list, i, label, COLOR_TEXT);
    }
}

static void on_enter(void) {
    reload();
    s_view_mode = 0;
    s_btn_back   = (UiButton){   4, 208, 74, 26, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_add    = (UiButton){  82, 208, 74, 26, "+ Neu",     COLOR_ACCENT,     COLOR_TEXT, 0 };
    s_btn_delete = (UiButton){ 160, 208, 74, 26, "Loeschen",  COLOR_ACCENT,     COLOR_TEXT, 0 };
    s_btn_view   = (UiButton){ 238, 208, 78, 26, "Anzeigen",  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
}

static void on_exit(void) {}

static void update(u32 kd, u32 kh, touchPosition* t, int td) {
    (void)kh;
    if (kd & KEY_B) {
        if (s_view_mode) { s_view_mode = 0; return; }
        screen_pop(); return;
    }
    if (kd & KEY_DOWN) ui_list_scroll(&s_list, 1);
    if (kd & KEY_UP)   ui_list_scroll(&s_list, -1);

    if (!td) return;
    ui_list_handle_touch(&s_list, t, td);

    if (ui_button_touched(&s_btn_back, t)) {
        if (s_view_mode) s_view_mode = 0; else screen_pop();
        return;
    }
    if (ui_button_touched(&s_btn_view, t) && s_list.selected >= 0) {
        s_view_mode = 1;
        return;
    }
    if (ui_button_touched(&s_btn_add, t)) {
        Note new_note = {0};
        new_note.character_id = g_active_char_id;
        if (ui_keyboard_input("Notiz-Titel:", new_note.title, NOTE_TITLE_MAX)) {
            ui_keyboard_input("Inhalt:", new_note.content, NOTE_CONTENT_MAX);
            notes_db_save(&new_note);
            reload();
        }
        return;
    }
    if (ui_button_touched(&s_btn_delete, t) && s_list.selected >= 0) {
        notes_db_delete(s_notes[s_list.selected].id);
        reload();
        return;
    }
}

static void draw_top(void) {
    // Header
    ui_draw_rect(0, 0, SCREEN_TOP_W, 26, COLOR_PANEL);
    ui_draw_text(10, 5, 0.5f, 0.58f, COLOR_GOLD, "Notizen");
    if (s_view_mode && s_list.selected >= 0 && s_list.selected < s_note_count)
        ui_draw_text(320, 7, 0.5f, 0.36f, COLOR_TEXT_DIM, "[B] Liste");
    ui_draw_separator(0, 26, SCREEN_TOP_W, COLOR_ACCENT);

    if (s_view_mode && s_list.selected >= 0 && s_list.selected < s_note_count) {
        Note* n = &s_notes[s_list.selected];
        // Notiz-Titel + Kategorie-Badge
        ui_draw_textf(10, 30, 0.5f, 0.52f, COLOR_TEXT, "%s", n->title);
        ui_draw_textf(320, 32, 0.5f, 0.35f, COLOR_TEXT_DIM, "[%s]", NOTE_CAT_NAMES[n->category]);
        ui_draw_separator(0, 50, SCREEN_TOP_W, COLOR_TEXT_DIM);

        // Inhalt mehrzeilig anzeigen
        const char* content = n->content;
        float y = 56.0f;
        char line[80];
        int ci = 0, li = 0;
        while (content[ci] && y < 222.0f) {
            if (content[ci] == '\n' || li >= 78) {
                line[li] = '\0';
                ui_draw_text(10, y, 0.5f, 0.40f, COLOR_TEXT, line);
                y += 16.0f; li = 0;
                if (content[ci] == '\n') ci++;
            } else {
                line[li++] = content[ci++];
            }
        }
        if (li > 0) {
            line[li] = '\0';
            ui_draw_text(10, y, 0.5f, 0.40f, COLOR_TEXT, line);
        }
    } else {
        // Vorschau der ausgewählten Notiz
        if (s_note_count == 0) {
            ui_draw_text(20, 100, 0.5f, 0.48f, COLOR_TEXT_DIM, "Noch keine Notizen.");
            ui_draw_text(20, 125, 0.5f, 0.40f, COLOR_TEXT_DIM, "Erstelle eine neue Notiz.");
        } else if (s_list.selected >= 0 && s_list.selected < s_note_count) {
            Note* n = &s_notes[s_list.selected];
            ui_draw_textf(10, 32, 0.5f, 0.50f, COLOR_GOLD, "%s", n->title);
            ui_draw_textf(10, 52, 0.5f, 0.36f, COLOR_TEXT_DIM, "[%s]", NOTE_CAT_NAMES[n->category]);
            ui_draw_separator(0, 68, SCREEN_TOP_W, COLOR_TEXT_DIM);
            // Erste paar Zeilen als Vorschau
            const char* content = n->content;
            float y = 74.0f;
            char line[80];
            int ci = 0, li = 0, rows = 0;
            while (content[ci] && y < 200.0f && rows < 6) {
                if (content[ci] == '\n' || li >= 78) {
                    line[li] = '\0';
                    ui_draw_text(10, y, 0.5f, 0.38f, COLOR_TEXT, line);
                    y += 14.0f; li = 0; rows++;
                    if (content[ci] == '\n') ci++;
                } else {
                    line[li++] = content[ci++];
                }
            }
            if (li > 0 && rows < 6) {
                line[li] = '\0';
                ui_draw_text(10, y, 0.5f, 0.38f, COLOR_TEXT, line);
            }
        }
        ui_draw_separator(0, 224, SCREEN_TOP_W, COLOR_TEXT_DIM);
        ui_draw_text(10, 228, 0.5f, 0.34f, COLOR_TEXT_DIM,
                     "[A] Anzeigen  [DPad] Navigieren  [B] Zurueck");
    }
}

static void draw_bottom(void) {
    // Header
    ui_draw_rect(0, 0, SCREEN_BTM_W, 26, COLOR_PANEL);
    ui_draw_textf(8, 5, 0.5f, 0.42f, COLOR_TEXT_DIM,
                  "Notizen (%d)", s_note_count);
    ui_draw_separator(0, 26, SCREEN_BTM_W, COLOR_ACCENT);

    if (!s_view_mode) ui_list_draw(&s_list);

    ui_draw_separator(0, 204, SCREEN_BTM_W, COLOR_TEXT_DIM);
    ui_button_draw(&s_btn_back);
    ui_button_draw(&s_btn_add);
    s_btn_delete.color_bg = (s_list.selected >= 0 && s_note_count > 0)
                            ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_delete);
    s_btn_view.color_bg = (s_list.selected >= 0 && !s_view_mode)
                          ? COLOR_BTN_NORMAL : COLOR_TEXT_DIM;
    ui_button_draw(&s_btn_view);
}

Screen g_screen_notes = {
    .id          = SCREEN_NOTES,
    .on_enter    = on_enter,
    .on_exit     = on_exit,
    .update      = update,
    .draw_top    = draw_top,
    .draw_bottom = draw_bottom
};
