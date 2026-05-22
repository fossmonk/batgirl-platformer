#include "tigr/tigr.h"

/* Game Res */
#ifndef _1080P
#define G_W            (1280)
#define G_H            (720)
#define GAME_GROUND_Y  (674)
#define BACKDROP       "res/background720.png"
#define BATARANG_ROT   "res/sprites/720/batarang_rotate/frame_%03d.png"
#define DRAGON_FLYL    "res/sprites/720/dragon_flyl/frame_%03d.png"
#define DRAGON_FLYR    "res/sprites/720/dragon_flyr/frame_%03d.png"
#define DRAGON_DEATHL  "res/sprites/720/dragon_deathl/frame_%03d.png"
#define DRAGON_DEATHR  "res/sprites/720/dragon_deathr/frame_%03d.png"
#define FIREBALL       "res/sprites/720/fireball/frame_%03d.png"
#else
#define G_W            (1920)
#define G_H            (1080)
#define GAME_GROUND_Y  (990)
#define BACKDROP       "res/background.png"
#define BATARANG_ROT   "res/sprites/1080/batarang_rotate/frame_%03d.png"
#define DRAGON_FLYL    "res/sprites/1080dragon_flyl/frame_%03d.png"
#define DRAGON_FLYR    "res/sprites/1080dragon_flyr/frame_%03d.png"
#define DRAGON_DEATHL  "res/sprites/1080dragon_deathl/frame_%03d.png"
#define DRAGON_DEATHR  "res/sprites/1080dragon_deathr/frame_%03d.png"
#define FIREBALL       "res/sprites/1080/fireball/frame_%03d.png"
#endif
#define PLAYER_IDLE    "res/sprites/idle/frame_%03d.png"
#define PLAYER_RUNR    "res/sprites/runright/frame_%03d.png"
#define PLAYER_RUNL    "res/sprites/runleft/frame_%03d.png"
#define PLAYER_JUMPR   "res/sprites/jumpright/frame_%03d.png"
#define PLAYER_JUMPL   "res/sprites/jumpleft/frame_%03d.png"
#define PLAYER_DEATH   "res/sprites/playerdeath/frame_%03d.png"

/* Game Title */
#define G_TITLE "BATGIRL IN CHINA"
