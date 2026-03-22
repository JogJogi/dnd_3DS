#pragma once
#include "../models/character.h"

// Gibt Anzahl Charaktere zurück, -1 bei Fehler
int character_db_count(void);

// Lädt alle Charakter-IDs und -Namen in Arrays (max. max_count Einträge)
// Gibt tatsächliche Anzahl zurück
int character_db_list(int* out_ids, char out_names[][64], int max_count);

// Lädt einen Charakter vollständig aus der DB. Gibt 0 bei Erfolg zurück.
int character_db_load(int id, Character* out);

// Speichert einen Charakter (INSERT wenn id==0, UPDATE sonst)
// Setzt c->id bei INSERT. Gibt 0 bei Erfolg zurück.
int character_db_save(Character* c);

// Löscht Charakter und alle verknüpften Daten. Gibt 0 bei Erfolg zurück.
int character_db_delete(int id);

// Speichert nur HP (häufige Operation - ohne vollen Save)
int character_db_save_hp(int id, int hp_current, int hp_temp);

// Speichert nur Inspiration & Death Saves
int character_db_save_status(int id, int inspiration, int successes, int failures);
