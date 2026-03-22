#include "feature_db.h"
#include "database.h"
#include <string.h>
#include <stdio.h>

#define COL_TEXT(stmt, col, dst, sz) do { \
    const char* _t = (const char*)sqlite3_column_text(stmt, col); \
    if (_t) snprintf(dst, sz, "%s", _t); else dst[0] = '\0'; \
} while(0)

// ============================================================================
// Features
// ============================================================================

int features_db_load(int char_id, Feature* out, int max) {
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT id,character_id,name,source,description,"
        "uses_max,uses_current,recharge_on "
        "FROM features WHERE character_id=? ORDER BY name";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, char_id);
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max) {
        Feature* f = &out[count++];
        memset(f, 0, sizeof(*f));
        f->id           = sqlite3_column_int(stmt, 0);
        f->character_id = sqlite3_column_int(stmt, 1);
        COL_TEXT(stmt, 2, f->name,         FEATURE_NAME_MAX);
        COL_TEXT(stmt, 3, f->source,       FEATURE_SRC_MAX);
        COL_TEXT(stmt, 4, f->description,  FEATURE_DESC_MAX);
        f->uses_max      = sqlite3_column_int(stmt, 5);
        f->uses_current  = sqlite3_column_int(stmt, 6);
        COL_TEXT(stmt, 7, f->recharge_on,  FEATURE_RECH_MAX);
    }
    sqlite3_finalize(stmt);
    return count;
}

int features_db_save(Feature* f) {
    sqlite3_stmt* stmt;
    if (f->id == 0) {
        const char* sql =
            "INSERT INTO features "
            "(character_id,name,source,description,uses_max,uses_current,recharge_on) "
            "VALUES (?,?,?,?,?,?,?)";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        sqlite3_bind_int (stmt, 1, f->character_id);
        sqlite3_bind_text(stmt, 2, f->name,        -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, f->source,       -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, f->description,  -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt, 5, f->uses_max);
        sqlite3_bind_int (stmt, 6, f->uses_current);
        sqlite3_bind_text(stmt, 7, f->recharge_on,  -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) f->id = (int)sqlite3_last_insert_rowid(g_db);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE ? 0 : -1;
    } else {
        const char* sql =
            "UPDATE features SET name=?,source=?,description=?,"
            "uses_max=?,uses_current=?,recharge_on=? WHERE id=?";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        sqlite3_bind_text(stmt, 1, f->name,        -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, f->source,       -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, f->description,  -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt, 4, f->uses_max);
        sqlite3_bind_int (stmt, 5, f->uses_current);
        sqlite3_bind_text(stmt, 6, f->recharge_on,  -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt, 7, f->id);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE ? 0 : -1;
    }
}

void features_db_delete(int feature_id) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, "DELETE FROM features WHERE id=?",
                           -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, feature_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void features_db_use(int feature_id, int uses_current) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, "UPDATE features SET uses_current=? WHERE id=?",
                           -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, uses_current);
    sqlite3_bind_int(stmt, 2, feature_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// ============================================================================
// Attacks
// ============================================================================

int attacks_db_load(int char_id, Attack* out, int max) {
    sqlite3_stmt* stmt;
    const char* sql =
        "SELECT id,character_id,name,hit_bonus,damage,damage_type,notes,action_type "
        "FROM attacks WHERE character_id=? ORDER BY action_type,name";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, char_id);
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < max) {
        Attack* a = &out[count++];
        memset(a, 0, sizeof(*a));
        a->id           = sqlite3_column_int(stmt, 0);
        a->character_id = sqlite3_column_int(stmt, 1);
        COL_TEXT(stmt, 2, a->name,        ATTACK_NAME_MAX);
        COL_TEXT(stmt, 3, a->hit_bonus,   ATTACK_BONUS_MAX);
        COL_TEXT(stmt, 4, a->damage,      ATTACK_DMG_MAX);
        COL_TEXT(stmt, 5, a->damage_type, ATTACK_DMG_MAX);
        COL_TEXT(stmt, 6, a->notes,       ATTACK_NOTE_MAX);
        a->action_type  = sqlite3_column_int(stmt, 7);
    }
    sqlite3_finalize(stmt);
    return count;
}

int attacks_db_save(Attack* a) {
    sqlite3_stmt* stmt;
    if (a->id == 0) {
        const char* sql =
            "INSERT INTO attacks "
            "(character_id,name,hit_bonus,damage,damage_type,notes,action_type) "
            "VALUES (?,?,?,?,?,?,?)";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        sqlite3_bind_int (stmt, 1, a->character_id);
        sqlite3_bind_text(stmt, 2, a->name,        -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, a->hit_bonus,    -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, a->damage,       -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, a->damage_type,  -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, a->notes,        -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt, 7, a->action_type);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) a->id = (int)sqlite3_last_insert_rowid(g_db);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE ? 0 : -1;
    } else {
        const char* sql =
            "UPDATE attacks SET name=?,hit_bonus=?,damage=?,damage_type=?,notes=?,action_type=? "
            "WHERE id=?";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        sqlite3_bind_text(stmt, 1, a->name,        -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, a->hit_bonus,    -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, a->damage,       -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, a->damage_type,  -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, a->notes,        -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt, 6, a->action_type);
        sqlite3_bind_int (stmt, 7, a->id);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return rc == SQLITE_DONE ? 0 : -1;
    }
}

void attacks_db_delete(int attack_id) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, "DELETE FROM attacks WHERE id=?",
                           -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, attack_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
