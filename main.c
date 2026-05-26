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
    Tigr* screen = tigrWindow(G_W, G_H, G_TITLE, GAME_WIN_MODE);
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

    // Start screen loop, moves forward when SPACE is pressed
    game_start_wait(screen, g);

    // Intro loop, moves forward when intro sound has finished playing
    game_intro_loop(screen, g);
    
    // Game main loop, moves forward when game is over
    game_main_loop(screen, canvas, g);

    // Game over loop, moves forward when user closes window
    game_over_loop(screen, canvas, g);

    game_free(g);
    tigrFree(canvas);
    tigrFree(screen);
    
    return 0;
}