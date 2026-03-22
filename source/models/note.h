#pragma once
#include <stdint.h>
#include <time.h>

#define NOTE_TITLE_MAX    128
#define NOTE_CONTENT_MAX 2048
#define NOTE_CAT_MAX      32

typedef enum {
    NOTE_CAT_GENERAL = 0,
    NOTE_CAT_QUEST,
    NOTE_CAT_NPC,
    NOTE_CAT_LORE,
    NOTE_CAT_OTHER,
    NOTE_CAT_COUNT
} NoteCategory;

extern const char* NOTE_CAT_NAMES[NOTE_CAT_COUNT];

typedef struct {
    int  id;
    int  character_id;
    char title[NOTE_TITLE_MAX];
    char content[NOTE_CONTENT_MAX];
    NoteCategory category;
    int  created_at;   // Unix timestamp
    int  updated_at;   // Unix timestamp
} Note;
