#pragma once
#include <3ds.h>

// ---- Screen IDs ------------------------------------------------------------
typedef enum {
    SCREEN_MAIN_MENU = 0,
    SCREEN_CHARACTER,
    SCREEN_INVENTORY,
    SCREEN_SPELLS,
    SCREEN_NOTES,
    SCREEN_DICE,
    SCREEN_BUILDER,   // Gefuehrter Charaktererstellungs-Wizard
    SCREEN_LEVELUP,   // Stufenaufstiegs-Wizard
    SCREEN_COUNT
} ScreenID;

// ---- Screen-Interface ------------------------------------------------------
// Jedes Screen-Modul implementiert diese Callbacks

typedef struct {
    ScreenID id;
    void (*on_enter)(void);                    // Wird aufgerufen wenn Screen aktiv wird
    void (*on_exit)(void);                     // Wird aufgerufen wenn Screen verlassen wird
    void (*update)(u32 keys_down, u32 keys_held, touchPosition* touch, int touch_down);
    void (*draw_top)(void);                    // Zeichnet auf den oberen Bildschirm
    void (*draw_bottom)(void);                 // Zeichnet auf den unteren Bildschirm
} Screen;

// ---- Screen-Manager --------------------------------------------------------
void screen_manager_init(void);

// Navigiert zu einem Screen (Push auf Stack, max. 8 Ebenen)
void screen_push(ScreenID id);

// Zurück zum vorherigen Screen
void screen_pop(void);

// Aktuellen Screen ersetzen (kein Stack-Eintrag)
void screen_replace(ScreenID id);

// Aktiver Screen verarbeitet Input und zeichnet
void screen_update(u32 keys_down, u32 keys_held, touchPosition* touch, int touch_down);
void screen_draw_top(void);
void screen_draw_bottom(void);

// Aktuell aktive Screen-ID abfragen
ScreenID screen_current_id(void);

// Globale Charakter-ID (welcher Char ist gerade geladen)
extern int g_active_char_id;
