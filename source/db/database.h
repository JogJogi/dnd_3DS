#pragma once
#include "../sqlite3.h"

// Pfad auf der 3DS SD-Karte
#define DB_PATH "sdmc:/3ds/dnd_companion/dnd.db"

// Globale DB-Instanz (Singleton)
extern sqlite3* g_db;

// Letzter Fehlertext (befüllt wenn db_init != 0 zurückgibt)
extern char g_db_last_error[256];

// Initialisierung & Shutdown
int  db_init(void);    // Öffnet DB, erstellt Schema falls nötig. Gibt 0 bei Erfolg zurück.
void db_close(void);

// Interner Helfer: SQL ausführen ohne Ergebnis
int  db_exec(const char* sql);
