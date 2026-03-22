#include "notes_db.h"
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BIND_INT(stmt, idx, val)  sqlite3_bind_int(stmt, idx, val)
#define BIND_TEXT(stmt, idx, val) sqlite3_bind_text(stmt, idx, val, -1, SQLITE_STATIC)
#define COL_TEXT(stmt, idx, dst, size) \
    do { const char* _s = (const char*)sqlite3_column_text(stmt, idx); \
         if (_s) { strncpy(dst, _s, size-1); dst[size-1] = '\0'; } \
         else { dst[0] = '\0'; } } while(0)

int notes_db_load(int char_id, NoteCategory filter, Note* out, int max_notes) {
    sqlite3_stmt* stmt = NULL;
    char sql[512];

    if (filter < 0 || filter >= NOTE_CAT_COUNT) {
        snprintf(sql, sizeof(sql),
            "SELECT id,character_id,title,content,category,created_at,updated_at "
            "FROM notes WHERE character_id=? ORDER BY updated_at DESC LIMIT ?;");
    } else {
        snprintf(sql, sizeof(sql),
            "SELECT id,character_id,title,content,category,created_at,updated_at "
            "FROM notes WHERE character_id=? AND category=%d ORDER BY updated_at DESC LIMIT ?;",
            (int)filter);
    }

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    BIND_INT(stmt, 1, char_id);
    BIND_INT(stmt, 2, max_notes);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_notes) {
        Note* note          = &out[n];
        note->id            = sqlite3_column_int(stmt, 0);
        note->character_id  = sqlite3_column_int(stmt, 1);
        COL_TEXT(stmt, 2, note->title,   NOTE_TITLE_MAX);
        COL_TEXT(stmt, 3, note->content, NOTE_CONTENT_MAX);
        note->category      = (NoteCategory)sqlite3_column_int(stmt, 4);
        note->created_at    = sqlite3_column_int(stmt, 5);
        note->updated_at    = sqlite3_column_int(stmt, 6);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

int notes_db_save(Note* note) {
    sqlite3_stmt* stmt = NULL;
    int now = (int)time(NULL);
    int rc;

    if (note->id == 0) {
        note->created_at = now;
        note->updated_at = now;
        const char* sql =
            "INSERT INTO notes (character_id,title,content,category,created_at,updated_at)"
            "VALUES (?,?,?,?,?,?);";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        BIND_INT (stmt, 1, note->character_id);
        BIND_TEXT(stmt, 2, note->title);
        BIND_TEXT(stmt, 3, note->content);
        BIND_INT (stmt, 4, (int)note->category);
        BIND_INT (stmt, 5, note->created_at);
        BIND_INT (stmt, 6, note->updated_at);
    } else {
        note->updated_at = now;
        const char* sql =
            "UPDATE notes SET title=?,content=?,category=?,updated_at=? WHERE id=?;";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
        BIND_TEXT(stmt, 1, note->title);
        BIND_TEXT(stmt, 2, note->content);
        BIND_INT (stmt, 3, (int)note->category);
        BIND_INT (stmt, 4, note->updated_at);
        BIND_INT (stmt, 5, note->id);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) return -1;
    if (note->id == 0) note->id = (int)sqlite3_last_insert_rowid(g_db);
    return 0;
}

int notes_db_delete(int note_id) {
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM notes WHERE id=%d;", note_id);
    return db_exec(sql);
}
