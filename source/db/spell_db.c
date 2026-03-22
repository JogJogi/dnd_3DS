#include "spell_db.h"
#include "database.h"
#include <string.h>
#include <stdio.h>

// Hilfsmakro: Text aus sqlite3_column_text sicher kopieren
#define COL_TEXT(stmt, col, dst, sz) do { \
    const char* _t = (const char*)sqlite3_column_text(stmt, col); \
    if (_t) snprintf(dst, sz, "%s", _t); else dst[0] = '\0'; \
} while(0)

int spells_db_load(int char_id, Spell* out, int max) {
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT id,character_id,name,spell_level,school,"
        "casting_time,range,components,duration,description,"
        "prepared,concentration,ritual "
        "FROM spells WHERE character_id=? ORDER BY spell_level,name";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, char_id);
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max) {
        Spell* s = &out[count++];
        memset(s, 0, sizeof(*s));
        s->id           = sqlite3_column_int(stmt, 0);
        s->character_id = sqlite3_column_int(stmt, 1);
        COL_TEXT(stmt, 2, s->name,         SPELL_NAME_MAX);
        s->spell_level  = sqlite3_column_int(stmt, 3);
        COL_TEXT(stmt, 4, s->school,       SPELL_TEXT_MAX);
        COL_TEXT(stmt, 5, s->casting_time, SPELL_TEXT_MAX);
        COL_TEXT(stmt, 6, s->range,        SPELL_TEXT_MAX);
        COL_TEXT(stmt, 7, s->components,   SPELL_TEXT_MAX);
        COL_TEXT(stmt, 8, s->duration,     SPELL_TEXT_MAX);
        COL_TEXT(stmt, 9, s->description,  SPELL_DESC_MAX);
        s->prepared      = sqlite3_column_int(stmt, 10);
        s->concentration = sqlite3_column_int(stmt, 11);
        s->ritual        = sqlite3_column_int(stmt, 12);
    }
    sqlite3_finalize(stmt);
    return count;
}

int spells_db_save(Spell* s) {
    sqlite3_stmt* stmt;
    if (s->id == 0) {
        const char* sql =
            "INSERT INTO spells "
            "(character_id,name,spell_level,school,casting_time,range,"
            "components,duration,description,prepared,concentration,ritual) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?)";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        sqlite3_bind_int (stmt,  1, s->character_id);
        sqlite3_bind_text(stmt,  2, s->name,         -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt,  3, s->spell_level);
        sqlite3_bind_text(stmt,  4, s->school,        -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  5, s->casting_time,  -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  6, s->range,         -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  7, s->components,    -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  8, s->duration,      -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  9, s->description,   -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt, 10, s->prepared);
        sqlite3_bind_int (stmt, 11, s->concentration);
        sqlite3_bind_int (stmt, 12, s->ritual);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) s->id = (int)sqlite3_last_insert_rowid(g_db);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE ? 0 : -1;
    } else {
        const char* sql =
            "UPDATE spells SET name=?,spell_level=?,school=?,casting_time=?,"
            "range=?,components=?,duration=?,description=?,"
            "prepared=?,concentration=?,ritual=? WHERE id=?";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        sqlite3_bind_text(stmt,  1, s->name,         -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt,  2, s->spell_level);
        sqlite3_bind_text(stmt,  3, s->school,        -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  4, s->casting_time,  -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  5, s->range,         -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  6, s->components,    -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  7, s->duration,      -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,  8, s->description,   -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt,  9, s->prepared);
        sqlite3_bind_int (stmt, 10, s->concentration);
        sqlite3_bind_int (stmt, 11, s->ritual);
        sqlite3_bind_int (stmt, 12, s->id);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE ? 0 : -1;
    }
}

void spells_db_delete(int spell_id) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, "DELETE FROM spells WHERE id=?", -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, spell_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void spells_db_set_prepared(int spell_id, int prepared) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, "UPDATE spells SET prepared=? WHERE id=?", -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, prepared);
    sqlite3_bind_int(stmt, 2, spell_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

int spell_slots_db_load(int char_id, SpellSlots* out) {
    memset(out, 0, sizeof(*out));
    out->character_id = char_id;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db,
            "SELECT slot_level,total,used FROM spell_slots WHERE character_id=?",
            -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, char_id);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int lv = sqlite3_column_int(stmt, 0);
        if (lv >= 1 && lv <= SPELL_SLOT_LEVELS) {
            out->total[lv-1] = sqlite3_column_int(stmt, 1);
            out->used[lv-1]  = sqlite3_column_int(stmt, 2);
        }
    }
    sqlite3_finalize(stmt);
    return 0;
}

void spell_slots_db_save(const SpellSlots* s) {
    const char* sql =
        "INSERT INTO spell_slots (character_id,slot_level,total,used) VALUES (?,?,?,?)"
        " ON CONFLICT(character_id,slot_level) DO UPDATE SET total=excluded.total,used=excluded.used";
    sqlite3_stmt* stmt;
    for (int i = 0; i < SPELL_SLOT_LEVELS; i++) {
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) continue;
        sqlite3_bind_int(stmt, 1, s->character_id);
        sqlite3_bind_int(stmt, 2, i + 1);
        sqlite3_bind_int(stmt, 3, s->total[i]);
        sqlite3_bind_int(stmt, 4, s->used[i]);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}
