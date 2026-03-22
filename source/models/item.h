#pragma once
#include <stdint.h>

#define ITEM_NAME_MAX 128
#define ITEM_DESC_MAX 512

typedef enum {
    ITEM_CAT_WEAPON = 0,
    ITEM_CAT_ARMOR,
    ITEM_CAT_TOOL,
    ITEM_CAT_CONSUMABLE,
    ITEM_CAT_TREASURE,
    ITEM_CAT_OTHER,
    ITEM_CAT_COUNT
} ItemCategory;

extern const char* ITEM_CAT_NAMES[ITEM_CAT_COUNT];

typedef struct {
    int  id;
    int  character_id;
    char name[ITEM_NAME_MAX];
    char description[ITEM_DESC_MAX];
    int  quantity;
    float weight;        // in lbs
    int  value_cp;       // Wert in Kupfermünzen
    int  equipped;       // 0 oder 1
    ItemCategory category;
} Item;

// Hilfsfunktionen
int    item_value_to_gp(int cp, int* out_gp, int* out_sp, int* out_cp);
float  item_total_weight(const Item* item);

typedef struct {
    int character_id;
    int copper;
    int silver;
    int electrum;
    int gold;
    int platinum;
} Currency;

// Gesamtwert in Kupfer
int currency_total_cp(const Currency* c);
