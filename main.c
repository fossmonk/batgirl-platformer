#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "colors.h"
#include "config.h"
#include "winmagic.h"
#include "game.h"
#include "tigr/tigr.h"

int main() {
    srand(time(NULL));
    Tigr* screen = tigrWindow(G_W, G_H, G_TITLE, TIGR_FIXED);
    Tigr *canvas = tigrBitmap(screen->w, screen->h);

    if(screen == NULL || screen == NULL) {
        printf("[ERROR]Couldn't allocate tigrWindow..exiting\n");
        return 1;
    }

    // update icons etc - Windows specific
    setup_windows_magic(screen);

    // initialize game
    game_t *g = game_init(screen, canvas);

    tigrSetPostFX(screen, 1, 1, 1, 1.1f);
    
    while(!tigrClosed(screen)) {
        float dt = tigrTime();

        // update game state and sprites
        game_update(g, dt);
        
        // if game is not over, update canvas
        if(!g->game_over) {
            game_draw(screen, canvas, g);
        } else {
            game_over_draw(screen, canvas, g);
        }

        // put some debug info to the screen in dev mode
        #ifndef PACKAGE
        game_debug_dump(g);
        #endif
        
        // update screen
        tigrUpdate(screen);
    }

    game_free(g);
    tigrFree(canvas);
    tigrFree(screen);
    
    return 0;
}