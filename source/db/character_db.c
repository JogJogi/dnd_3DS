#include "character_db.h"
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// ---- Hilfsmakros für sqlite3 -----------------------------------------------
#define BIND_TEXT(stmt, idx, val)  sqlite3_bind_text(stmt, idx, val, -1, SQLITE_STATIC)
#define BIND_INT(stmt, idx, val)   sqlite3_bind_int(stmt, idx, val)
#define BIND_DOUBLE(stmt, idx, val) sqlite3_bind_double(stmt, idx, val)
#define COL_TEXT(stmt, idx, dst, size) \
    do { const char* _s = (const char*)sqlite3_column_text(stmt, idx); \
         if (_s) { strncpy(dst, _s, size-1); dst[size-1] = '\0'; } \
         else { dst[0] = '\0'; } } while(0)

// ---- Skill / Save Proficiencies laden/speichern ----------------------------

static void load_skills(int char_id, Character* c) {
    for (int i = 0; i < SKILL_COUNT; i++) c->skill_proficient[i] = 0;

    sqlite3_stmt* stmt = NULL;
    const char* sql = "SELECT skill_index, proficient FROM character_skills WHERE character_id=?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return;

    sqlite3_bind_int(stmt, 1, char_id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int idx  = sqlite3_column_int(stmt, 0);
        int prof = sqlite3_column_int(stmt, 1);
        if (idx >= 0 && idx < SKILL_COUNT)
            c->skill_proficient[idx] = prof;
    }
    sqlite3_finalize(stmt);
}

static void save_skills(const Character* c) {
    char sql[256];
    snprintf(sql, sizeof(sql),
        "DELETE FROM character_skills WHERE character_id=%d;", c->id);
    db_exec(sql);

    sqlite3_stmt* stmt = NULL;
    const char* ins = "INSERT INTO character_skills (character_id, skill_index, proficient) VALUES (?,?,?);";
    if (sqlite3_prepare_v2(g_db, ins, -1, &stmt, NULL) != SQLITE_OK) return;

    for (int i = 0; i < SKILL_COUNT; i++) {
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, c->id);
        sqlite3_bind_int(stmt, 2, i);
        sqlite3_bind_int(stmt, 3, c->skill_proficient[i]);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

static void load_saves(int char_id, Character* c) {
    for (int i = 0; i < ABILITY_COUNT; i++) c->save_proficient[i] = 0;

    sqlite3_stmt* stmt = NULL;
    const char* sql = "SELECT ability_index, proficient FROM save_proficiencies WHERE character_id=?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return;

    sqlite3_bind_int(stmt, 1, char_id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int idx  = sqlite3_column_int(stmt, 0);
        int prof = sqlite3_column_int(stmt, 1);
        if (idx >= 0 && idx < ABILITY_COUNT)
            c->save_proficient[idx] = prof;
    }
    sqlite3_finalize(stmt);
}

static void save_saves(const Character* c) {
    char sql[256];
    snprintf(sql, sizeof(sql),
        "DELETE FROM save_proficiencies WHERE character_id=%d;", c->id);
    db_exec(sql);

    sqlite3_stmt* stmt = NULL;
    const char* ins = "INSERT INTO save_proficiencies (character_id, ability_index, proficient) VALUES (?,?,?);";
    if (sqlite3_prepare_v2(g_db, ins, -1, &stmt, NULL) != SQLITE_OK) return;

    for (int i = 0; i < ABILITY_COUNT; i++) {
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, c->id);
        sqlite3_bind_int(stmt, 2, i);
        sqlite3_bind_int(stmt, 3, c->save_proficient[i]);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

// ---- Öffentliche Funktionen ------------------------------------------------

int character_db_count(void) {
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(g_db, "SELECT COUNT(*) FROM characters;", -1, &stmt, NULL) != SQLITE_OK)
        return -1;
    int count = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
}

int character_db_list(int* out_ids, char out_names[][64], int max_count) {
    sqlite3_stmt* stmt = NULL;
    const char* sql = "SELECT id, name FROM characters ORDER BY id ASC LIMIT ?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;

    sqlite3_bind_int(stmt, 1, max_count);
    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_count) {
        out_ids[n] = sqlite3_column_int(stmt, 0);
        const char* nm = (const char*)sqlite3_column_text(stmt, 1);
        if (nm) { strncpy(out_names[n], nm, 63); out_names[n][63] = '\0'; }
        else    { out_names[n][0] = '\0'; }
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

int character_db_load(int id, Character* c) {
    sqlite3_stmt* stmt = NULL;
    const char* sql =
        "SELECT id,name,class_name,subclass,level,race,background,alignment,experience,"
        "str,dex,con,int_,wis,cha,"
        "hp_max,hp_current,hp_temp,"
        "armor_class,speed,inspiration,"
        "death_saves_successes,death_saves_failures,"
        "personality_traits,ideals,bonds,flaws,backstory "
        "FROM characters WHERE id=?;";

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }

    c->id           = sqlite3_column_int(stmt, 0);
    COL_TEXT(stmt, 1,  c->name,             CHAR_NAME_MAX);
    COL_TEXT(stmt, 2,  c->class_name,        CHAR_NAME_MAX);
    COL_TEXT(stmt, 3,  c->subclass,          CHAR_NAME_MAX);
    c->level        = sqlite3_column_int(stmt, 4);
    COL_TEXT(stmt, 5,  c->race,              CHAR_NAME_MAX);
    COL_TEXT(stmt, 6,  c->background,        CHAR_NAME_MAX);
    COL_TEXT(stmt, 7,  c->alignment,         CHAR_NAME_MAX);
    c->experience   = sqlite3_column_int(stmt, 8);
    for (int i = 0; i < ABILITY_COUNT; i++)
        c->ability[i] = sqlite3_column_int(stmt, 9 + i);
    c->hp_max       = sqlite3_column_int(stmt, 15);
    c->hp_current   = sqlite3_column_int(stmt, 16);
    c->hp_temp      = sqlite3_column_int(stmt, 17);
    c->armor_class  = sqlite3_column_int(stmt, 18);
    c->speed        = sqlite3_column_int(stmt, 19);
    c->inspiration  = sqlite3_column_int(stmt, 20);
    c->death_saves_successes = sqlite3_column_int(stmt, 21);
    c->death_saves_failures  = sqlite3_column_int(stmt, 22);
    COL_TEXT(stmt, 23, c->personality_traits, CHAR_TEXT_MAX);
    COL_TEXT(stmt, 24, c->ideals,             CHAR_TEXT_MAX);
    COL_TEXT(stmt, 25, c->bonds,              CHAR_TEXT_MAX);
    COL_TEXT(stmt, 26, c->flaws,              CHAR_TEXT_MAX);
    COL_TEXT(stmt, 27, c->backstory,          CHAR_LONG_MAX);
    sqlite3_finalize(stmt);

    load_skills(id, c);
    load_saves(id, c);
    return 0;
}

int character_db_save(Character* c) {
    sqlite3_stmt* stmt = NULL;
    int rc;

    if (c->id == 0) {
        // INSERT
        const char* sql =
            "INSERT INTO characters (name,class_name,subclass,level,race,background,alignment,experience,"
            "str,dex,con,int_,wis,cha,hp_max,hp_current,hp_temp,"
            "armor_class,speed,inspiration,death_saves_successes,death_saves_failures,"
            "personality_traits,ideals,bonds,flaws,backstory)"
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    } else {
        // UPDATE
        const char* sql =
            "UPDATE characters SET name=?,class_name=?,subclass=?,level=?,race=?,background=?,alignment=?,experience=?,"
            "str=?,dex=?,con=?,int_=?,wis=?,cha=?,hp_max=?,hp_current=?,hp_temp=?,"
            "armor_class=?,speed=?,inspiration=?,death_saves_successes=?,death_saves_failures=?,"
            "personality_traits=?,ideals=?,bonds=?,flaws=?,backstory=? WHERE id=?;";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    }

    BIND_TEXT(stmt, 1,  c->name);
    BIND_TEXT(stmt, 2,  c->class_name);
    BIND_TEXT(stmt, 3,  c->subclass);
    BIND_INT (stmt, 4,  c->level);
    BIND_TEXT(stmt, 5,  c->race);
    BIND_TEXT(stmt, 6,  c->background);
    BIND_TEXT(stmt, 7,  c->alignment);
    BIND_INT (stmt, 8,  c->experience);
    for (int i = 0; i < ABILITY_COUNT; i++)
        BIND_INT(stmt, 9+i, c->ability[i]);
    BIND_INT (stmt, 15, c->hp_max);
    BIND_INT (stmt, 16, c->hp_current);
    BIND_INT (stmt, 17, c->hp_temp);
    BIND_INT (stmt, 18, c->armor_class);
    BIND_INT (stmt, 19, c->speed);
    BIND_INT (stmt, 20, c->inspiration);
    BIND_INT (stmt, 21, c->death_saves_successes);
    BIND_INT (stmt, 22, c->death_saves_failures);
    BIND_TEXT(stmt, 23, c->personality_traits);
    BIND_TEXT(stmt, 24, c->ideals);
    BIND_TEXT(stmt, 25, c->bonds);
    BIND_TEXT(stmt, 26, c->flaws);
    BIND_TEXT(stmt, 27, c->backstory);
    if (c->id != 0) BIND_INT(stmt, 28, c->id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;

    if (c->id == 0) {
        c->id = (int)sqlite3_last_insert_rowid(g_db);
        // Default-Währung anlegen
        char sql[128];
        snprintf(sql, sizeof(sql),
            "INSERT OR IGNORE INTO currency (character_id) VALUES (%d);", c->id);
        db_exec(sql);
    }

    save_skills(c);
    save_saves(c);
    return 0;
}

int character_db_delete(int id) {
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM characters WHERE id=%d;", id);
    return db_exec(sql);
}

int character_db_save_hp(int id, int hp_current, int hp_temp) {
    sqlite3_stmt* stmt = NULL;
    const char* sql = "UPDATE characters SET hp_current=?, hp_temp=? WHERE id=?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    BIND_INT(stmt, 1, hp_current);
    BIND_INT(stmt, 2, hp_temp);
    BIND_INT(stmt, 3, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

int character_db_save_status(int id, int inspiration, int successes, int failures) {
    sqlite3_stmt* stmt = NULL;
    const char* sql =
        "UPDATE characters SET inspiration=?, death_saves_successes=?, death_saves_failures=? WHERE id=?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    BIND_INT(stmt, 1, inspiration);
    BIND_INT(stmt, 2, successes);
    BIND_INT(stmt, 3, failures);
    BIND_INT(stmt, 4, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}
