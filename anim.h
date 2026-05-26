#ifndef _ANIM_H_
#define _ANIM_H_

#include "tigr/tigr.h"

typedef enum {
    PLAYER = 0,
    NPC    = 1,
    OBJECT = 2,
} anim_type;

typedef enum {
    IDLE     = 0,
    RUN      = 1,
    JUMP     = 2,
    ATTACK   = 3,
    INTERACT = 4,
    DEATH    = 5,
    NONE     = 6,
} anim_action;

typedef struct {
    Tigr** frames;
    int framecount;
    int currframe;
    char *name;
    anim_type type;
    anim_action action;
    float timer;
    float duration;
    int repeat;
} anim_t;

// load sprites from file
void anim_load_sprites_file(anim_t *anim, 
                       char *filenamepattern,
                       char *name,
                       int count,
                       anim_type type,
                       anim_action action,
                       float duration,
                       int repeat);

// load sprites but pointing to frames already in memory
// useful for multiple instance of sprite - npcs, interactibles, weapons 
void anim_load_sprites_mem(anim_t *anim, 
                       Tigr **frames,
                       char *name,
                       int count,
                       anim_type type,
                       anim_action action,
                       float duration,
                       int repeat);

void anim_free(anim_t *anim);
void anim_advance_frame(anim_t *anim, float dt);

#endif
