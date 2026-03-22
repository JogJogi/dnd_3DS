#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <stdio.h>

#include "db/database.h"
#include "ui/ui_core.h"
#include "ui/screen_manager.h"
#include "utils/dice.h"

// Framerate-Cap: 60 FPS
#define FRAME_SYNC C3D_FRAME_SYNCDRAW

int main(void) {
    // ---- 3DS Services initialisieren ---------------------------------------
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // Render-Targets für beide Bildschirme
    g_target_top    = C2D_CreateScreenTarget(GFX_TOP,    GFX_LEFT);
    g_target_bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // Debug-Konsole auf Top-Screen (nur aktivieren wenn DB-Fehler - für Entwicklung)
    // consoleInit(GFX_TOP, NULL);

    // ---- Subsysteme initialisieren -----------------------------------------
    dice_init();
    ui_text_init();

    if (db_init() != 0) {
        // Fallback: Debug-Konsole zeigen
        consoleInit(GFX_TOP, NULL);
        printf("FEHLER: DB-Init fehlgeschlagen!\n\n");
        printf("%s\n\n", g_db_last_error);
        printf("Druecke START zum Beenden.\n");
        while (aptMainLoop()) {
            hidScanInput();
            if (hidKeysDown() & KEY_START) break;
            gspWaitForVBlank();
        }
        goto cleanup;
    }

    // ---- Screen-Manager starten -------------------------------------------
    screen_manager_init();

    // ---- Haupt-Gameloop ----------------------------------------------------
    while (aptMainLoop()) {
        hidScanInput();

        u32 keys_down = hidKeysDown();
        u32 keys_held = hidKeysHeld();

        // Touch-Input lesen
        // touch_down = 1 NUR im ersten Frame des Kontakts (steigende Flanke).
        // Verhindert Multi-Clicks und Screen-Transitions die sofort den neuen
        // Screen triggern, weil der Finger noch drauf liegt.
        touchPosition touch = {0};
        int touch_down = 0;
        if (keys_held & KEY_TOUCH) {
            hidTouchRead(&touch);          // Position immer aktuell halten
        }
        if (keys_down & KEY_TOUCH) {      // Nur Frame 1 des Kontakts
            touch_down = 1;
        }

        // START + SELECT = App beenden
        if ((keys_held & KEY_START) && (keys_held & KEY_SELECT)) break;

        // Screen-Update (Logik)
        screen_update(keys_down, keys_held, &touch, touch_down);

        // ---- Rendering -----------------------------------------------------
        C3D_FrameBegin(FRAME_SYNC);

        // Oberer Bildschirm
        C2D_TargetClear(g_target_top, COLOR_BG);
        C2D_SceneBegin(g_target_top);
        ui_text_buf_clear();
        screen_draw_top();

        // Unterer Bildschirm
        C2D_TargetClear(g_target_bottom, COLOR_BG);
        C2D_SceneBegin(g_target_bottom);
        ui_text_buf_clear();
        screen_draw_bottom();

        C3D_FrameEnd(0);
    }

cleanup:
    // ---- Aufräumen ---------------------------------------------------------
    db_close();
    ui_text_fini();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
