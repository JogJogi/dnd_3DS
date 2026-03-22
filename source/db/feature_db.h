#pragma once
#include "../models/feature.h"

// ---- Features / Traits / Feats ---------------------------------------------
int  features_db_load(int char_id, Feature* out, int max);
int  features_db_save(Feature* f);           // INSERT wenn id==0, sonst UPDATE
void features_db_delete(int feature_id);
void features_db_use(int feature_id, int uses_current);  // Verwendungen aktualisieren

// ---- Angriffe --------------------------------------------------------------
int  attacks_db_load(int char_id, Attack* out, int max);
int  attacks_db_save(Attack* a);             // INSERT wenn id==0, sonst UPDATE
void attacks_db_delete(int attack_id);
