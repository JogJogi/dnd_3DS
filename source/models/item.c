#include "item.h"

const char* ITEM_CAT_NAMES[ITEM_CAT_COUNT] = {
    "Waffe", "Ruestung", "Werkzeug", "Verbrauchsgut", "Schatz", "Sonstiges"
};

float item_total_weight(const Item* item) {
    return item->weight * item->quantity;
}

int item_value_to_gp(int cp, int* out_gp, int* out_sp, int* out_cp) {
    *out_gp = cp / 100;
    cp     %= 100;
    *out_sp = cp / 10;
    *out_cp = cp % 10;
    return *out_gp;
}

int currency_total_cp(const Currency* c) {
    return c->copper
         + c->silver   * 10
         + c->electrum * 50
         + c->gold     * 100
         + c->platinum * 1000;
}
