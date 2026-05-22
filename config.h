#ifndef _GAME_CONFIG_H_
#define _GAME_CONFIG_H_

/* Game Window mode */
/*
    Possible options
    - TIGR_FIXED
    - TIGR_FULLSCREEN
    - TIGR_2X
    - TIGR_3X
    - TIGR_4X
    - TIGR_AUTO
*/
#define GAME_WIN_MODE  TIGR_FIXED

/* Game Res */
#define G_W            (1280)
#define G_H            (720)
#define GAME_GROUND_Y  (674)

/* Game Sprites */
#define BACKDROP       "res/background.png"
#define BATARANG_ROT   "res/sprites/batarang_rotate/frame_%03d.png"
#define DRAGON_FLYL    "res/sprites/dragon_flyl/frame_%03d.png"
#define DRAGON_FLYR    "res/sprites/dragon_flyr/frame_%03d.png"
#define DRAGON_DEATHL  "res/sprites/dragon_deathl/frame_%03d.png"
#define DRAGON_DEATHR  "res/sprites/dragon_deathr/frame_%03d.png"
#define FIREBALL       "res/sprites/fireball/frame_%03d.png"
#define SPIKY          "res/sprites/spiky/frame_%03d.png"
#define SPIKY_DEATH    "res/sprites/spiky_death/frame_%03d.png"
#define PLAYER_IDLE    "res/sprites/idle/frame_%03d.png"
#define PLAYER_RUNR    "res/sprites/runright/frame_%03d.png"
#define PLAYER_RUNL    "res/sprites/runleft/frame_%03d.png"
#define PLAYER_JUMPR   "res/sprites/jumpright/frame_%03d.png"
#define PLAYER_JUMPL   "res/sprites/jumpleft/frame_%03d.png"
#define PLAYER_DEATH   "res/sprites/playerdeath/frame_%03d.png"

/* Game Fonts */
#define H1_FONT        "res/fonts/go3v2_30.png"
#define H2_FONT        "res/fonts/roboto.png"
#define TEXT_FONT      "res/fonts/profont17.png"

/* Game Config */
#define MAX_BATRS                       (3)
#define MAX_DRAGONS                     (2)
#define MAX_FIREBALLS_PER_DRAGON        (5)
#define MAX_FIREBALLS                   (MAX_DRAGONS * MAX_FIREBALLS_PER_DRAGON)
#define MAX_SPIKY                       (2)

/* Game Title */
#define G_TITLE "BATGIRL IN CHINA"

#endif