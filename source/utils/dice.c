#include "dice.h"
#include <stdlib.h>
#include <time.h>

#ifdef __3DS__
#include <3ds.h>
#endif

const int DICE_TYPES[]   = {4, 6, 8, 10, 12, 20, 100};
const int DICE_TYPE_COUNT = 7;

void dice_init(void) {
#ifdef __3DS__
    // Auf dem 3DS: osGetTime() liefert Millisekunden seit Systemstart
    srand((unsigned int)osGetTime());
#else
    srand((unsigned int)time(NULL));
#endif
}

int dice_roll(int sides) {
    if (sides <= 0) return 0;
    return (rand() % sides) + 1;
}

int dice_roll_n(int count, int sides) {
    int total = 0;
    for (int i = 0; i < count; i++)
        total += dice_roll(sides);
    return total;
}

int dice_roll_mod(int count, int sides, int modifier) {
    return dice_roll_n(count, sides) + modifier;
}

int dice_roll_advantage(void) {
    int a = dice_roll(20);
    int b = dice_roll(20);
    return a > b ? a : b;
}

int dice_roll_disadvantage(void) {
    int a = dice_roll(20);
    int b = dice_roll(20);
    return a < b ? a : b;
}
