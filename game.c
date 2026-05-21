#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "colors.h"
#include "config.h"
#include "tigr/tigr.h"

#define _min(a, b)   ((a) < (b) ? (a) : (b))
#define _max(a, b)   ((a) > (b) ? (a) : (b))

#define DIR_LEFT    (1)
#define DIR_RIGHT   (2)

#define dbg_print printf("DEBUG HIT in %s:%d@%s\n", __FILE__, __LINE__, __FUNCTION__)

g_obj* game_init(Tigr* screen, Tigr* canvas) {
    // initialize game object
    g_obj* g = malloc(sizeof(*g));
    g->fcount = S_FRAME_CNT;
    g->screen = screen;
    g->canvas = canvas;

    // load fonts
    Tigr *tfi = tigrLoadImage("res/fonts/profont29.png");
    if(tfi) {
        g->titlefont = tigrLoadFont(tfi, TCP_ASCII);
    } else {
        g->titlefont = tfont;
    }
    
    Tigr *rfi = tigrLoadImage("res/fonts/profont17.png");
    if(rfi) {
        g->regfont = tigrLoadFont(rfi, TCP_ASCII);
    } else {
        g->regfont = tfont;
    }
    
    // init sprites
    g->anim_idle = malloc(sizeof(Tigr *) * g->fcount);
    g->anim_runr = malloc(sizeof(Tigr *) * g->fcount);
    g->anim_runl = malloc(sizeof(Tigr *) * g->fcount);
    g->anim_jumpr = malloc(sizeof(Tigr *) * g->fcount);
    g->anim_jumpl = malloc(sizeof(Tigr *) * g->fcount);
    
    char buf[100] = { 0 };
    for(int i = 0; i < g->fcount; i++) {
        snprintf(buf, 99, "res/sprites/idle/frame_%03d.png", i+1);
        g->anim_idle[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }
    for(int i = 0; i < g->fcount; i++) {
        snprintf(buf, 99, "res/sprites/runright/frame_%03d.png", i+1);
        g->anim_runr[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }
    for(int i = 0; i < g->fcount; i++) {
        snprintf(buf, 99, "res/sprites/runleft/frame_%03d.png", i+1);
        g->anim_runl[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }
    for(int i = 0; i < g->fcount; i++) {
        snprintf(buf, 99, "res/sprites/jumpright/frame_%03d.png", i+1);
        g->anim_jumpr[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }
    for(int i = 0; i < g->fcount; i++) {
        snprintf(buf, 99, "res/sprites/jumpleft/frame_%03d.png", i+1);
        g->anim_jumpl[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }

    g->curr_anim = g->anim_idle;
    g->curr_sprite = g->curr_anim[0];
    g->p.xpos = 0;
    g->p.ypos = g->screen->h - g->curr_sprite->h;
    g->p.xvel = 0;
    g->p.yvel = 0;

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

Tigr* player_anim_get_next(g_obj *g, float dt) {
    static float animation_timer = 0.0f;
    const float frame_duration = 0.07f;

    animation_timer += dt;

    if (animation_timer >= frame_duration) {
        // in case of jump animation, run it only once.
        if(g->curr_anim == g->anim_jumpl || g->curr_anim == g->anim_jumpr) {
            g->fidx = _min(g->fidx + 1, (g->fcount - 1));
        } else {
            g->fidx = (g->fidx + 1) % g->fcount;
        }
        animation_timer -= frame_duration; 
    }

    return g->curr_anim[g->fidx];
}

void game_update(g_obj *g, float dt) {
    if(dt > 0.1f)dt = 0.1f;

    // update velocity with movement
    if (tigrKeyHeld(g->screen, TK_RIGHT) || tigrKeyHeld(g->screen, 'D')) {
        g->p.h_dir = DIR_RIGHT;
        if(!g->p.jumping) g->curr_anim = g->anim_runr;
        g->curr_sprite = player_anim_get_next(g, dt);
        g->p.xvel += 40;
    }
    if (tigrKeyHeld(g->screen, TK_LEFT) || tigrKeyHeld(g->screen, 'A')) {
        g->p.h_dir = DIR_LEFT;
        if(!g->p.jumping) g->curr_anim = g->anim_runl;
        g->curr_sprite = player_anim_get_next(g, dt);
        g->p.xvel -= 40;
    }
    if (tigrKeyDown(g->screen, TK_SPACE)) {
        if(!g->p.jumping) {
            g->p.jumping = 1;
            g->p.yvel -= 700;
            if(g->p.h_dir == DIR_LEFT) {
                g->curr_anim = g->anim_jumpl;
            } else {
                g->curr_anim = g->anim_jumpr;
            }
            g->fidx = 0;
        }
    }

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
    g->p.xpos = _min(g->p.xpos, g->screen->w - g->curr_sprite->w);
    // limit y position
    g->p.ypos = _max(g->p.ypos, 0);
    g->p.ypos = _min(g->p.ypos, g->screen->h - g->curr_sprite->h);

    if(g->p.jumping && 
        (g->curr_anim == g->anim_jumpl || g->curr_anim == g->anim_jumpr)) {
        g->curr_sprite = player_anim_get_next(g, dt);
    }

    if((int)g->p.ypos == (g->screen->h - g->curr_sprite->h)) {
        g->p.jumping = 0;
    }

    // if sprite x velocity is 0, 
    // if sprite y pos is ground, switch to idle
    if(((int)g->p.xvel == 0) && !g->p.jumping) {
        g->curr_anim = g->anim_idle;
        g->curr_sprite = player_anim_get_next(g, dt);
    }
}

char* get_curr_anim_name(g_obj *g) {
    if(g->curr_anim == g->anim_idle) {
        return "IDLE";
    } else if(g->curr_anim == g->anim_jumpl) {
        return "JUMP LEFT";
    } else if(g->curr_anim == g->anim_jumpr) {
        return "JUMP RIGHT";
    } else if(g->curr_anim == g->anim_runl) {
        return "RUN LEFT";
    } else if(g->curr_anim == g->anim_runr) {
        return "RUN RIGHT";
    } else {
        return "UNKNOWN ANIM";
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

    // current animation
    snprintf(buf, 49, "Current Animation: %s\n", get_curr_anim_name(g));
    tw = tigrTextWidth(g->regfont, buf);
    tigrPrint(g->screen, g->regfont, g->screen->w - tw, th + gap, BAT_WHITE, buf);
    memset(buf, 0, sizeof(char));
}

void free_game(g_obj *g) {
    free(g->anim_idle);
    free(g->anim_runr);
    free(g->anim_runl);
    free(g->anim_jumpr);
    free(g->anim_jumpl);
    free(g);
}