#pragma once
#include <stdint.h>

// ---- Feature / Trait / Feat ------------------------------------------------
#define FEATURE_NAME_MAX  64
#define FEATURE_SRC_MAX   32
#define FEATURE_DESC_MAX 256
#define FEATURE_RECH_MAX  16

typedef struct {
    int  id;
    int  character_id;
    char name[FEATURE_NAME_MAX];
    char source[FEATURE_SRC_MAX];      // z.B. "Barde 1", "Aarakocra", "Feat"
    char description[FEATURE_DESC_MAX];
    int  uses_max;       // 0 = unbegrenzt / passiv
    int  uses_current;
    char recharge_on[FEATURE_RECH_MAX]; // "Langer Rast", "Kurzer Rast", ""
} Feature;

#define FEATURES_MAX 32

// ---- Angriff ---------------------------------------------------------------
#define ATTACK_NAME_MAX  64
#define ATTACK_BONUS_MAX 16
#define ATTACK_DMG_MAX   32
#define ATTACK_NOTE_MAX 128

// action_type Werte für Attack
#define ATTACK_ACTION  0   // Aktion
#define ATTACK_BONUS   1   // Bonusaktion
#define ATTACK_REACTION 2  // Reaktion
#define ATTACK_OTHER   3   // Sonstiges

typedef struct {
    int  id;
    int  character_id;
    char name[ATTACK_NAME_MAX];        // "Dolch", "Talons"
    char hit_bonus[ATTACK_BONUS_MAX];  // "+4", "+2"
    char damage[ATTACK_DMG_MAX];       // "1d4+2", "1d6"
    char damage_type[ATTACK_DMG_MAX];  // "Stich", "Hieb", "Wucht"
    char notes[ATTACK_NOTE_MAX];       // "Wurfwaffe, Finesse", "Vielseitig: 1d8"
    int  action_type;                  // ATTACK_ACTION/BONUS/REACTION/OTHER
} Attack;

#define ATTACKS_MAX 16
