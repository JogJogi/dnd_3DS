#include "database.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef __3DS__
#include <3ds.h>
#endif

sqlite3* g_db = NULL;
char g_db_last_error[256] = {0};

// ---- Schema ---------------------------------------------------------------

static const char* SCHEMA_SQL =
    "PRAGMA journal_mode=MEMORY;"
    "PRAGMA synchronous=OFF;"
    "PRAGMA foreign_keys=ON;"

    "CREATE TABLE IF NOT EXISTS characters ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  name TEXT NOT NULL DEFAULT 'Unbenannt',"
    "  class_name TEXT DEFAULT '',"
    "  subclass TEXT DEFAULT '',"
    "  level INTEGER DEFAULT 1,"
    "  race TEXT DEFAULT '',"
    "  background TEXT DEFAULT '',"
    "  alignment TEXT DEFAULT '',"
    "  experience INTEGER DEFAULT 0,"
    // Ability scores
    "  str INTEGER DEFAULT 10,"
    "  dex INTEGER DEFAULT 10,"
    "  con INTEGER DEFAULT 10,"
    "  int_ INTEGER DEFAULT 10,"
    "  wis INTEGER DEFAULT 10,"
    "  cha INTEGER DEFAULT 10,"
    // HP
    "  hp_max INTEGER DEFAULT 10,"
    "  hp_current INTEGER DEFAULT 10,"
    "  hp_temp INTEGER DEFAULT 0,"
    // Combat
    "  armor_class INTEGER DEFAULT 10,"
    "  speed INTEGER DEFAULT 30,"
    // Status
    "  inspiration INTEGER DEFAULT 0,"
    "  death_saves_successes INTEGER DEFAULT 0,"
    "  death_saves_failures INTEGER DEFAULT 0,"
    // RP
    "  personality_traits TEXT DEFAULT '',"
    "  ideals TEXT DEFAULT '',"
    "  bonds TEXT DEFAULT '',"
    "  flaws TEXT DEFAULT '',"
    "  backstory TEXT DEFAULT ''"
    ");"

    "CREATE TABLE IF NOT EXISTS character_skills ("
    "  character_id INTEGER NOT NULL,"
    "  skill_index INTEGER NOT NULL,"
    "  proficient INTEGER DEFAULT 0,"   // 0=keine, 1=proficient, 2=expertise
    "  PRIMARY KEY (character_id, skill_index),"
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS save_proficiencies ("
    "  character_id INTEGER NOT NULL,"
    "  ability_index INTEGER NOT NULL,"
    "  proficient INTEGER DEFAULT 0,"
    "  PRIMARY KEY (character_id, ability_index),"
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS inventory ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  character_id INTEGER NOT NULL,"
    "  name TEXT NOT NULL DEFAULT 'Item',"
    "  description TEXT DEFAULT '',"
    "  quantity INTEGER DEFAULT 1,"
    "  weight REAL DEFAULT 0.0,"
    "  value_cp INTEGER DEFAULT 0,"
    "  equipped INTEGER DEFAULT 0,"
    "  category INTEGER DEFAULT 5,"   // ITEM_CAT_OTHER
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS currency ("
    "  character_id INTEGER PRIMARY KEY,"
    "  copper INTEGER DEFAULT 0,"
    "  silver INTEGER DEFAULT 0,"
    "  electrum INTEGER DEFAULT 0,"
    "  gold INTEGER DEFAULT 0,"
    "  platinum INTEGER DEFAULT 0,"
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS spells ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  character_id INTEGER NOT NULL,"
    "  name TEXT NOT NULL DEFAULT 'Zauber',"
    "  spell_level INTEGER DEFAULT 0,"
    "  school TEXT DEFAULT '',"
    "  casting_time TEXT DEFAULT '1 action',"
    "  range TEXT DEFAULT '',"
    "  components TEXT DEFAULT '',"
    "  duration TEXT DEFAULT '',"
    "  description TEXT DEFAULT '',"
    "  prepared INTEGER DEFAULT 0,"
    "  concentration INTEGER DEFAULT 0,"
    "  ritual INTEGER DEFAULT 0,"
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS spell_slots ("
    "  character_id INTEGER NOT NULL,"
    "  slot_level INTEGER NOT NULL,"  // 1-9
    "  total INTEGER DEFAULT 0,"
    "  used INTEGER DEFAULT 0,"
    "  PRIMARY KEY (character_id, slot_level),"
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS notes ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  character_id INTEGER NOT NULL,"
    "  title TEXT NOT NULL DEFAULT 'Notiz',"
    "  content TEXT DEFAULT '',"
    "  category INTEGER DEFAULT 0,"
    "  created_at INTEGER DEFAULT 0,"
    "  updated_at INTEGER DEFAULT 0,"
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS conditions ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  character_id INTEGER NOT NULL,"
    "  condition_name TEXT NOT NULL,"
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS features ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  character_id INTEGER NOT NULL,"
    "  name TEXT NOT NULL DEFAULT 'Fähigkeit',"
    "  source TEXT DEFAULT '',"
    "  description TEXT DEFAULT '',"
    "  uses_max INTEGER DEFAULT 0,"
    "  uses_current INTEGER DEFAULT 0,"
    "  recharge_on TEXT DEFAULT NULL,"
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS attacks ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  character_id INTEGER NOT NULL,"
    "  name TEXT NOT NULL DEFAULT 'Angriff',"
    "  hit_bonus TEXT DEFAULT '+0',"
    "  damage TEXT DEFAULT '1d6',"
    "  damage_type TEXT DEFAULT '',"
    "  notes TEXT DEFAULT '',"
    "  action_type INTEGER DEFAULT 0,"
    "  FOREIGN KEY (character_id) REFERENCES characters(id) ON DELETE CASCADE"
    ");"

    "CREATE TABLE IF NOT EXISTS dice_history ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  character_id INTEGER,"
    "  roll_expression TEXT DEFAULT '',"
    "  result INTEGER DEFAULT 0,"
    "  rolled_at INTEGER DEFAULT 0"
    ");";

// ---- 3DS-VFS -----------------------------------------------------------------
// Zwei Probleme mit SQLite's unix VFS auf dem 3DS:
// 1. xFullPathname: unix VFS prüft ob Pfad mit '/' beginnt. 'sdmc:/...' beginnt
//    mit 's' → SQLite ruft getcwd() auf und baut falschen Pfad.
// 2. xLock/xUnlock: unix VFS nutzt fcntl() für POSIX Advisory Locks, die auf
//    FAT32 (sdmc) nicht unterstützt werden → SQLITE_IOERR_LOCK.
// Lösung: Minimale VFS-Wrapper-Schicht, die beide Probleme behebt.

static sqlite3_vfs             s_vfs_3ds;
static sqlite3_io_methods      s_io_3ds;
static int                     s_io_patched = 0;
static int (*s_base_xOpen)(sqlite3_vfs*, const char*, sqlite3_file*, int, int*);

// --- Locking-Stubs (FAT32 unterstützt kein fcntl-Locking) ---
static int noop_lock(sqlite3_file* f, int e)        { (void)f;(void)e; return SQLITE_OK; }
static int noop_unlock(sqlite3_file* f, int e)      { (void)f;(void)e; return SQLITE_OK; }
static int noop_checklock(sqlite3_file* f, int* p)  { (void)f; *p=0; return SQLITE_OK; }

// --- xOpen: ruft unix-xOpen auf, patcht dann io_methods ---
static int vfs_xOpen(sqlite3_vfs* vfs, const char* zName, sqlite3_file* pFile,
                     int flags, int* pOutFlags) {
    int rc = s_base_xOpen(vfs, zName, pFile, flags, pOutFlags);
    if (rc == SQLITE_OK && pFile->pMethods) {
        if (!s_io_patched) {
            s_io_3ds                    = *pFile->pMethods;
            s_io_3ds.xLock              = noop_lock;
            s_io_3ds.xUnlock            = noop_unlock;
            s_io_3ds.xCheckReservedLock = noop_checklock;
            s_io_patched = 1;
        }
        pFile->pMethods = &s_io_3ds;
    }
    return rc;
}

// --- xFullPathname: 'sdmc:/...' als absolut behandeln ---
static int vfs_fullpathname(sqlite3_vfs* vfs, const char* zIn, int nOut, char* zOut) {
    (void)vfs;
    if (strstr(zIn, ":/") != NULL) {
        sqlite3_snprintf(nOut, zOut, "%s", zIn);
        return SQLITE_OK;
    }
    sqlite3_snprintf(nOut, zOut, "sdmc:/3ds/dnd_companion/%s", zIn);
    return SQLITE_OK;
}

static void register_3ds_vfs(void) {
    sqlite3_vfs* base = sqlite3_vfs_find("unix");
    if (!base) return;
    s_base_xOpen          = base->xOpen;
    s_vfs_3ds             = *base;
    s_vfs_3ds.zName       = "3ds";
    s_vfs_3ds.xOpen       = vfs_xOpen;
    s_vfs_3ds.xFullPathname = vfs_fullpathname;
    sqlite3_vfs_register(&s_vfs_3ds, 1);
}

// ---- Öffentliche Funktionen -----------------------------------------------

int db_exec(const char* sql) {
    char* err = NULL;
    int rc = sqlite3_exec(g_db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "DB Error: %s\n", err ? err : "unbekannt");
        sqlite3_free(err);
    }
    return rc == SQLITE_OK ? 0 : -1;
}

int db_init(void) {
    // VFS registrieren (muss VOR sqlite3_open passieren)
    register_3ds_vfs();

    // Verzeichnisse anlegen; -1 mit EEXIST ist OK (existiert schon)
    mkdir("sdmc:/3ds", 0777);
    int r2 = mkdir("sdmc:/3ds/dnd_companion", 0777);
    int e2 = errno;

    int rc = sqlite3_open(DB_PATH, &g_db);
    if (rc != SQLITE_OK) {
        const char* msg = g_db ? sqlite3_errmsg(g_db) : "NULL";
        snprintf(g_db_last_error, sizeof(g_db_last_error),
                 "SQLite %d: %s\nDir: r=%d e=%d",
                 rc, msg, r2, e2);
        return -1;
    }

    // Exclusive locking: kein fcntl-Locking nötig (single-user app)
    sqlite3_exec(g_db, "PRAGMA locking_mode=EXCLUSIVE;", NULL, NULL, NULL);

    // Schema anlegen (idempotent - IF NOT EXISTS)
    if (db_exec(SCHEMA_SQL) != 0) {
        snprintf(g_db_last_error, sizeof(g_db_last_error),
                 "Schema: %s (ext=%d)",
                 sqlite3_errmsg(g_db), sqlite3_extended_errcode(g_db));
        return -1;
    }

    // Migrationen: fehlende Spalten hinzufügen (Fehler ignorieren = Spalte existiert schon)
    sqlite3_exec(g_db,
        "ALTER TABLE attacks ADD COLUMN action_type INTEGER DEFAULT 0;",
        NULL, NULL, NULL);

    return 0;
}

void db_close(void) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}
