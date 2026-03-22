#pragma once
#include "../models/note.h"

#define NOTES_MAX 64

// Lädt alle Notizen eines Charakters (optional nach Kategorie filtern, -1 = alle)
int notes_db_load(int char_id, NoteCategory filter, Note* out, int max_notes);

// Speichert eine Notiz (INSERT wenn id==0, UPDATE sonst). Gibt 0 bei Erfolg.
int notes_db_save(Note* note);

// Löscht eine Notiz. Gibt 0 bei Erfolg.
int notes_db_delete(int note_id);
