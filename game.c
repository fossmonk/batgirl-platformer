#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anim.h"
#include "colors.h"
#include "config.h"
#include "font.h"
#include "game.h"
#include "tigr/tigr.h"

#ifndef _min
#define _min(a, b)   ((a) < (b) ? (a) : (b))
#endif
#ifndef _max
#define _max(a, b)   ((a) > (b) ? (a) : (b))
#endif

#define DIR_LEFT    (1)
#define DIR_RIGHT   (2)

#define dbg_print printf("DEBUG HIT in %s:%d@%s\n", __FILE__, __LINE__, __FUNCTION__)

// Declare the different animations for player
anim_t p_idle;
anim_t p_runr;
anim_t p_runl;
anim_t p_jumpr;
anim_t p_jumpl;

// Declare animation for batarangs
// Load from file for only one
anim_t batarang[MAX_BATRS];

// Declare the player's current animation pointer
anim_t *p_curr_anim;


g_obj* game_init(Tigr* screen, Tigr* canvas) {
    // initialize game object
    g_obj* g = malloc(sizeof(*g));
    g->screen = screen;
    g->canvas = canvas;

    // load fonts
    font_load_ascii("res/fonts/profont29.png", &g->titlefont);
    font_load_ascii("res/fonts/profont17.png", &g->regfont);
    
    // load sprites
    // player
    anim_load_sprites_file(&p_idle, "res/sprites/idle/frame_%03d.png", "IDLE", 7, PLAYER, IDLE, 0.07f, 1);
    anim_load_sprites_file(&p_runr, "res/sprites/runright/frame_%03d.png", "RUN RIGHT", 7, PLAYER, RUN, 0.07f, 1);
    anim_load_sprites_file(&p_runl, "res/sprites/runleft/frame_%03d.png", "RUN LEFT", 7, PLAYER, RUN, 0.07f, 1);
    anim_load_sprites_file(&p_jumpr, "res/sprites/jumpright/frame_%03d.png", "JUMP RIGHT", 7, PLAYER, JUMP, 0.07f, 0);
    anim_load_sprites_file(&p_jumpl, "res/sprites/jumpleft/frame_%03d.png", "JUMP LEFT", 7, PLAYER, JUMP, 0.07f, 0);

    anim_load_sprites_file(&batarang[0], "res/sprites/batarang/frame_%03d.png", "BATARANG", 8, WEAPON, NONE, 0.06f, 1);

    for(int i = 1; i < MAX_BATRS; i++) {
        anim_load_sprites_mem(&batarang[i], batarang[0].frames, "BATARANG", 8, WEAPON, NONE, 0.1f, 1);
    }

    // init current player animation and starting sprite
    p_curr_anim = &p_idle;
    g->p.curr_sprite = p_curr_anim->frames[0];
    g->p.xpos = 0;
    g->p.ypos = g->screen->h - g->p.curr_sprite->h;
    g->p.xvel = 0;
    g->p.yvel = 0;

    // mark all batarangs as inactive and map the animations
    for(int i = 0; i < MAX_BATRS; i++) {
        g->batrs[i].active = 0;
        g->batrs[i].anim   = &batarang[i];
    }
    
    // init_backdrop
    tigrClear(g->canvas, BAT_BLUE1);
    Tigr *bg_image = tigrLoadImage("res/background.png");
    if(bg_image) {
        tigrBlit(g->canvas, bg_image, 0,0,0,0, bg_image->w, bg_image->h);
    }
    
    int tw = tigrTextWidth(g->titlefont, G_TITLE);
    tigrPrint(g->canvas, g->titlefont, (G_W-tw)/2, 0, BAT_BLUE3, G_TITLE);

    return g;
}

void game_update(g_obj *g, float dt) {
    if(dt > 0.1f)dt = 0.1f;

    // update velocity with movement
    if (tigrKeyHeld(g->screen, TK_RIGHT) || tigrKeyHeld(g->screen, 'D')) {
        g->p.h_dir = DIR_RIGHT;
        if(!g->p.jumping) p_curr_anim = &p_runr;
        anim_advance_frame(p_curr_anim, dt);
        g->p.curr_sprite = p_curr_anim->frames[p_curr_anim->currframe];
        g->p.xvel += 40;
    }
    if (tigrKeyHeld(g->screen, TK_LEFT) || tigrKeyHeld(g->screen, 'A')) {
        g->p.h_dir = DIR_LEFT;
        if(!g->p.jumping) p_curr_anim = &p_runl;
        anim_advance_frame(p_curr_anim, dt);
        g->p.curr_sprite = p_curr_anim->frames[p_curr_anim->currframe];
        g->p.xvel -= 40;
    }
    if (tigrKeyDown(g->screen, TK_SPACE)) {
        if(!g->p.jumping) {
            g->p.jumping = 1;
            g->p.yvel -= 700;
            if(g->p.h_dir == DIR_LEFT) {
                p_curr_anim = &p_jumpl;
            } else {
                p_curr_anim = &p_jumpr;
            }
            p_curr_anim->currframe = 0;
        }
    }
    if (tigrKeyDown(g->screen, 'B')) {
        // check for an empty slot in batarangs array
        // assign to one, init position as player's hdir corner
        weapon_t *w = NULL;
        for(int i = 0; i < MAX_BATRS; i++) {
            if(!g->batrs[i].active) {
                w = &g->batrs[i];
                break;
            }
        }
        if(w) {
            w->active = 1;
            w->curr_sprite = w->anim->frames[0];
            // xpos, ypos is little above middle of player
            float delta = 70.0;
            w->xpos = g->p.xpos + g->p.curr_sprite->w/2;
            w->ypos = g->p.ypos + g->p.curr_sprite->h/2 - delta;
            if(g->p.h_dir == DIR_LEFT) {
                // xvel -ve, yvel -ve
                w->xvel = -5;
                w->yvel = -5;
            } else {
                // xvel +ve, yvel -ve
                w->xvel = 5;
                w->yvel = -5;
            }
        }
    }

    /////////////////////////////
    //    PLAYER UPDATES       //
    ////////////////////////////
    // decay velocities after updation
    g->p.xvel *= expf(-20*dt);
    g->p.yvel *= expf(-2*dt);

    // update vertical velocity with gravity
    g->p.yvel += 400*dt;

    // update x and y position
    g->p.xpos += g->p.xvel * dt;
    g->p.ypos += g->p.yvel * dt;

    // limit x position
    g->p.xpos = _max(g->p.xpos, 0);
    g->p.xpos = _min(g->p.xpos, g->screen->w - g->p.curr_sprite->w);
    // limit y position
    g->p.ypos = _max(g->p.ypos, 0);
    g->p.ypos = _min(g->p.ypos, g->screen->h - g->p.curr_sprite->h);

    if(g->p.jumping && (p_curr_anim->action == JUMP)) {
        anim_advance_frame(p_curr_anim, dt);
        g->p.curr_sprite = p_curr_anim->frames[p_curr_anim->currframe];
    }

    if((int)g->p.ypos == (g->screen->h - g->p.curr_sprite->h)) {
        g->p.jumping = 0;
    }

    // if sprite x velocity is 0, 
    // if sprite y pos is ground, switch to idle
    if(((int)g->p.xvel == 0) && !g->p.jumping) {
        p_curr_anim = &p_idle;
        anim_advance_frame(p_curr_anim, dt);
        g->p.curr_sprite = p_curr_anim->frames[p_curr_anim->currframe];
    }

      ////////////////////////////////
     //    BATARANGS UPDATE       //
    ///////////////////////////////
    // cycle through all batarangs, update the position and advance the frame
    // if curr position is beyond canvas limits, mark it as inactive
    for(int i = 0; i < MAX_BATRS; i++) {
        weapon_t *w = &g->batrs[i];
        if(w->active) {
            // check curr positions
            if((w->xpos < 0) || (w->ypos < 0) || (w->xpos > g->canvas->w) || (w->ypos > g->canvas->h)) {
                // mark as inactive
                w->active = 0;
            } else {
                // update positions with velocity
                w->xpos += w->xvel;
                w->ypos += w->yvel;

                // advance frame
                anim_advance_frame(w->anim, dt);
                w->curr_sprite = w->anim->frames[w->anim->currframe];
            }
        }
    }
}

void game_draw(Tigr* screen, Tigr* canvas, g_obj *g) {
    // draw canvas to screen
    tigrBlit(screen, canvas, 0, 0, 0, 0, canvas->w, canvas->h);

    // draw player to screen
    tigrBlitAlpha(screen, 
                      g->p.curr_sprite, 
                      (int)g->p.xpos, 
                      (int)g->p.ypos, 
                      0, 
                      0, 
                      g->p.curr_sprite->w,
                      g->p.curr_sprite->h, 
                      1.0f);
    // draw all active batarangs
    for(int i = 0; i < MAX_BATRS; i++) {
        weapon_t *w = &g->batrs[i];
        if(w->active) {
            tigrBlitAlpha(screen, 
                      w->curr_sprite, 
                      (int)w->xpos, 
                      (int)w->ypos, 
                      0, 
                      0, 
                      w->curr_sprite->w,
                      w->curr_sprite->h,
                      1.0f);
        }
    }
}

void game_debug_dump(g_obj *g) {
    char buf[50] = { 0 };
    int tw = 0;
    int th = 0;
    const int gap = 2;
    
    // position and velocity
    snprintf(buf, 49, "X: %0.1f, Y: %0.1f, VX: %0.1f, VY: %0.1f\n", 
        g->p.xpos, g->p.ypos, g->p.xvel, g->p.yvel);
    tw = tigrTextWidth(g->regfont, buf);
    tigrPrint(g->screen, g->regfont, g->screen->w - tw, th + gap, BAT_WHITE, buf);
    th += tigrTextHeight(g->regfont, buf);
    memset(buf, 0, sizeof(char));

    // player current animation
    snprintf(buf, 49, "Current Player Animation: %s\n", p_curr_anim->name);
    tw = tigrTextWidth(g->regfont, buf);
    tigrPrint(g->screen, g->regfont, g->screen->w - tw, th + gap, BAT_WHITE, buf);
    th += tigrTextHeight(g->regfont, buf);
    memset(buf, 0, sizeof(char));

    // number of active batarangs
    int count = 0;
    for(int i = 0; i < MAX_BATRS; i++) {
        if(g->batrs[i].active)count++;
    }
    snprintf(buf, 49, "Active Batarangs: %d\n", count);
    tw = tigrTextWidth(g->regfont, buf);
    tigrPrint(g->screen, g->regfont, g->screen->w - tw, th + gap, BAT_WHITE, buf);
    th += tigrTextHeight(g->regfont, buf);
    memset(buf, 0, sizeof(char));
}

void game_free(g_obj *g) {
    anim_free(&p_idle);
    anim_free(&p_runr);
    anim_free(&p_runl);
    anim_free(&p_jumpr);
    anim_free(&p_jumpl);
    free(g);
}