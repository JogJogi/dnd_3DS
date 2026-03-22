#include "screen_manager.h"
#include "ui_core.h"

// Forward-Deklarationen aller Screen-Module
extern Screen g_screen_main_menu;
extern Screen g_screen_character;
extern Screen g_screen_inventory;
extern Screen g_screen_spells;
extern Screen g_screen_notes;
extern Screen g_screen_dice;
extern Screen g_screen_builder;
extern Screen g_screen_levelup;

int g_active_char_id = 0;

// ---- Screen-Registry -------------------------------------------------------
static Screen* s_screens[SCREEN_COUNT];

// ---- Stack -----------------------------------------------------------------
#define STACK_MAX 8
static ScreenID s_stack[STACK_MAX];
static int      s_stack_top = -1;

static Screen* current_screen(void) {
    if (s_stack_top < 0) return NULL;
    ScreenID id = s_stack[s_stack_top];
    if (id < 0 || id >= SCREEN_COUNT) return NULL;
    return s_screens[id];
}

// ---- Öffentliche Funktionen ------------------------------------------------

void screen_manager_init(void) {
    s_screens[SCREEN_MAIN_MENU] = &g_screen_main_menu;
    s_screens[SCREEN_CHARACTER] = &g_screen_character;
    s_screens[SCREEN_INVENTORY] = &g_screen_inventory;
    s_screens[SCREEN_SPELLS]    = &g_screen_spells;
    s_screens[SCREEN_NOTES]     = &g_screen_notes;
    s_screens[SCREEN_DICE]      = &g_screen_dice;
    s_screens[SCREEN_BUILDER]   = &g_screen_builder;
    s_screens[SCREEN_LEVELUP]   = &g_screen_levelup;

    // Startscreen
    screen_push(SCREEN_MAIN_MENU);
}

void screen_push(ScreenID id) {
    if (s_stack_top >= STACK_MAX - 1) return;

    Screen* scr = s_screens[id];
    if (!scr) return;

    s_stack[++s_stack_top] = id;
    if (scr->on_enter) scr->on_enter();
}

void screen_pop(void) {
    if (s_stack_top < 0) return;

    Screen* scr = current_screen();
    if (scr && scr->on_exit) scr->on_exit();

    s_stack_top--;

    scr = current_screen();
    if (scr && scr->on_enter) scr->on_enter();
}

void screen_replace(ScreenID id) {
    if (s_stack_top >= 0) {
        Screen* old = current_screen();
        if (old && old->on_exit) old->on_exit();
        s_stack_top--;
    }
    screen_push(id);
}

void screen_update(u32 keys_down, u32 keys_held, touchPosition* touch, int touch_down) {
    Screen* scr = current_screen();
    if (scr && scr->update)
        scr->update(keys_down, keys_held, touch, touch_down);
}

void screen_draw_top(void) {
    Screen* scr = current_screen();
    if (scr && scr->draw_top)
        scr->draw_top();
}

void screen_draw_bottom(void) {
    Screen* scr = current_screen();
    if (scr && scr->draw_bottom)
        scr->draw_bottom();
}

ScreenID screen_current_id(void) {
    if (s_stack_top < 0) return SCREEN_MAIN_MENU;
    return s_stack[s_stack_top];
}
