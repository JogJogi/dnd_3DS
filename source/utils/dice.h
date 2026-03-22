#pragma once
#include <stdint.h>

// Würfelsysteme initialisieren (Zufallszahl-Seed setzen)
void dice_init(void);

// Einen Würfel würfeln: d4, d6, d8, d10, d12, d20, d100
int dice_roll(int sides);

// Mehrere Würfel: z.B. dice_roll_n(3, 6) = 3d6
int dice_roll_n(int count, int sides);

// Mit Modifier: z.B. dice_roll_mod(1, 20, 5) = 1d20+5
int dice_roll_mod(int count, int sides, int modifier);

// Advantage: Höheres von zwei d20 Würfen
int dice_roll_advantage(void);

// Disadvantage: Niedrigeres von zwei d20 Würfen
int dice_roll_disadvantage(void);

// Vordefinierte Würfeltypen
typedef enum {
    DICE_D4  = 4,
    DICE_D6  = 6,
    DICE_D8  = 8,
    DICE_D10 = 10,
    DICE_D12 = 12,
    DICE_D20 = 20,
    DICE_D100= 100
} DiceType;

extern const int DICE_TYPES[];
extern const int DICE_TYPE_COUNT;
