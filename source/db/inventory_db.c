#include "inventory_db.h"
#include "database.h"
#include <stdio.h>
#include <string.h>

#define BIND_INT(stmt, idx, val)   sqlite3_bind_int(stmt, idx, val)
#define BIND_DOUBLE(stmt, idx, val) sqlite3_bind_double(stmt, idx, val)
#define BIND_TEXT(stmt, idx, val)  sqlite3_bind_text(stmt, idx, val, -1, SQLITE_STATIC)
#define COL_TEXT(stmt, idx, dst, size) \
    do { const char* _s = (const char*)sqlite3_column_text(stmt, idx); \
         if (_s) { strncpy(dst, _s, size-1); dst[size-1] = '\0'; } \
         else { dst[0] = '\0'; } } while(0)

int inventory_db_load(int char_id, Item* out, int max_items) {
    sqlite3_stmt* stmt = NULL;
    const char* sql =
        "SELECT id,character_id,name,description,quantity,weight,value_cp,equipped,category "
        "FROM inventory WHERE character_id=? ORDER BY name ASC LIMIT ?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;

    BIND_INT(stmt, 1, char_id);
    BIND_INT(stmt, 2, max_items);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_items) {
        Item* it        = &out[n];
        it->id          = sqlite3_column_int(stmt, 0);
        it->character_id= sqlite3_column_int(stmt, 1);
        COL_TEXT(stmt, 2, it->name,        ITEM_NAME_MAX);
        COL_TEXT(stmt, 3, it->description, ITEM_DESC_MAX);
        it->quantity    = sqlite3_column_int(stmt, 4);
        it->weight      = (float)sqlite3_column_double(stmt, 5);
        it->value_cp    = sqlite3_column_int(stmt, 6);
        it->equipped    = sqlite3_column_int(stmt, 7);
        it->category    = (ItemCategory)sqlite3_column_int(stmt, 8);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

int inventory_db_save(Item* item) {
    sqlite3_stmt* stmt = NULL;
    int rc;

    if (item->id == 0) {
        const char* sql =
            "INSERT INTO inventory (character_id,name,description,quantity,weight,value_cp,equipped,category)"
            "VALUES (?,?,?,?,?,?,?,?);";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        BIND_INT   (stmt, 1, item->character_id);
        BIND_TEXT  (stmt, 2, item->name);
        BIND_TEXT  (stmt, 3, item->description);
        BIND_INT   (stmt, 4, item->quantity);
        BIND_DOUBLE(stmt, 5, item->weight);
        BIND_INT   (stmt, 6, item->value_cp);
        BIND_INT   (stmt, 7, item->equipped);
        BIND_INT   (stmt, 8, (int)item->category);
    } else {
        const char* sql =
            "UPDATE inventory SET name=?,description=?,quantity=?,weight=?,value_cp=?,equipped=?,category=?"
            "WHERE id=?;";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        BIND_TEXT  (stmt, 1, item->name);
        BIND_TEXT  (stmt, 2, item->description);
        BIND_INT   (stmt, 3, item->quantity);
        BIND_DOUBLE(stmt, 4, item->weight);
        BIND_INT   (stmt, 5, item->value_cp);
        BIND_INT   (stmt, 6, item->equipped);
        BIND_INT   (stmt, 7, (int)item->category);
        BIND_INT   (stmt, 8, item->id);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) return -1;
    if (item->id == 0) item->id = (int)sqlite3_last_insert_rowid(g_db);
    return 0;
}

int inventory_db_delete(int item_id) {
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM inventory WHERE id=%d;", item_id);
    return db_exec(sql);
}

int currency_db_load(int char_id, Currency* out) {
    sqlite3_stmt* stmt = NULL;
    const char* sql =
        "SELECT copper,silver,electrum,gold,platinum FROM currency WHERE character_id=?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;

    BIND_INT(stmt, 1, char_id);
    out->character_id = char_id;
    out->copper = out->silver = out->electrum = out->gold = out->platinum = 0;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->copper   = sqlite3_column_int(stmt, 0);
        out->silver   = sqlite3_column_int(stmt, 1);
        out->electrum = sqlite3_column_int(stmt, 2);
        out->gold     = sqlite3_column_int(stmt, 3);
        out->platinum = sqlite3_column_int(stmt, 4);
    }
    sqlite3_finalize(stmt);
    return 0;
}

int currency_db_save(const Currency* c) {
    sqlite3_stmt* stmt = NULL;
    const char* sql =
        "INSERT INTO currency (character_id,copper,silver,electrum,gold,platinum) VALUES (?,?,?,?,?,?)"
        "ON CONFLICT(character_id) DO UPDATE SET copper=excluded.copper,"
        "silver=excluded.silver,electrum=excluded.electrum,"
        "gold=excluded.gold,platinum=excluded.platinum;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;

    BIND_INT(stmt, 1, c->character_id);
    BIND_INT(stmt, 2, c->copper);
    BIND_INT(stmt, 3, c->silver);
    BIND_INT(stmt, 4, c->electrum);
    BIND_INT(stmt, 5, c->gold);
    BIND_INT(stmt, 6, c->platinum);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE ? 0 : -1;
}

float inventory_db_total_weight(int char_id) {
    sqlite3_stmt* stmt = NULL;
    const char* sql =
        "SELECT SUM(weight * quantity) FROM inventory WHERE character_id=?;";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0.0f;

    BIND_INT(stmt, 1, char_id);
    float total = 0.0f;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        total = (float)sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    return total;
}
