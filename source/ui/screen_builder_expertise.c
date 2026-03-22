// screen_builder_expertise.c – Expertise-Wahl im Builder (Schurke Stufe 1)
// Erscheint nur fuer Schurken: Waehle 2 Skills mit doppeltem Uebungsbonus.

#include "screen_builder.h"
#include "ui_core.h"
#include "../data/srd_classes.h"
#include "../utils/dnd_rules.h"
#include <stdio.h>
#include <string.h>

#define EXPERTISE_COUNT_ROGUE 2

static UiList   s_exp_list;
static UiButton s_btn_back;
static UiButton s_btn_next;

static void refresh_list(void) {
    ui_list_init(&s_exp_list, 4, 28, 312, 172, 22);
    for (int i = 0; i < SKILL_COUNT; i++) {
        int prof    = g_builder.chosen_skills[i];
        int expert  = g_builder.chosen_expertise[i];
        char row[128];
        if (!prof) {
            // Skill nicht proficient – nicht waehlbar fuer Expertise
            snprintf(row, sizeof(row), "[ ] %s (keine Uebung)", SKILL_NAMES[i]);
            ui_list_set_item(&s_exp_list, i, row, COLOR_TEXT_DIM);
        } else {
            snprintf(row, sizeof(row), "[%s] %s", expert ? "E" : " ", SKILL_NAMES[i]);
            ui_list_set_item(&s_exp_list, i, row, expert ? COLOR_GOLD : COLOR_TEXT);
        }
    }
}

void step_expertise_init(void) {
    memset(g_builder.chosen_expertise, 0, sizeof(g_builder.chosen_expertise));
    g_builder.expertise_count = 0;
    refresh_list();
    s_btn_back = (UiButton){ 4,   206, 100, 26, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    s_btn_next = (UiButton){ 210, 206, 106, 26, "Weiter >",  COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
}

void step_expertise_draw_top(void) {
    ui_draw_rect(0, 0, SCREEN_TOP_W, 28, COLOR_PANEL);
    ui_draw_textf(10, 6, 0.5f, 0.50f, COLOR_GOLD, "Schritt 3/%d: Expertise", BUILDER_VISIBLE_STEPS);
    ui_draw_separator(0, 28, SCREEN_TOP_W, COLOR_ACCENT);

    ui_draw_textf(10, 34, 0.5f, 0.42f, COLOR_TEXT,
                  "Schurke: Waehle %d Fertigkeiten fuer Expertise",
                  EXPERTISE_COUNT_ROGUE);
    ui_draw_textf(10, 50, 0.5f, 0.38f, COLOR_TEXT_DIM,
                  "Expertise verdoppelt deinen Uebungsbonus fuer diese Fertigkeiten.");
    ui_draw_textf(10, 64, 0.5f, 0.38f, COLOR_TEXT_DIM,
                  "Du kannst nur Fertigkeiten waehlen, in denen du bereits Uebung hast.");

    ui_draw_separator(0, 80, SCREEN_TOP_W, COLOR_ACCENT);
    ui_draw_textf(10, 86, 0.5f, 0.40f,
                  g_builder.expertise_count >= EXPERTISE_COUNT_ROGUE ? COLOR_HP_GREEN : COLOR_ACCENT2,
                  "Expertise gewaehlt: %d / %d",
                  g_builder.expertise_count, EXPERTISE_COUNT_ROGUE);

    // Ausgewaehlte Skills anzeigen
    int y = 104;
    for (int i = 0; i < SKILL_COUNT && y < 235; i++) {
        if (g_builder.chosen_expertise[i]) {
            ui_draw_textf(10, y, 0.5f, 0.38f, COLOR_GOLD, "* %s", SKILL_NAMES[i]);
            y += 14;
        }
    }
}

void step_expertise_draw_bottom(void) {
    ui_list_draw(&s_exp_list);
    s_btn_next.color_bg = (g_builder.expertise_count >= EXPERTISE_COUNT_ROGUE)
                          ? COLOR_ACCENT : COLOR_BTN_NORMAL;
    ui_button_draw(&s_btn_back);
    ui_button_draw(&s_btn_next);
    ui_draw_text(4, 232, 0.5f, 0.30f, COLOR_TEXT_DIM,
                 "[A] Expertise  [DPad] Scrollen  [B] Zurück");
}

void step_expertise_update(u32 keys_down, u32 keys_held,
                           touchPosition* touch, int touch_down) {
    (void)keys_held;

    if (keys_down & KEY_UP)   ui_list_scroll(&s_exp_list, -1);
    if (keys_down & KEY_DOWN) ui_list_scroll(&s_exp_list,  1);

    if (keys_down & KEY_A) {
        int sel = s_exp_list.selected;
        if (sel >= 0 && sel < SKILL_COUNT && g_builder.chosen_skills[sel]) {
            if (g_builder.chosen_expertise[sel]) {
                // Abwaehlen
                g_builder.chosen_expertise[sel] = 0;
                g_builder.expertise_count--;
            } else if (g_builder.expertise_count < EXPERTISE_COUNT_ROGUE) {
                g_builder.chosen_expertise[sel] = 1;
                g_builder.expertise_count++;
            }
            refresh_list();
        }
    }

    if (keys_down & KEY_B) { builder_prev_step(); return; }
    if (keys_down & KEY_START && g_builder.expertise_count >= EXPERTISE_COUNT_ROGUE) {
        builder_next_step(); return;
    }

    if (!touch_down) return;

    ui_list_handle_touch(&s_exp_list, touch, touch_down);

    if (ui_button_touched(&s_btn_back, touch)) {
        builder_prev_step();
    } else if (ui_button_touched(&s_btn_next, touch) &&
               g_builder.expertise_count >= EXPERTISE_COUNT_ROGUE) {
        builder_next_step();
    }
}
