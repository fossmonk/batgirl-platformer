#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tigr.h"
#include "colors.h"

#ifdef __WIN32
#include <windows.h>
#endif

#define G_W         (1280)
#define G_H         (720)

#define TFONT_W     (16)
#define TFONT_H     (29)

#define TITLE_X     (G_W/2 - 10*TFONT_W)
#define TITLE_Y     (0)
#define S_FRAME_CNT (7)

#define _min(a, b)   ((a) < (b) ? (a) : (b))
#define _max(a, b)   ((a) > (b) ? (a) : (b))

#define DIR_LEFT    (1)
#define DIR_RIGHT   (2)

Tigr *gspr;

Tigr **grun_r;
Tigr **grun_l;
Tigr **gjump_r;
Tigr **gjump_l;
Tigr **gidle;
Tigr **gspr_type;

float spr_x = 0;
float spr_y = 0;
float spr_sx = 0;
float spr_sy = 0;

int in_jump = 0;
int jump_frame = 0;
int spr_dir = DIR_RIGHT;

int anim_idx = 0;

const char *game_title = "Adventures of Batgirl";

Tigr *gcanvas;
Tigr *gscreen;
TigrFont *titlefont;

void setup_windows_magic() {
    #ifdef __WIN32
    HWND hwnd = (HWND)gscreen->handle; 

    if (hwnd != NULL) {
        HICON hIcon = LoadIcon(GetModuleHandle(NULL), "MAINICON");
        
        if (hIcon != NULL) {
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon); // Title bar (16x16)
            SendMessage(hwnd, WM_SETICON, ICON_BIG,   (LPARAM)hIcon); // Taskbar thumb (32x32)
        }
    }
    #endif
}

void init_sprites() {
    // allocate sprite pointer buffers
    grun_r  = malloc(sizeof(*grun_r) * S_FRAME_CNT);
    grun_l  = malloc(sizeof(*grun_l) * S_FRAME_CNT);
    gidle   = malloc(sizeof(*gidle) * S_FRAME_CNT);
    gjump_r = malloc(sizeof(*gjump_r) * S_FRAME_CNT);
    gjump_l = malloc(sizeof(*gjump_l) * S_FRAME_CNT);

    char buf[100] = { 0 };
    for(int i = 0; i < S_FRAME_CNT; i++) {
        snprintf(buf, 99, "sprites/runright/frame_%03d.png", i+1);
        grun_r[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }
    for(int i = 0; i < S_FRAME_CNT; i++) {
        snprintf(buf, 99, "sprites/runleft/frame_%03d.png", i+1);
        grun_l[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }
    for(int i = 0; i < S_FRAME_CNT; i++) {
        snprintf(buf, 99, "sprites/idle/frame_%03d.png", i+1);
        gidle[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }
    for(int i = 0; i < S_FRAME_CNT; i++) {
        snprintf(buf, 99, "sprites/jumpright/frame_%03d.png", i+1);
        gjump_r[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }
    for(int i = 0; i < S_FRAME_CNT; i++) {
        snprintf(buf, 99, "sprites/jumpleft/frame_%03d.png", i+1);
        gjump_l[i] = tigrLoadImage(buf);
        memset(buf, 0, sizeof(char));
    }

    // initialize sprite parameters
    gspr = gidle[0];
    spr_x = 100;
    spr_y = gscreen->h - gspr->h;
    spr_sx = 0;
    spr_sy = 0;
}

void free_sprites() {
    free(grun_r);
    free(grun_l);
    free(gidle);
}

void load_fonts() {
    Tigr *tfi = tigrLoadImage("fonts/profont29.png");
    titlefont = tigrLoadFont(tfi, TCP_ASCII);
}

void init_backdrop() {
    tigrClear(gcanvas, BAT_BLUE1);
    Tigr *bg_image = tigrLoadImage("background.png");
    if(bg_image) {
        tigrBlit(gcanvas, bg_image, 0,0,0,0, bg_image->w, bg_image->h);
    }
    #if defined(PACKAGE)
    tigrPrint(gcanvas, titlefont, TITLE_X, TITLE_Y, BAT_WHITE, game_title);
    #endif
}

Tigr* get_next_sframe(float dt) {
    static float animation_timer = 0.0f;
    const float frame_duration = 0.07f; 

    animation_timer += dt;

    if (animation_timer >= frame_duration) {
        // in case of jump animation, run it only once.
        if(gspr_type == gjump_l || gspr_type == gjump_r) {
            anim_idx = _min(anim_idx + 1, S_FRAME_CNT-1);
        } else {
            anim_idx = (anim_idx + 1) % S_FRAME_CNT;
        }
        animation_timer -= frame_duration; 
    }

    return gspr_type[anim_idx];
}

void update_game(float dt) {
    if(dt > 0.1f)dt = 0.1f;

    // update velocity with movement
    if (tigrKeyHeld(gscreen, TK_RIGHT) || tigrKeyHeld(gscreen, 'D')) {
        spr_dir = DIR_RIGHT;
        if(!in_jump) gspr_type = grun_r;
        gspr = get_next_sframe(dt);
        spr_sx += 40;
    }
    if (tigrKeyHeld(gscreen, TK_LEFT) || tigrKeyHeld(gscreen, 'A')) {
        spr_dir = DIR_LEFT;
        if(!in_jump) gspr_type = grun_l;
        gspr = get_next_sframe(dt);
        spr_sx -= 40;
    }
    if (tigrKeyDown(gscreen, TK_SPACE)) {
        if(!in_jump) {
            in_jump = 1;
            spr_sy -= 700;
            if(spr_dir == DIR_LEFT) {
                gspr_type = gjump_l;
            } else {
                gspr_type = gjump_r;
            }
            anim_idx = 0;
        }
    }

    // decay velocities after updation
    spr_sx *= expf(-20*dt);
    spr_sy *= expf(-2*dt);

    // update vertical velocity with gravity
    spr_sy += 400*dt;

    // update x and y position
    spr_x += spr_sx * dt;
    spr_y += spr_sy * dt;

    // limit x position
    spr_x = _max(spr_x, 0);
    spr_x = _min(spr_x, G_W - gspr->w);
    // limit y position
    spr_y = _max(spr_y, 0);
    spr_y = _min(spr_y, G_H - gspr->h);

    if(in_jump && (gspr_type == gjump_l || gspr_type == gjump_r)) {
        gspr = get_next_sframe(dt);
    }

    if((int)spr_y == (G_H - gspr->h)) {
        in_jump = 0;
    }

    // if sprite x velocity is 0, 
    // if sprite y pos is ground, switch to idle
    if(((int)spr_sx == 0) && (int)spr_y == (G_H - gspr->h)) {
        gspr_type = gidle;
        gspr = get_next_sframe(dt);
    }

}

int main() {
    gscreen = tigrWindow(G_W, G_H, game_title, TIGR_AUTO);
    gcanvas = tigrBitmap(gscreen->w, gscreen->h);

    if(gscreen == NULL || gcanvas == NULL) {
        printf("[ERROR]Couldn't allocate tigrWindow..exiting\n");
        return 1;
    }

    // update icons etc - Win specific
    setup_windows_magic();

    load_fonts();
    init_sprites();
    init_backdrop();

    tigrSetPostFX(gscreen, 1, 1, 1, 1.1f);
    
    while(!tigrClosed(gscreen)) {
        float dt = tigrTime();
        update_game(dt);
        tigrBlit(gscreen, gcanvas, 0, 0, 0, 0, gcanvas->w, gcanvas->h);
        tigrBlitAlpha(gscreen, gspr, (int)spr_x, (int)spr_y, 0, 0, gspr->w, gspr->h, 1.0f);
        #if !defined(PACKAGE)
        tigrPrint(gscreen, titlefont, 1000, 0, tigrRGB(255,0,0), "X: %f, Y: %f, VX: %f, VY: %f\n", spr_x,spr_y,spr_sx,spr_sy);
        #endif
        tigrUpdate(gscreen);
    }

    free_sprites();
    tigrFree(gcanvas);
    tigrFree(gscreen);
    
    return 0;
}