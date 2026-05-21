#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anim.h"
#include "tigr/tigr.h"

#ifndef _min
#define _min(a, b)   ((a) < (b) ? (a) : (b))
#endif
#ifndef _max
#define _max(a, b)   ((a) > (b) ? (a) : (b))
#endif

void anim_load_sprites_file(anim_t *anim, 
                       char *filenamepattern,
                       char *name,
                       int count,
                       anim_type type,
                       anim_action action,
                       float duration,
                       int repeat) {
    anim->framecount = count;
    anim->currframe  = 0;
    anim->name       = name;
    anim->type       = type;
    anim->action     = action;
    anim->timer      = 0.0f;
    anim->duration   = duration;
    anim->repeat     = repeat;
    
    anim->frames = malloc(sizeof(Tigr *) * anim->framecount);

    char buf[100] = { 0 };
    for(int i = 0; i < anim->framecount; i++) {
        snprintf(buf, 99, filenamepattern, i+1);
        anim->frames[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }
}

void anim_load_sprites_mem(anim_t *anim, 
                       Tigr** frames,
                       char *name,
                       int count,
                       anim_type type,
                       anim_action action,
                       float duration,
                       int repeat) {
    anim->framecount = count;
    anim->currframe  = 0;
    anim->name       = name;
    anim->type       = type;
    anim->action     = action;
    anim->timer      = 0.0f;
    anim->duration   = duration;
    anim->repeat     = repeat;
    anim->frames     = frames;
}

void anim_free(anim_t *anim) {
    if(anim->frames) {
        free(anim->frames);
        anim->frames = NULL;
    }
}

void anim_advance_frame(anim_t *anim, float dt) {
    anim->timer += dt;

    if(anim->timer >= anim->duration) {
        if(!anim->repeat) {
            anim->currframe = _min(anim->currframe + 1, (anim->framecount - 1));
        } else {
            anim->currframe = (anim->currframe + 1) % anim->framecount;
        }
        anim->timer -= anim->duration;
    }
}