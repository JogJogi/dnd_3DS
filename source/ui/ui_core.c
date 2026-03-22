#include "ui_core.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

C3D_RenderTarget* g_target_top    = NULL;
C3D_RenderTarget* g_target_bottom = NULL;

C2D_Font    g_font     = NULL;
C2D_TextBuf g_text_buf = NULL;

#define TEXT_BUF_SIZE 8192

// ---- Text-System -----------------------------------------------------------

void ui_text_init(void) {
    g_font     = C2D_FontLoadSystem(CFG_REGION_EUR);
    g_text_buf = C2D_TextBufNew(TEXT_BUF_SIZE);
}

void ui_text_fini(void) {
    if (g_text_buf) { C2D_TextBufDelete(g_text_buf); g_text_buf = NULL; }
    if (g_font)     { C2D_FontFree(g_font);           g_font     = NULL; }
}

void ui_text_buf_clear(void) {
    C2D_TextBufClear(g_text_buf);
}

void ui_draw_text(float x, float y, float z, float size, u32 color, const char* text) {
    C2D_Text t;
    C2D_TextFontParse(&t, g_font, g_text_buf, text);
    C2D_TextOptimize(&t);
    C2D_DrawText(&t, C2D_WithColor, x, y, z, size, size, color);
}

void ui_draw_textf(float x, float y, float z, float size, u32 color, const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ui_draw_text(x, y, z, size, color, buf);
}

float ui_text_width(float size, const char* text) {
    C2D_Text t;
    C2D_TextFontParse(&t, g_font, g_text_buf, text);
    C2D_TextOptimize(&t);
    float width = 0.0f, height = 0.0f;
    C2D_TextGetDimensions(&t, size, size, &width, &height);
    return width;
}

// ---- Geometrie -------------------------------------------------------------

void ui_draw_rect(float x, float y, float w, float h, u32 color) {
    C2D_DrawRectSolid(x, y, 0.5f, w, h, color);
}

void ui_draw_rect_outline(float x, float y, float w, float h, float thickness, u32 color) {
    // Oben
    C2D_DrawRectSolid(x,               y,               0.5f, w,         thickness, color);
    // Unten
    C2D_DrawRectSolid(x,               y + h - thickness, 0.5f, w,       thickness, color);
    // Links
    C2D_DrawRectSolid(x,               y,               0.5f, thickness, h,         color);
    // Rechts
    C2D_DrawRectSolid(x + w - thickness, y,             0.5f, thickness, h,         color);
}

void ui_draw_hp_bar(float x, float y, float w, float h, int current, int max) {
    if (max <= 0) return;

    float ratio = (float)current / (float)max;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    // Hintergrund
    ui_draw_rect(x, y, w, h, COLOR_PANEL);

    // Balken-Farbe je nach Füllstand
    u32 bar_color;
    if (ratio > 0.5f)       bar_color = COLOR_HP_GREEN;
    else if (ratio > 0.25f) bar_color = COLOR_HP_YELLOW;
    else                    bar_color = COLOR_HP_RED;

    if (current > 0)
        ui_draw_rect(x, y, w * ratio, h, bar_color);

    // Rahmen
    ui_draw_rect_outline(x, y, w, h, 1.0f, COLOR_TEXT_DIM);
}

// ---- Panel + Separator -----------------------------------------------------

void ui_draw_panel(float x, float y, float w, float h, u32 bg, u32 border) {
    ui_draw_rect(x, y, w, h, bg);
    ui_draw_rect_outline(x, y, w, h, 1.0f, border);
}

void ui_draw_separator(float x, float y, float w, u32 color) {
    C2D_DrawRectSolid(x, y, 0.5f, w, 1.0f, color);
}

// ---- Button ----------------------------------------------------------------

void ui_button_draw(const UiButton* btn) {
    u32 bg = btn->active ? COLOR_BTN_HOVER : btn->color_bg;
    ui_draw_rect(btn->x, btn->y, btn->w, btn->h, bg);
    // Leichter Glanz-Effekt oben (1px hellerer Streifen)
    u32 top_line = btn->active
        ? C2D_Color32(0x30, 0x24, 0x18, 0xFF)
        : C2D_Color32(0x48, 0x3A, 0x28, 0xFF);
    C2D_DrawRectSolid(btn->x, btn->y, 0.51f, btn->w, 1.0f, top_line);
    // Rahmen
    ui_draw_rect_outline(btn->x, btn->y, btn->w, btn->h, 1.0f, COLOR_TEXT_DIM);

    if (btn->label) {
        float tw = ui_text_width(0.45f, btn->label);
        float tx = btn->x + (btn->w - tw) * 0.5f;
        float ty = btn->y + (btn->h - 12.0f) * 0.5f;
        ui_draw_text(tx, ty, 0.6f, 0.45f, btn->color_text, btn->label);
    }
}

int ui_button_touched(const UiButton* btn, touchPosition* touch) {
    // 3px Padding vergrössert die Hit-Zone ohne die Optik zu ändern.
    // Hilft bei kleinen Buttons (h<20px) und normaler Finger-/Stylus-Ungenauigkeit.
    const float PAD = 3.0f;
    return ui_point_in_rect((float)touch->px, (float)touch->py,
                             btn->x - PAD,          btn->y - PAD,
                             btn->w + 2.0f * PAD,   btn->h + 2.0f * PAD);
}

// ---- Scrollbare Liste -------------------------------------------------------

void ui_list_init(UiList* list, float x, float y, float w, float h, float item_h) {
    list->x           = x;
    list->y           = y;
    list->w           = w;
    list->h           = h;
    list->item_height = item_h;
    list->selected    = -1;
    list->scroll_offset = 0;
    list->count       = 0;
    for (int i = 0; i < UI_LIST_MAX_ITEMS; i++) {
        list->items[i][0] = '\0';
        list->item_colors[i] = COLOR_TEXT;
    }
}

void ui_list_set_item(UiList* list, int idx, const char* text, u32 color) {
    if (idx < 0 || idx >= UI_LIST_MAX_ITEMS) return;
    strncpy(list->items[idx], text, 127);
    list->items[idx][127] = '\0';
    list->item_colors[idx] = color;
    if (idx >= list->count) list->count = idx + 1;
}

void ui_list_draw(const UiList* list) {
    // Hintergrund
    ui_draw_rect(list->x, list->y, list->w, list->h, COLOR_PANEL);
    ui_draw_rect_outline(list->x, list->y, list->w, list->h, 1.0f, COLOR_TEXT_DIM);

    int visible = (int)(list->h / list->item_height);
    float iy    = list->y + 2.0f;

    for (int i = list->scroll_offset; i < list->count && i < list->scroll_offset + visible; i++) {
        float row_y = iy + (i - list->scroll_offset) * list->item_height;

        // Hintergrund des ausgewählten Eintrags
        if (i == list->selected) {
            ui_draw_rect(list->x + 1.0f, row_y - 1.0f,
                         list->w - 2.0f, list->item_height - 1.0f,
                         COLOR_ACCENT);
        }

        ui_draw_text(list->x + 6.0f, row_y + 2.0f, 0.6f, 0.45f,
                     list->item_colors[i], list->items[i]);

        // Trennlinie
        if (i < list->count - 1) {
            ui_draw_rect(list->x + 4.0f, row_y + list->item_height - 1.0f,
                         list->w - 8.0f, 1.0f, COLOR_TEXT_DIM);
        }
    }

    // Scrollbar (wenn nötig)
    if (list->count > visible) {
        float sb_x  = list->x + list->w - 4.0f;
        float sb_h  = list->h;
        float thumb_h = sb_h * ((float)visible / list->count);
        float thumb_y = list->y + sb_h * ((float)list->scroll_offset / list->count);
        ui_draw_rect(sb_x, list->y, 3.0f, sb_h, COLOR_BTN_NORMAL);
        ui_draw_rect(sb_x, thumb_y, 3.0f, thumb_h, COLOR_ACCENT2);
    }
}

void ui_list_handle_touch(UiList* list, touchPosition* touch, int touch_down) {
    if (!touch_down) return;
    if (!ui_point_in_rect((float)touch->px, (float)touch->py,
                           list->x, list->y, list->w, list->h)) return;

    int idx = list->scroll_offset +
              (int)((touch->py - list->y) / list->item_height);
    if (idx >= 0 && idx < list->count)
        list->selected = idx;
}

void ui_list_scroll(UiList* list, int delta) {
    int visible = (int)(list->h / list->item_height);
    list->scroll_offset += delta;
    if (list->scroll_offset < 0)
        list->scroll_offset = 0;
    if (list->scroll_offset > list->count - visible)
        list->scroll_offset = list->count - visible;
    if (list->scroll_offset < 0)
        list->scroll_offset = 0;
}

// ---- Input Helpers ---------------------------------------------------------

int ui_point_in_rect(float px, float py, float rx, float ry, float rw, float rh) {
    return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

int ui_keyboard_input(const char* hint, char* out_buf, int max_len) {
    SwkbdState swkbd;
    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, max_len - 1);
    swkbdSetHintText(&swkbd, hint);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT,  "Abbruch", false);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "OK",      true);
    swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 0);
    if (out_buf[0]) swkbdSetInitialText(&swkbd, out_buf);

    SwkbdButton btn = swkbdInputText(&swkbd, out_buf, max_len);
    return btn == SWKBD_BUTTON_CONFIRM ? 1 : 0;
}

int ui_keyboard_input_numeric(const char* hint, int* out_value, int min_val, int max_val) {
    SwkbdState swkbd;
    char buf[16] = {0};
    snprintf(buf, sizeof(buf), "%d", *out_value);

    swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 2, 6);
    swkbdSetHintText(&swkbd, hint);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT,  "Abbruch", false);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "OK",      true);
    swkbdSetInitialText(&swkbd, buf);

    SwkbdButton btn = swkbdInputText(&swkbd, buf, sizeof(buf));
    if (btn == SWKBD_BUTTON_CONFIRM) {
        int val = atoi(buf);
        if (val < min_val) val = min_val;
        if (val > max_val) val = max_val;
        *out_value = val;
        return 1;
    }
    return 0;
}
