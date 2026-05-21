#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "colors.h"
#include "config.h"
#include "winmagic.h"
#include "game.h"
#include "tigr/tigr.h"

int main() {
    Tigr* screen = tigrWindow(G_W, G_H, G_TITLE, TIGR_FIXED);
    Tigr *canvas = tigrBitmap(screen->w, screen->h);

    if(screen == NULL || screen == NULL) {
        printf("[ERROR]Couldn't allocate tigrWindow..exiting\n");
        return 1;
    }

    // update icons etc - Windows specific
    setup_windows_magic(screen);

    // initialize game
    g_obj *g = game_init(screen, canvas);

    tigrSetPostFX(screen, 1, 1, 1, 1.1f);
    
    while(!tigrClosed(screen)) {
        float dt = tigrTime();
        game_update(g, dt);
        tigrBlit(screen, canvas, 0, 0, 0, 0, canvas->w, canvas->h);
        tigrBlitAlpha(screen, 
                      g->curr_sprite, 
                      (int)g->p.xpos, 
                      (int)g->p.ypos, 
                      0, 
                      0, 
                      g->curr_sprite->w,
                      g->curr_sprite->h, 
                      1.0f);

        // put some debug info to the screen in dev mode
        #ifndef PACKAGE
        game_debug_dump(g);
        #endif
        
        // update screen
        tigrUpdate(screen);
    }

    free_game(g);
    tigrFree(canvas);
    tigrFree(screen);
    
    return 0;
}