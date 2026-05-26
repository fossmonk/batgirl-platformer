#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anim.h"
#include "colors.h"
#include "config.h"
#include "font.h"
#include "game.h"
#include "tiny_audio.h"
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

#define game_obj_blit_alpha(obj) do { if((obj)->active) { tigrBlitAlpha(screen,(obj)->curr_sprite,(int)(obj)->xpos,(int)(obj)->ypos,0,0,(obj)->curr_sprite->w,(obj)->curr_sprite->h,1.0f); } } while(0)

#define dbg_print printf("DEBUG HIT in %s:%d@%s\n", __FILE__, __LINE__, __FUNCTION__)

// Declare the different animations for player
anim_t p_idle;
anim_t p_runr;
anim_t p_runl;
anim_t p_jumpr;
anim_t p_jumpl;
anim_t p_deathburnl;
anim_t p_deathhitl;
anim_t p_deathburnr;
anim_t p_deathhitr;

// Declare different game sounds
ta_sound theme;
ta_sound jump;
ta_sound batarang[MAX_BATRS];
ta_sound player_burn;
ta_sound player_hit;
ta_sound dragon_burn[MAX_DRAGONS];
ta_sound intro;

// Declare animation for batarangs
// Load from file for only one
anim_t batarang_rotate[MAX_BATRS];

// Declare animation for dragons
// Load from file for only one
anim_t dragon_flyl[MAX_DRAGONS];
anim_t dragon_flyr[MAX_DRAGONS];
anim_t dragon_deathl[MAX_DRAGONS];
anim_t dragon_deathr[MAX_DRAGONS];

// Declare fireball animation
anim_t fireball[MAX_FIREBALLS];

// Declare spiky animation
anim_t spiky[MAX_SPIKY];
anim_t spiky_death[MAX_SPIKY];

void game_update_mouse(game_t *g) {
    g->mouse_prev_buttons = g->mouse_buttons;
    tigrMouse(g->screen, &g->mousex, &g->mousey, &g->mouse_buttons);
}

int game_detect_obj_collision_aabb(obj_t* attack, obj_t* attackee) {
    int collided = 0;

    int s_h = attack->curr_sprite->h;
    int s_w = attack->curr_sprite->w;

    float xlim = s_w/1.2;
    float ylim = s_h/1.2;

    float cx1 = attack->xpos + attack->curr_sprite->w/2;
    float cx2 = attackee->xpos + attackee->curr_sprite->w/2;
    float cy1 = attack->ypos + attack->curr_sprite->h/2;
    float cy2 = attackee->ypos + attackee->curr_sprite->h/2;

    if(fabsf(cx2-cx1) < xlim && fabsf(cy2-cy1) < ylim)collided = 1;

    return collided;
}

anim_t * game_get_player_death_type(game_t *g) {
    if(g->p.h_dir == DIR_LEFT) {
        if(g->p.dtype == BURN) {
            return &p_deathburnl;
        } else {
            return &p_deathhitl;
        }
    } else {
        if(g->p.dtype == BURN) {
            return &p_deathburnr;
        } else {
            return &p_deathhitr;
        }
    }
}

game_t* game_init(Tigr* screen, Tigr* canvas) {
    // initialize game object
    game_t* g = malloc(sizeof(*g));
    g->screen = screen;
    g->canvas = canvas;
    g->game_over = 0;
    g->game_wclosed = 0;

    // init sound engine and load sounds
    ta_sound_init();
    ta_sound_load(&theme, SOUND_THEME);
    ta_sound_load(&intro, SOUND_INTRO);
    ta_sound_load(&jump, SOUND_JUMP);
    ta_sound_load(&player_burn, SOUND_BURN);
    ta_sound_load_copies(dragon_burn, SOUND_EXPLOSION, MAX_DRAGONS);
    ta_sound_load_copies(batarang, SOUND_BATARANG, MAX_BATRS);

    // load fonts
    font_load_ascii(H1_FONT, &g->h1font);
    font_load_ascii(H2_FONT, &g->h2font);
    font_load_ascii(TEXT_FONT, &g->textfont);
    
    // load sprites
    // player
    anim_load_sprites_file(&p_idle, PLAYER_IDLE, "IDLE", 7, PLAYER, IDLE, 0.07f, 1);
    anim_load_sprites_file(&p_runr, PLAYER_RUNR, "RUN RIGHT", 7, PLAYER, RUN, 0.07f, 1);
    anim_load_sprites_file(&p_runl, PLAYER_RUNL, "RUN LEFT", 7, PLAYER, RUN, 0.07f, 1);
    anim_load_sprites_file(&p_jumpr, PLAYER_JUMPR, "JUMP RIGHT", 7, PLAYER, JUMP, 0.07f, 0);
    anim_load_sprites_file(&p_jumpl, PLAYER_JUMPL, "JUMP LEFT", 7, PLAYER, JUMP, 0.07f, 0);
    anim_load_sprites_file(&p_deathburnl, PLAYER_DEATH_BURNL, "DEATH", 16, PLAYER, DEATH, 0.07f, 0);
    anim_load_sprites_file(&p_deathhitl, PLAYER_DEATH_HITL, "DEATH", 16, PLAYER, DEATH, 0.07f, 0);
    anim_load_sprites_file(&p_deathburnr, PLAYER_DEATH_BURNR, "DEATH", 16, PLAYER, DEATH, 0.07f, 0);
    anim_load_sprites_file(&p_deathhitr, PLAYER_DEATH_HITR, "DEATH", 16, PLAYER, DEATH, 0.07f, 0);

    // batarang
    anim_load_sprites_file(&batarang_rotate[0], BATARANG_ROT, "BATARANG ROTATE", 8, OBJECT, NONE, 0.06f, 1);

    for(int i = 1; i < MAX_BATRS; ++i) {
        anim_load_sprites_mem(&batarang_rotate[i], batarang_rotate[0].frames, "BATARANG ROTATE", 8, OBJECT, NONE, 0.08f, 1);
    }

    // dragon
    anim_load_sprites_file(&dragon_flyl[0], DRAGON_FLYL, "DRAGON FLY LEFT", 25, NPC, NONE, 0.1f, 1);
    anim_load_sprites_file(&dragon_flyr[0], DRAGON_FLYR, "DRAGON FLY RIGHT", 25, NPC, NONE, 0.1f, 1);
    anim_load_sprites_file(&dragon_deathl[0], DRAGON_DEATHL, "DRAGON DEATH LEFT", 9, NPC, DEATH, 0.07f, 0);
    anim_load_sprites_file(&dragon_deathr[0], DRAGON_DEATHR, "DRAGON DEATH RIGHT", 9, NPC, DEATH, 0.07f, 0);
    for(int i = 1; i < MAX_DRAGONS; ++i) {
        anim_load_sprites_mem(&dragon_flyl[i], dragon_flyl[0].frames, "DRAGON FLY LEFT", 25, NPC, NONE, 0.1f, 1);
        anim_load_sprites_mem(&dragon_flyr[i], dragon_flyr[0].frames, "DRAGON FLY RIGHT", 25, NPC, NONE, 0.1f, 1);
        anim_load_sprites_mem(&dragon_deathl[i], dragon_deathl[0].frames, "DRAGON DEATH LEFT", 9, NPC, DEATH, 0.07f, 0);
        anim_load_sprites_mem(&dragon_deathr[i], dragon_deathr[0].frames, "DRAGON DEATH RIGHT", 9, NPC, DEATH, 0.07f, 0);
    }

    // fireball
    anim_load_sprites_file(&fireball[0], FIREBALL, "FIREBALL", 4, OBJECT, NONE, 0.01f, 1);
    for(int i = 1; i < MAX_FIREBALLS; ++i) {
        anim_load_sprites_mem(&fireball[i], fireball[0].frames, "FIREBALL", 4, OBJECT, NONE, 0.01f, 1);
    }

    // spiky
    anim_load_sprites_file(&spiky[0], SPIKY, "SPIKY", 8, NPC, NONE, 0.1f, 1);
    anim_load_sprites_file(&spiky_death[0], SPIKY_DEATH, "SPIKY DEATH", 7, NPC, NONE, 0.04f, 0);
    for(int i = 1; i < MAX_SPIKY; ++i) {
        anim_load_sprites_mem(&spiky[i], spiky[0].frames, "SPIKY", 8, NPC, NONE, 0.1f, 1);
        anim_load_sprites_mem(&spiky_death[i], spiky_death[0].frames, "SPIKY DEATH", 7, NPC, NONE, 0.04f, 0);
    }

    // init current player animation and starting sprite
    g->p.curr_anim = &p_idle;
    g->p.curr_sprite = g->p.curr_anim->frames[0];
    g->p.xpos = (g->screen->w - g->p.curr_sprite->w)/2;
    g->p.ypos = GAME_GROUND_Y - g->p.curr_sprite->h;
    g->p.xvel = 0;
    g->p.yvel = 0;
    g->p.health = 100;
    g->p.dying = 0;
    g->p.jumping = 0;
    g->p.score = 0;

    // mark all batarangs as inactive and map the animations
    for(int i = 0; i < MAX_BATRS; ++i) {
        g->batrs[i].active = 0;
        g->batrs[i].curr_anim = &batarang_rotate[i];
    }

    // mark all dragons inactive and map animations
    for(int i = 0; i < MAX_DRAGONS; ++i) {
        g->dragons[i].active = 0;
        g->dragons[i].dying = 0;
        g->dragons[i].curr_anim = &dragon_flyl[i];
    }

    // mark all fireballs inactive and map animations
    for(int i = 0; i < MAX_FIREBALLS; ++i) {
        g->fireballs[i].active = 0;
        g->fireballs[i].curr_anim = &fireball[i];
    }

    // mark all spiky inactive and map animations
    for(int i = 0; i < MAX_SPIKY; ++i) {
        g->spiky[i].active = 0;
        g->spiky[i].dying = 0;
        g->spiky[i].curr_anim = &spiky[i];
    }

    // init_backdrop
    tigrClear(g->canvas, BAT_BLUE1);
    Tigr *bg_image = tigrLoadImage(BACKDROP);
    if(bg_image) {
        tigrBlit(g->canvas, bg_image, 0,0,0,0, bg_image->w, bg_image->h);
        tigrFree(bg_image);
    }
    
    int tw = tigrTextWidth(g->h1font, G_TITLE);
    tigrPrint(g->canvas, g->h1font, (G_W-tw)/2, 0, BAT_WHITE, G_TITLE);

    return g;
}

void game_update(game_t *g, float dt) {
    if(dt > 0.1f)dt = 0.1f;
    float h_accel = 5000.0f;

    // handle inputs
    // update velocity with movement
    if(!g->p.dying) {
        if (tigrKeyHeld(g->screen, TK_RIGHT) || tigrKeyHeld(g->screen, 'D')) {
            g->p.h_dir = DIR_RIGHT;
            if(!g->p.jumping) g->p.curr_anim = &p_runr;
            anim_advance_frame(g->p.curr_anim, dt);
            g->p.curr_sprite = g->p.curr_anim->frames[g->p.curr_anim->currframe];
            g->p.xvel += h_accel*dt;
        }
        if (tigrKeyHeld(g->screen, TK_LEFT) || tigrKeyHeld(g->screen, 'A')) {
            g->p.h_dir = DIR_LEFT;
            if(!g->p.jumping) g->p.curr_anim = &p_runl;
            anim_advance_frame(g->p.curr_anim, dt);
            g->p.curr_sprite = g->p.curr_anim->frames[g->p.curr_anim->currframe];
            g->p.xvel -= h_accel*dt;
        }
        if (tigrKeyDown(g->screen, TK_SPACE) && !g->p.jumping) {
            g->p.jumping = 1;
            g->p.yvel -= 700;
            if(g->p.h_dir == DIR_LEFT) {
                g->p.curr_anim = &p_jumpl;
            } else {
                g->p.curr_anim = &p_jumpr;
            }
            g->p.curr_anim->currframe = 0;
            ta_sound_play(&jump, 0);
        }
    
        game_update_mouse(g);
        if((g->mouse_buttons & MOUSE_LEFT) && !(g->mouse_prev_buttons & MOUSE_LEFT)) {
            // click detected
            // check for an empty slot in batarangs array
            // assign to one, init position as player's hdir corner
            obj_t *w = NULL;
            int b_index = 0;
            for(int i = 0; i < MAX_BATRS; ++i) {
                if(!g->batrs[i].active) {
                    w = &g->batrs[i];
                    b_index = i;
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
                ta_sound_play(&batarang[b_index], 0);
            }
        }
    }

    // randomly initialize a dragon if there is an empty slot
    if(rand() % 233 == 0) {
        npc_t *d = NULL;
        int d_yoff = 0;
        for(int i = 0; i < MAX_DRAGONS; ++i) {
            if(!g->dragons[i].active) {
                d = &g->dragons[i];
                d_yoff = i;
                break;
            }
        }
        if(d) {
            d->active = 1;
            d->dying = 0;
            d->curr_anim = &dragon_flyl[d_yoff];
            d->curr_sprite = d->curr_anim->frames[0];
            // start at right end of screen
            d->xpos = g->screen->w - d->curr_sprite->w;
            // depending on your index, start at a height offset
            d->ypos = d_yoff * d->curr_sprite->h;
            d->xvel = -300;
            d->yvel = 0; // no vertical movement
        }
    }

    // randomly initialize a spiky if there is an empty slot
    if(rand() % 379 == 0) {
        npc_t *spk = NULL;
        int idx = 0;
        for(int i = 0; i < MAX_SPIKY; ++i) {
            if(!g->spiky[i].active) {
                spk = &g->spiky[i];
                idx = i;
                break;
            }
        }
        if(spk) {
            spk->active = 1;
            spk->dying = 0;
            spk->curr_anim = &spiky[idx];
            spk->curr_sprite = spk->curr_anim->frames[0];
            if(rand() % 7 == 0) {
                // start from right
                spk->xpos = g->screen->w - spk->curr_sprite->w;
                spk->ypos = GAME_GROUND_Y - spk->curr_sprite->w;
                spk->xvel = -400;
                spk->yvel = 0;
            } else {
                // start from left
                spk->xpos = 0;
                spk->ypos = GAME_GROUND_Y - spk->curr_sprite->w;
                spk->xvel = 400;
                spk->yvel = 0;
            }
        }
    }

    /////////////////////////////
    //    PLAYER UPDATES       //
    ////////////////////////////
    // check for player death
    if(g->p.dying) {
        if(g->p.curr_anim->action != DEATH) {
            g->p.curr_anim = game_get_player_death_type(g);
            g->p.curr_anim->currframe = 0;
        } else {
            if (g->p.curr_anim->currframe == g->p.curr_anim->framecount - 1) {
                g->game_over = 1;
            }
        }
        anim_advance_frame(g->p.curr_anim, dt);
        g->p.curr_sprite = g->p.curr_anim->frames[g->p.curr_anim->currframe];
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
    g->p.xpos = _min(g->p.xpos, g->screen->w - g->p.curr_sprite->w);
    // limit y position
    g->p.ypos = _max(g->p.ypos, 0);
    g->p.ypos = _min(g->p.ypos, GAME_GROUND_Y - g->p.curr_sprite->h);

    if(g->p.jumping && (g->p.curr_anim->action == JUMP)) {
        anim_advance_frame(g->p.curr_anim, dt);
        g->p.curr_sprite = g->p.curr_anim->frames[g->p.curr_anim->currframe];
    }

    if(g->p.ypos >= (float)(GAME_GROUND_Y - g->p.curr_sprite->h)) {
        g->p.jumping = 0;
    } else {
        g->p.jumping = 1;
    }
    // if sprite x velocity is 0, 
    // if sprite y pos is ground, switch to idle
    if(((int)(g->p.xvel) == 0) && !g->p.jumping && !g->p.dying) {
        g->p.curr_anim = &p_idle;
        anim_advance_frame(g->p.curr_anim, dt);
        g->p.curr_sprite = g->p.curr_anim->frames[g->p.curr_anim->currframe];
    }

      ////////////////////////////////
     //    BATARANGS UPDATE       //
    ///////////////////////////////
    // cycle through all batarangs, update the position and advance the frame
    // if curr position is beyond canvas limits, mark it as inactive
    for(int i = 0; i < MAX_BATRS; ++i) {
        obj_t *w = &g->batrs[i];
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
    for(int i = 0; i < MAX_DRAGONS; ++i) {
        npc_t *d = &g->dragons[i];
        if(d->active) {
            // check for dying 
            if(d->dying) {
                // start death animation if not already
                if(d->curr_anim == &dragon_flyl[i]) {
                    d->curr_anim = &dragon_deathl[i];
                    d->curr_anim->currframe = 0;
                } else if (d->curr_anim == &dragon_flyr[i]) {
                    d->curr_anim = &dragon_deathr[i];
                    d->curr_anim->currframe = 0;
                } else {
                    // already in dying animation
                    // check for last frame
                    if(d->curr_anim->currframe == d->curr_anim->framecount - 1) {
                        d->active = 0;
                    }
                }
            } else {
                // check curr positions, only need for x
                if((d->xpos <= 0) || (d->xpos + d->curr_sprite->w >= g->canvas->w)) {
                    d->xvel = -d->xvel;
                    if(d->curr_anim == &dragon_flyl[i]) {
                        d->curr_anim = &dragon_flyr[i];
                    } else {
                        d->curr_anim = &dragon_flyl[i];
                    }
                }
                // update x positions with velocity
                d->xpos += d->xvel * dt;
    
                // randomly drop a fireball if conditions are met
                // => free slot in fireball array
                if(rand() % 678 == 0) {
                    obj_t *fb = NULL;
                    for(int j = 0; j < MAX_FIREBALLS_PER_DRAGON; ++j) {
                        if(!g->fireballs[MAX_FIREBALLS_PER_DRAGON*i + j].active) {
                            fb = &g->fireballs[MAX_FIREBALLS_PER_DRAGON*i + j];
                            break;
                        }
                    }
                    if(fb) {
                        fb->active = 1;
                        fb->curr_sprite = fb->curr_anim->frames[0];
                        // drop from dragon's xcenter, ycenter + delta
                        fb->xpos = d->xpos + d->curr_sprite->w/2;
                        fb->ypos = d->ypos + d->curr_sprite->h/2 + 30.0;
                        // it's a drop, so no xvel, only yvel = g*dt
                        fb->xvel = 0;
                        fb->yvel = 400*dt;
                    }
                }
            }

            // advance frame
            anim_advance_frame(d->curr_anim, dt);
            d->curr_sprite = d->curr_anim->frames[d->curr_anim->currframe];
        }
    }

    ////////////////////////////////
    //     FIREBALL UPDATE        //
    ///////////////////////////////
    // cycle through fireballs and update position and velocity of actives
    for(int i = 0; i < MAX_FIREBALLS; ++i) {
        obj_t *fb = &g->fireballs[i];
        if(fb) {
            // check for ypos exceeding ground height
            if(fb->ypos > GAME_GROUND_Y) {
                fb->active = 0;
            } else {
                // update yvel and ypos
                fb->yvel += 400 * dt;
                fb->ypos += fb->yvel * dt;

                // advance frame
                anim_advance_frame(fb->curr_anim, dt);
                fb->curr_sprite = fb->curr_anim->frames[fb->curr_anim->currframe];
            }
        }
    }

    ////////////////////////////////
    //      SPIKY UPDATE          //
    ///////////////////////////////
    // cycle throught spikys and update position and velocity of actives
    for(int i = 0; i < MAX_SPIKY; ++i) {
        npc_t *spk = &g->spiky[i];
        if(spk->active) {
            if(spk->dying) {
                // start death animation if not already
                if(spk->curr_anim != &spiky_death[i]) {
                    spk->curr_anim = &spiky_death[i];
                    spk->curr_anim->currframe = 0;
                } else {
                    // already in dying animation
                    // check for last frame
                    if(spk->curr_anim->currframe == spk->curr_anim->framecount - 1) {
                        spk->active = 0;
                    }
                }
            } else {
                if(spk->xpos < 0 || spk->xpos > g->screen->w) {
                    spk->active = 0;
                } else {
                    // update xpos
                    spk->xpos += spk->xvel * dt;
    
                }
            }
            // advance frame
            anim_advance_frame(spk->curr_anim, dt);
            spk->curr_sprite = spk->curr_anim->frames[spk->curr_anim->currframe];
        }
    }

    ////////COLLISION DETECTION/////////////
    // check for batarang collision with dragons
    for(int i = 0; i < MAX_DRAGONS; ++i) {
        npc_t *dragon = &g->dragons[i];
        if(dragon->active) {
            for(int j = 0; j < MAX_BATRS; ++j) {
                obj_t *batr = &g->batrs[j];
                if(batr->active) {
                    if(game_detect_obj_collision_aabb((obj_t *)&g->batrs[j], (obj_t *)dragon)) {
                        dragon->dying = 1;
                        batr->active = 0;
                        g->p.score += 10;
                        ta_sound_play(&dragon_burn[i], 0);
                    }
                }
            }
        }
    }

    // check for fireball collision with player
    for(int i = 0; i < MAX_FIREBALLS; ++i) {
        obj_t *fb = &g->fireballs[i];
        if(fb->active) {
            if(game_detect_obj_collision_aabb(fb, (obj_t *)&g->p)) {
                g->p.dying = 1;
                g->p.dtype = BURN;
                fb->active = 0;
                ta_sound_play(&player_burn, 0);
            }
        }
    }

    // check for spiky collision with player
    for(int i = 0; i < MAX_SPIKY; ++i) {
        npc_t *spk = &g->spiky[i];
        if(spk->active && !spk->dying && !g->p.dying) {
            if(game_detect_obj_collision_aabb((obj_t *)spk, (obj_t *)&g->p)) {
                g->p.dying = 1;
                g->p.dtype = HIT;
                ta_sound_play(&player_burn, 0);
            }
        }
    }

    // check for spiky collision with each other
    // C(n,r)
    for(int i = 0; i < MAX_SPIKY; ++i) {
        for(int j = i + 1; j < MAX_SPIKY; ++j) {
            npc_t *spk0 = &g->spiky[i];
            npc_t *spk1 = &g->spiky[j];
            if(spk0->active && !spk0->dying && spk1->active && !spk1->dying) {
                if(game_detect_obj_collision_aabb((obj_t *)spk0, (obj_t *)spk1)) {
                    spk0->dying = 1;
                    spk1->dying = 1;
                }
            }
        }
    }
}

void game_draw(Tigr* screen, Tigr* canvas, game_t *g) {
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
    for(int i = 0; i < MAX_BATRS; ++i) {
        obj_t *w = &g->batrs[i];
        game_obj_blit_alpha(w);
    }
    // draw all active dragons
    for(int i = 0; i < MAX_DRAGONS; ++i) {
        npc_t *d = &g->dragons[i];
        game_obj_blit_alpha(d);
    }
    // draw all active fireballs
    for(int i = 0; i < MAX_FIREBALLS; ++i) {
        obj_t *fb = &g->fireballs[i];
        game_obj_blit_alpha(fb);
    }
    // draw all active spiky
    for(int i = 0; i < MAX_SPIKY; ++i) {
        npc_t *spk = &g->spiky[i];
        game_obj_blit_alpha(spk);
    }

    // draw score to top left
    tigrPrint(screen, g->textfont, 0, 0, tigrRGB(0, 255, 0), "Your Score: %d", g->p.score);
}

void game_over_draw(Tigr* screen, Tigr* canvas, game_t *g) {
    // draw canvas to screen
    tigrBlit(screen, canvas, 0, 0, 0, 0, canvas->w, canvas->h);

    // draw all active dragons
    for(int i = 0; i < MAX_DRAGONS; ++i) {
        npc_t *d = &g->dragons[i];
        game_obj_blit_alpha(d);
    }

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

    // blit "gameover" image
    Tigr *gameoverimage = tigrLoadImage("res/gameover.png");
    int gw = gameoverimage->w;
    int gh = gameoverimage->h;
    tigrBlitAlpha(screen, gameoverimage, (screen->w - gw)/2, (screen->h - gh)/2, 0, 0, gw, gh, 1.0f);

    // blit the scroll below
    Tigr *scrollim = tigrLoadImage("res/scroll.png");
    int sw = scrollim->w;
    int sh = scrollim->h;
    int sx = (screen->w - sw)/2;
    int sy = gh + (screen->h - sh)/2;
    tigrBlitAlpha(screen, scrollim, sx, sy, 0, 0, sw, sh, 1.0f);

    // Score
    char buf[50] = { 0 };
    snprintf(buf, 49, "Your Score: %d", g->p.score);
    int tw = tigrTextWidth(g->h2font, buf);
    int tx = (screen->w-tw)/2;
    int ty = sy + sh/2;
    tigrPrint(screen, g->h2font, tx, ty, tigrRGB(255, 255, 255), "Your Score: %d", g->p.score);
}

void game_debug_dump(game_t *g) {
    char buf[50] = { 0 };
    int tw = 0;
    int th = 0;
    const int gap = 2;
    
    // position and velocity
    snprintf(buf, 49, "X: %0.1f, Y: %0.1f, VX: %0.1f, VY: %0.1f\n", 
        g->p.xpos, g->p.ypos, g->p.xvel, g->p.yvel);
    tw = tigrTextWidth(g->textfont, buf);
    tigrPrint(g->screen, g->textfont, g->screen->w - tw, th + gap, BAT_WHITE, buf);
    th += tigrTextHeight(g->textfont, buf);
    memset(buf, 0, sizeof(char));

    // player current animation
    snprintf(buf, 49, "Current Player Animation: %s\n", g->p.curr_anim->name);
    tw = tigrTextWidth(g->textfont, buf);
    tigrPrint(g->screen, g->textfont, g->screen->w - tw, th + gap, BAT_WHITE, buf);
    th += tigrTextHeight(g->textfont, buf);
    memset(buf, 0, sizeof(char));

    // player jump stats
    snprintf(buf, 49, "Player is Jumping: %d\n", g->p.jumping);
    tw = tigrTextWidth(g->textfont, buf);
    tigrPrint(g->screen, g->textfont, g->screen->w - tw, th + gap, BAT_WHITE, buf);
    th += tigrTextHeight(g->textfont, buf);
    memset(buf, 0, sizeof(char));

    // number of active batarangs
    int count = 0;
    for(int i = 0; i < MAX_BATRS; ++i) {
        if(g->batrs[i].active)count++;
    }
    snprintf(buf, 49, "Active Batarangs: %d\n", count);
    tw = tigrTextWidth(g->textfont, buf);
    tigrPrint(g->screen, g->textfont, g->screen->w - tw, th + gap, BAT_WHITE, buf);
    th += tigrTextHeight(g->textfont, buf);
    memset(buf, 0, sizeof(char));
}

void game_start_wait(Tigr* s, game_t *g) {
    char *p2s = "PRESS <SPACE> TO START THE GAME";
    char *controls[5] = {
        "    CONTROLS   ",
        "---------------",
        "<A>/<D> to move",
        "<SPACE> to jump",
        "<CLICK> to throw"
    };
    
    int tw, th, tx, ty = s->h/2 - 100;
    int spacing = 15;
    TigrFont *font = g->h2font;

    tw = tigrTextWidth(g->h1font, G_TITLE);
    th = tigrTextHeight(g->h1font, G_TITLE);
    tx = s->w/2 - tw/2;
    ty = ty + th/2;
    tigrPrint(s, g->h1font, tx, ty, BAT_WHITE, G_TITLE);
    ty += th/2 + spacing;
    tigrPrint(s, g->h1font, tx, ty, BAT_WHITE, " ");

    for(int i = 0; i < 5; i++) {
        tw = tigrTextWidth(font, controls[i]);
        th = tigrTextHeight(font, controls[i]);
        tx = s->w/2 - tw/2;
        ty += th/2 + spacing;
        tigrPrint(s, font, tx, ty, BAT_WHITE, controls[i]);
    }

    tw = tigrTextWidth(font, p2s);
    th = tigrTextHeight(font, p2s);
    tx = s->w/2 - tw/2;
    ty += th/2 + 2*spacing;
    tigrPrint(s, font, tx, ty, BAT_WHITE, p2s);

    int start = 0;

    while(!g->game_wclosed && !start) {
        g->game_wclosed = tigrClosed(s);
        if(tigrKeyDown(s, TK_SPACE)) {
            start = 1;
        }
        tigrUpdate(s);
    }
}

void game_intro_loop(Tigr* s, game_t* g) {
    anim_t intro_anim;
    anim_load_sprites_file(&intro_anim, INTRO_ANIM, "INTRO", 25, PLAYER, NONE, 0.1f, 1);
    
    tigrClear(s, BAT_WHITE);
    int w = intro_anim.frames[0]->w;
    int h = intro_anim.frames[0]->h;
    int x = s->w/2 - w/2;
    int y = s->h - h;
    
    ta_sound_play(&intro, 0);
    
    while(!g->game_wclosed && ta_sound_is_playing(&intro) && !tigrKeyDown(s, TK_ESCAPE)) {
        g->game_wclosed = tigrClosed(s);
        float dt = tigrTime();

        anim_advance_frame(&intro_anim, dt);
        tigrClear(s, BAT_WHITE);
        tigrBlitAlpha(s, 
                      intro_anim.frames[intro_anim.currframe], 
                      x, y, 0, 0, w, h, 
                      1.0f);
        tigrUpdate(s);
    }

    ta_sound_stop(&intro);
}

void game_main_loop(Tigr* s, Tigr* c, game_t* g) {
    // start playing the theme
    ta_sound_play(&theme, 1);
    while(!g->game_wclosed && !g->game_over) {
        g->game_wclosed = tigrClosed(s);

        float dt = tigrTime();

        // update game state and sprites
        game_update(g, dt);
        
        // update canvas
        game_draw(s, c, g);

        // put some debug info to the screen in dev mode
        #ifdef DEBUG
        game_debug_dump(g);
        #endif
        
        // update screen
        tigrUpdate(s);
    }
}

void game_over_loop(Tigr* s, Tigr* c, game_t *g) {
    while(!g->game_wclosed) {
        g->game_wclosed = tigrClosed(s);

        float dt = tigrTime();

        game_update(g, dt);

        game_over_draw(s, c, g);

        tigrUpdate(s);
    }
}

void game_free(game_t *g) {
    anim_free(&p_idle);
    anim_free(&p_runr);
    anim_free(&p_runl);
    anim_free(&p_jumpr);
    anim_free(&p_jumpl);
    anim_free(&batarang_rotate[0]);
    anim_free(&dragon_flyl[0]);
    free(g);
}