#pragma once
#include <citro2d.h>
#include <3ds.h>

// ---- Farben (RGBA8) - Warmes D&D Fantasy Thema -----------------------------
#define COLOR_BG         C2D_Color32(0x0E, 0x0B, 0x08, 0xFF)  // Warmes Tiefschwarz
#define COLOR_PANEL      C2D_Color32(0x1C, 0x15, 0x0E, 0xFF)  // Dunkles Braun
#define COLOR_PANEL2     C2D_Color32(0x28, 0x20, 0x16, 0xFF)  // Helleres Panel / Highlight
#define COLOR_ACCENT     C2D_Color32(0x8B, 0x1A, 0x10, 0xFF)  // D&D Blutrot
#define COLOR_ACCENT2    C2D_Color32(0xBC, 0x3C, 0x20, 0xFF)  // Helleres Rot
#define COLOR_TEXT       C2D_Color32(0xEC, 0xE0, 0xCC, 0xFF)  // Warmes Pergament
#define COLOR_TEXT_DIM   C2D_Color32(0x80, 0x72, 0x5C, 0xFF)  // Gedimmter Text
#define COLOR_BTN_NORMAL C2D_Color32(0x24, 0x1C, 0x12, 0xFF)  // Dunkler Button
#define COLOR_BTN_HOVER  C2D_Color32(0x3A, 0x2E, 0x20, 0xFF)  // Aktiver Button
#define COLOR_HP_GREEN   C2D_Color32(0x2E, 0x9E, 0x4A, 0xFF)  // Waldgrün
#define COLOR_HP_YELLOW  C2D_Color32(0xC8, 0x8C, 0x14, 0xFF)  // Amber
#define COLOR_HP_RED     C2D_Color32(0xAA, 0x28, 0x14, 0xFF)  // Dunkelrot
#define COLOR_GOLD       C2D_Color32(0xD4, 0xAA, 0x28, 0xFF)  // D&D Gold
#define COLOR_WHITE      C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)
#define COLOR_BLACK      C2D_Color32(0x00, 0x00, 0x00, 0xFF)
#define COLOR_TRANSPARENT C2D_Color32(0x00, 0x00, 0x00, 0x00)

// ---- Bildschirmgrößen ------------------------------------------------------
#define SCREEN_TOP_W     400
#define SCREEN_TOP_H     240
#define SCREEN_BTM_W     320
#define SCREEN_BTM_H     240

// ---- Render-Targets (global, werden in main.c initialisiert) ---------------
extern C3D_RenderTarget* g_target_top;
extern C3D_RenderTarget* g_target_bottom;

// ---- Text-System -----------------------------------------------------------
extern C2D_Font     g_font;
extern C2D_TextBuf  g_text_buf;      // Statischer Buffer für UI-Text

void ui_text_init(void);
void ui_text_fini(void);
void ui_text_buf_clear(void);

// Text zeichnen (x,y in Bildschirmkoordinaten, size = Schriftgröße 0.4..1.0)
void ui_draw_text(float x, float y, float z, float size, u32 color, const char* text);
void ui_draw_textf(float x, float y, float z, float size, u32 color, const char* fmt, ...);

// Texte mit bekannter Breite (für Rechtsbündigkeit)
float ui_text_width(float size, const char* text);

// ---- Geometrie -------------------------------------------------------------
void ui_draw_rect(float x, float y, float w, float h, u32 color);
void ui_draw_rect_outline(float x, float y, float w, float h, float thickness, u32 color);

// HP-Balken: zeigt farbigen Fortschrittsbalken
void ui_draw_hp_bar(float x, float y, float w, float h, int current, int max);

// Panel: gefülltes Rechteck mit Rahmen
void ui_draw_panel(float x, float y, float w, float h, u32 bg, u32 border);

// Trennlinie (horizontal)
void ui_draw_separator(float x, float y, float w, u32 color);

// ---- Button ----------------------------------------------------------------
typedef struct {
    float x, y, w, h;
    const char* label;
    u32   color_bg;
    u32   color_text;
    int   active;   // 1 = wird gerade gedrückt (visuelles Feedback)
} UiButton;

void ui_button_draw(const UiButton* btn);

// Gibt 1 zurück wenn Button im Frame gedrückt wurde (touch-based)
int  ui_button_touched(const UiButton* btn, touchPosition* touch);

// ---- Scrollbare Liste -------------------------------------------------------
#define UI_LIST_MAX_ITEMS 64

typedef struct {
    float x, y, w, h;        // Zeichenbereich
    float item_height;        // Höhe eines Eintrags
    int   selected;           // Aktuell ausgewählter Index (-1 = keiner)
    int   scroll_offset;      // Erste sichtbare Zeile
    int   count;              // Anzahl Einträge
    char  items[UI_LIST_MAX_ITEMS][128];
    u32   item_colors[UI_LIST_MAX_ITEMS];  // Optionale Farbe pro Eintrag
} UiList;

void ui_list_init(UiList* list, float x, float y, float w, float h, float item_h);
void ui_list_set_item(UiList* list, int idx, const char* text, u32 color);
void ui_list_draw(const UiList* list);
void ui_list_handle_touch(UiList* list, touchPosition* touch, int touch_down);
void ui_list_scroll(UiList* list, int delta);   // delta: +1 runter, -1 hoch

// ---- Input Helpers ---------------------------------------------------------
// Gibt 1 zurück wenn Touchscreen-Punkt im Rechteck liegt
int ui_point_in_rect(float px, float py, float rx, float ry, float rw, float rh);

// Software-Tastatur öffnen und Text einholen
// max_len: maximale Zeichenzahl (inkl. \0)
// Gibt 1 bei OK, 0 bei Abbrechen zurück
int ui_keyboard_input(const char* hint, char* out_buf, int max_len);
int ui_keyboard_input_numeric(const char* hint, int* out_value, int min_val, int max_val);
