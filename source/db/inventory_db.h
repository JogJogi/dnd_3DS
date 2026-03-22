#pragma once
#include "../models/item.h"

#define INVENTORY_MAX 128

// Lädt das komplette Inventar eines Charakters. Gibt Anzahl Items zurück.
int inventory_db_load(int char_id, Item* out, int max_items);

// Speichert ein Item (INSERT wenn id==0, UPDATE sonst). Gibt 0 bei Erfolg.
int inventory_db_save(Item* item);

// Löscht ein Item. Gibt 0 bei Erfolg.
int inventory_db_delete(int item_id);

// Währung laden
int currency_db_load(int char_id, Currency* out);

// Währung speichern
int currency_db_save(const Currency* c);

// Gesamtgewicht des Inventars
float inventory_db_total_weight(int char_id);
