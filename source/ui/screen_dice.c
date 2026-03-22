#include "screen_manager.h"
#include "ui_core.h"
#include "../utils/dice.h"
#include <stdio.h>
#include <string.h>

#define HISTORY_MAX 8

static int s_last_result  = 0;
static int s_last_sides   = 20;
static int s_modifier     = 0;
static int s_count        = 1;
static int s_history[HISTORY_MAX];
static char s_history_labels[HISTORY_MAX][48];
static int  s_history_top = 0;

static UiButton s_btn_dice[7];   // d4,d6,d8,d10,d12,d20,d100
static UiButton s_btn_back;
static UiButton s_btn_adv;
static UiButton s_btn_dis;
static UiButton s_btn_mod_plus;
static UiButton s_btn_mod_minus;
static UiButton s_btn_cnt_plus;
static UiButton s_btn_cnt_minus;

static void push_history(const char* label, int result) {
    s_history_top = (s_history_top + 1) % HISTORY_MAX;
    s_history[s_history_top] = result;
    snprintf(s_history_labels[s_history_top], 48, "%s", label);
}

static void on_enter(void) {
    // Würfel-Buttons: obere Zeile, 7 Buttons, volle Breite
    static const char* labels[] = {"d4","d6","d8","d10","d12","d20","d100"};
    float bw = 42.0f, gap = 2.0f, sx = 4.0f;
    for (int i = 0; i < 7; i++) {
        s_btn_dice[i] = (UiButton){ sx + i*(bw+gap), 4, bw, 32, labels[i],
                                    COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
    }

    // Modifier-Zeile (im linken Drittel des Panels)
    s_btn_mod_minus = (UiButton){   4, 76, 34, 28, "-",  COLOR_ACCENT,   COLOR_TEXT, 0 };
    s_btn_mod_plus  = (UiButton){  86, 76, 34, 28, "+",  COLOR_HP_GREEN, COLOR_TEXT, 0 };

    // Anzahl-Zeile (im mittleren Drittel des Panels)
    s_btn_cnt_minus = (UiButton){ 124, 76, 34, 28, "-",  COLOR_ACCENT,   COLOR_TEXT, 0 };
    s_btn_cnt_plus  = (UiButton){ 206, 76, 34, 28, "+",  COLOR_HP_GREEN, COLOR_TEXT, 0 };

    // Advantage / Disadvantage
    s_btn_adv = (UiButton){   4, 152, 150, 28, "Vorteil",   COLOR_BTN_NORMAL, COLOR_HP_GREEN, 0 };
    s_btn_dis = (UiButton){ 162, 152, 154, 28, "Nachteil",  COLOR_BTN_NORMAL, COLOR_HP_RED,   0 };

    // Zurück
    s_btn_back = (UiButton){  4, 208, 80, 24, "< Zurueck", COLOR_BTN_NORMAL, COLOR_TEXT, 0 };
}

static void on_exit(void) {}

static void do_roll(int count, int sides, int mod) {
    char expr[32];
    int result = dice_roll_mod(count, sides, mod);
    s_last_result = result;
    s_last_sides  = sides;
    if (mod != 0)
        snprintf(expr, sizeof(expr), "%dd%d%+d", count, sides, mod);
    else
        snprintf(expr, sizeof(expr), "%dd%d", count, sides);
    char label[48];
    snprintf(label, sizeof(label), "%s = %d", expr, result);
    push_history(label, result);
}

static void update(u32 kd, u32 kh, touchPosition* t, int td) {
    (void)kh;
    if (kd & KEY_B) { screen_pop(); return; }

    // Tastatur-Shortcuts: A = d20
    if (kd & KEY_A) { do_roll(s_count, 20, s_modifier); return; }
    if (kd & KEY_X) { do_roll(s_count, s_last_sides, s_modifier); return; }

    if (!td) return;

    // Würfel-Buttons
    static const int sides[] = {4,6,8,10,12,20,100};
    for (int i = 0; i < 7; i++) {
        if (ui_button_touched(&s_btn_dice[i], t)) {
            do_roll(s_count, sides[i], s_modifier);
            return;
        }
    }

    if (ui_button_touched(&s_btn_adv, t)) {
        int a = dice_roll_advantage();
        char label[48];
        snprintf(label, sizeof(label), "1d20 Adv = %d", a);
        s_last_result = a;
        push_history(label, a);
        return;
    }
    if (ui_button_touched(&s_btn_dis, t)) {
        int a = dice_roll_disadvantage();
        char label[48];
        snprintf(label, sizeof(label), "1d20 Dis = %d", a);
        s_last_result = a;
        push_history(label, a);
        return;
    }

    if (ui_button_touched(&s_btn_mod_minus, t)) { s_modifier--; return; }
    if (ui_button_touched(&s_btn_mod_plus,  t)) { s_modifier++; return; }
    if (ui_button_touched(&s_btn_cnt_minus, t) && s_count > 1) { s_count--; return; }
    if (ui_button_touched(&s_btn_cnt_plus,  t) && s_count < 20) { s_count++; return; }
    if (ui_button_touched(&s_btn_back, t)) { screen_pop(); return; }
}

static void draw_top(void) {
    // Header
    ui_draw_rect(0, 0, SCREEN_TOP_W, 24, COLOR_PANEL);
    ui_draw_text(10, 4, 0.5f, 0.58f, COLOR_GOLD, "Wuerfelroller");
    ui_draw_text(10, 228, 0.5f, 0.34f, COLOR_TEXT_DIM, "[A]=d20  [X]=Nochmal  [B]=Zurueck");
    ui_draw_separator(0, 24, SCREEN_TOP_W, COLOR_ACCENT);

    // Letztes Ergebnis sehr groß
    if (s_last_result > 0) {
        char big[16];
        snprintf(big, sizeof(big), "%d", s_last_result);
        float tw = ui_text_width(2.4f, big);
        ui_draw_textf((SCREEN_TOP_W - tw) * 0.5f, 34.0f, 0.5f, 2.4f, COLOR_ACCENT2, "%s", big);
        // Würfelformel darunter
        if (s_history_labels[s_history_top][0]) {
            float fw = ui_text_width(0.44f, s_history_labels[s_history_top]);
            ui_draw_text((SCREEN_TOP_W - fw) * 0.5f, 118.0f, 0.5f, 0.44f,
                         COLOR_TEXT_DIM, s_history_labels[s_history_top]);
        }
    } else {
        float dw = ui_text_width(2.4f, "?");
        ui_draw_textf((SCREEN_TOP_W - dw) * 0.5f, 34.0f, 0.5f, 2.4f, COLOR_TEXT_DIM, "?");
    }

    // Würfelhistorie
    ui_draw_separator(0, 138, SCREEN_TOP_W, COLOR_TEXT_DIM);
    ui_draw_text(10, 142, 0.5f, 0.37f, COLOR_TEXT_DIM, "Letzte Wuerfe:");
    for (int i = 0; i < HISTORY_MAX; i++) {
        int idx = (s_history_top - i + HISTORY_MAX) % HISTORY_MAX;
        if (s_history_labels[idx][0]) {
            float y = 156.0f + i * 11.0f;
            if (y > 226.0f) break;
            u32 col = (i == 0) ? COLOR_TEXT : COLOR_TEXT_DIM;
            ui_draw_text(10, y, 0.5f, 0.34f, col, s_history_labels[idx]);
        }
    }
}

static void draw_bottom(void) {
    // Würfel-Button-Leiste (y=4-36)
    for (int i = 0; i < 7; i++) {
        // Letzter gewürfelter Typ hervorheben
        static const int sides_arr[] = {4,6,8,10,12,20,100};
        s_btn_dice[i].color_bg = (sides_arr[i] == s_last_sides && s_last_result > 0)
                                  ? COLOR_ACCENT : COLOR_BTN_NORMAL;
        ui_button_draw(&s_btn_dice[i]);
    }

    ui_draw_separator(0, 40, SCREEN_BTM_W, COLOR_TEXT_DIM);

    // Modifier-Bereich (y=44-108)
    ui_draw_panel(4, 44, 312, 62, COLOR_PANEL, COLOR_TEXT_DIM);

    // Modifier
    ui_draw_textf(44, 48, 0.5f, 0.38f, COLOR_TEXT_DIM, "Modifier");
    ui_button_draw(&s_btn_mod_minus);
    ui_draw_textf(42, 82, 0.5f, 0.55f, COLOR_TEXT, "%+d", s_modifier);
    ui_button_draw(&s_btn_mod_plus);

    // Anzahl
    ui_draw_textf(162, 48, 0.5f, 0.38f, COLOR_TEXT_DIM, "Anzahl");
    ui_button_draw(&s_btn_cnt_minus);
    ui_draw_textf(162, 82, 0.5f, 0.55f, COLOR_TEXT, "%d", s_count);
    ui_button_draw(&s_btn_cnt_plus);

    // Würfel-Auswahl anzeigen
    char expr[32];
    if (s_modifier != 0)
        snprintf(expr, sizeof(expr), "%dd? %+d", s_count, s_modifier);
    else
        snprintf(expr, sizeof(expr), "%dd?", s_count);
    ui_draw_textf(248, 48, 0.5f, 0.38f, COLOR_TEXT_DIM, "Formel:");
    ui_draw_textf(248, 62, 0.5f, 0.42f, COLOR_TEXT, "%s", expr);

    ui_draw_separator(0, 110, SCREEN_BTM_W, COLOR_TEXT_DIM);

    // Vorteil / Nachteil
    ui_button_draw(&s_btn_adv);
    ui_button_draw(&s_btn_dis);

    ui_draw_separator(0, 184, SCREEN_BTM_W, COLOR_TEXT_DIM);

    // Hilfe + Zurück
    ui_button_draw(&s_btn_back);
    ui_draw_text(92, 213, 0.5f, 0.34f, COLOR_TEXT_DIM, "[B] Zurueck   [X] Wiederholen");
}

Screen g_screen_dice = {
    .id          = SCREEN_DICE,
    .on_enter    = on_enter,
    .on_exit     = on_exit,
    .update      = update,
    .draw_top    = draw_top,
    .draw_bottom = draw_bottom
};
