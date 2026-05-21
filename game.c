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

#define MOUSE_LEFT   (1 << 0)
#define MOUSE_RIGHT  (1 << 1)
#define MOUSE_MIDDLE (1 << 2)

#define dbg_print printf("DEBUG HIT in %s:%d@%s\n", __FILE__, __LINE__, __FUNCTION__)

// Declare the different animations for player
anim_t p_idle;
anim_t p_runr;
anim_t p_runl;
anim_t p_jumpr;
anim_t p_jumpl;

// Declare animation for batarangs
// Load from file for only one
anim_t batarang_rotate[MAX_BATRS];

// Declare animation for dragons
// Load from file for only one
anim_t dragon_flyl[MAX_DRAGONS];
anim_t dragon_flyr[MAX_DRAGONS];

void game_update_mouse(g_obj *g) {
    g->mouse_prev_buttons = g->mouse_buttons;
    tigrMouse(g->screen, &g->mousex, &g->mousey, &g->mouse_buttons);
}

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

    // batarang
    anim_load_sprites_file(&batarang_rotate[0], "res/sprites/batarang_rotate/frame_%03d.png", "BATARANG ROTATE", 8, WEAPON, NONE, 0.06f, 1);

    for(int i = 1; i < MAX_BATRS; i++) {
        anim_load_sprites_mem(&batarang_rotate[i], batarang_rotate[0].frames, "BATARANG ROTATE", 8, WEAPON, NONE, 0.1f, 1);
    }

    // dragon
    anim_load_sprites_file(&dragon_flyl[0], "res/sprites/dragon_flyl/frame_%03d.png", "DRAGON FLY LEFT", 25, NPC, NONE, 0.1f, 1);
    anim_load_sprites_file(&dragon_flyr[0], "res/sprites/dragon_flyr/frame_%03d.png", "DRAGON FLY RIGHT", 25, NPC, NONE, 0.1f, 1);
    for(int i = 1; i < MAX_DRAGONS; i++) {
        anim_load_sprites_mem(&dragon_flyl[i], dragon_flyl[0].frames, "DRAGON FLY LEFT", 25, NPC, NONE, 0.1f, 1);
        anim_load_sprites_mem(&dragon_flyr[i], dragon_flyr[0].frames, "DRAGON FLY RIGHT", 25, NPC, NONE, 0.1f, 1);
    }

    // init current player animation and starting sprite
    g->p.curr_anim = &p_idle;
    g->p.curr_sprite = g->p.curr_anim->frames[0];
    g->p.xpos = 0;
    g->p.ypos = g->screen->h - g->p.curr_sprite->h;
    g->p.xvel = 0;
    g->p.yvel = 0;
    g->p.health = 100;

    // mark all batarangs as inactive and map the animations
    for(int i = 0; i < MAX_BATRS; i++) {
        g->batrs[i].active = 0;
        g->batrs[i].curr_anim   = &batarang_rotate[i];
    }

    // mark all dragons inactive and init other attributes
    for(int i = 0; i < MAX_DRAGONS; i++) {
        g->dragons[i].active = 0;
        g->dragons[i].curr_anim = &dragon_flyl[i];
    }
    
    // init_backdrop
    tigrClear(g->canvas, BAT_BLUE1);
    Tigr *bg_image = tigrLoadImage("res/background.png");
    if(bg_image) {
        tigrBlit(g->canvas, bg_image, 0,0,0,0, bg_image->w, bg_image->h);
        tigrFree(bg_image);
    }
    
    int tw = tigrTextWidth(g->titlefont, G_TITLE);
    tigrPrint(g->canvas, g->titlefont, (G_W-tw)/2, 0, BAT_BLUE3, G_TITLE);

    return g;
}

void game_update(g_obj *g, float dt) {
    if(dt > 0.1f)dt = 0.1f;

    // handle inputs
    // update velocity with movement
    if (tigrKeyHeld(g->screen, TK_RIGHT) || tigrKeyHeld(g->screen, 'D')) {
        g->p.h_dir = DIR_RIGHT;
        if(!g->p.jumping) g->p.curr_anim = &p_runr;
        anim_advance_frame(g->p.curr_anim, dt);
        g->p.curr_sprite = g->p.curr_anim->frames[g->p.curr_anim->currframe];
        g->p.xvel += 40;
    }
    if (tigrKeyHeld(g->screen, TK_LEFT) || tigrKeyHeld(g->screen, 'A')) {
        g->p.h_dir = DIR_LEFT;
        if(!g->p.jumping) g->p.curr_anim = &p_runl;
        anim_advance_frame(g->p.curr_anim, dt);
        g->p.curr_sprite = g->p.curr_anim->frames[g->p.curr_anim->currframe];
        g->p.xvel -= 40;
    }
    if (tigrKeyDown(g->screen, TK_SPACE)) {
        if(!g->p.jumping) {
            g->p.jumping = 1;
            g->p.yvel -= 700;
            if(g->p.h_dir == DIR_LEFT) {
                g->p.curr_anim = &p_jumpl;
            } else {
                g->p.curr_anim = &p_jumpr;
            }
            g->p.curr_anim->currframe = 0;
        }
    }

    game_update_mouse(g);
    if((g->mouse_buttons & MOUSE_LEFT) && !(g->mouse_prev_buttons & MOUSE_LEFT)) {
        // click detected
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
            w->curr_sprite = w->curr_anim->frames[0];
            // xpos is middle, ypos is little above middle of player
            // since this is a projectile, treat as point. Find center coordinates
            float delta = 70.0;
            w->xpos = (g->p.xpos + g->p.curr_sprite->w/2) - (w->curr_sprite->w/2);
            w->ypos = (g->p.ypos + g->p.curr_sprite->h/2 - delta) - (w->curr_sprite->h/2);
            
            // calculate velocity angle from mousex, mousey
            float dx = (float)(g->mousex - w->xpos);
            float dy = (float)(g->mousey - w->ypos);
            float d  = sqrt(dx*dx + dy*dy);
            if(d == 0.0f)d = 1.0f;
            float sintheta = dy/d;
            float costheta = dx/d;
            w->xvel = 500.0 * costheta;
            w->yvel = 500.0 * sintheta;
        }
    }

    // randomly initialize a dragon if there is an empty slot
    if(rand() % 233 == 0) {
        npc_t *d = NULL;
        int d_yoff = 0;
        for(int i = 0; i < MAX_DRAGONS; i++) {
            if(!g->dragons[i].active) {
                d = &g->dragons[i];
                d_yoff = i;
                break;
            }
        }
        if(d) {
            d->active = 1;
            d->curr_sprite = d->curr_anim->frames[0];
            // start at right end of screen
            d->xpos = g->screen->w - d->curr_sprite->w;
            // depending on your index, start at a height offset
            d->ypos = d_yoff * d->curr_sprite->h + 10;
            d->xvel = -300;
            d->yvel = 0; // no vertical movement
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

    if(g->p.jumping && (g->p.curr_anim->action == JUMP)) {
        anim_advance_frame(g->p.curr_anim, dt);
        g->p.curr_sprite = g->p.curr_anim->frames[g->p.curr_anim->currframe];
    }

    if((int)g->p.ypos == (g->screen->h - g->p.curr_sprite->h)) {
        g->p.jumping = 0;
    }

    // if sprite x velocity is 0, 
    // if sprite y pos is ground, switch to idle
    if(((int)g->p.xvel == 0) && !g->p.jumping) {
        g->p.curr_anim = &p_idle;
        anim_advance_frame(g->p.curr_anim, dt);
        g->p.curr_sprite = g->p.curr_anim->frames[g->p.curr_anim->currframe];
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
                w->xpos += w->xvel * dt;
                w->ypos += w->yvel * dt;

                // advance frame
                anim_advance_frame(w->curr_anim, dt);
                w->curr_sprite = w->curr_anim->frames[w->curr_anim->currframe];
            }
        }
    }

    ////////////////////////////////
    //    DRAGONS UPDATE          //
    ///////////////////////////////
    // cycle through active dragons, update the position and advance frame
    // if curr position is beyond canvas limits, reverse x vel
    for(int i = 0; i < MAX_DRAGONS; i++) {
        npc_t *d = &g->dragons[i];
        if(d->active) {
            // check curr positions, only need for x
            if((d->xpos < 0) || (d->xpos + d->curr_sprite->w > g->canvas->w)) {
                d->xvel = -d->xvel;
                if(d->curr_anim == &dragon_flyl[i]) {
                    d->curr_anim = &dragon_flyr[i];
                } else {
                    d->curr_anim = &dragon_flyl[i];
                }
            }
            // update x positions with velocity
            d->xpos += d->xvel * dt;

            // advance frame
            anim_advance_frame(d->curr_anim, dt);
            d->curr_sprite = d->curr_anim->frames[d->curr_anim->currframe];
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
    // draw all active dragons
    for(int i = 0; i < MAX_DRAGONS; i++) {
        npc_t *d = &g->dragons[i];
        if(d->active) {
            tigrBlitAlpha(screen, 
                      d->curr_sprite, 
                      (int)d->xpos, 
                      (int)d->ypos, 
                      0, 
                      0, 
                      d->curr_sprite->w,
                      d->curr_sprite->h,
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
    snprintf(buf, 49, "Current Player Animation: %s\n", g->p.curr_anim->name);
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
    anim_free(&batarang_rotate[0]);
    anim_free(&dragon_flyl[0]);
    free(g);
}