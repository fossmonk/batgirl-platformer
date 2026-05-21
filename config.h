#include "tigr/tigr.h"

/* Game Title */
#define G_W         (1920)
#define G_H         (1080)

/* Frame count for animations. Assume same for all anims */
#define S_FRAME_CNT (7)

/* Game Title */
#define G_TITLE "Adventures of Batgirl"

/* Player object */
typedef struct {
    float xpos;
    float ypos;
    float xvel;
    float yvel;
    
    int jumping;
    int h_dir;
} player_t;

/* Game Object */
typedef struct {
    Tigr *canvas;
    Tigr *screen;
    TigrFont *titlefont;
    TigrFont *regfont;

    Tigr *curr_sprite;

    Tigr **anim_runr;
    Tigr **anim_runl;
    Tigr **anim_jumpr;
    Tigr **anim_jumpl;
    Tigr **anim_idle;
    Tigr **curr_anim;

    player_t p;

    int fcount;
    int fidx;
} g_obj;