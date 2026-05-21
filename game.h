#include "tigr/tigr.h"
#include "anim.h"

#define MAX_BATRS (10)

/* Player object */
typedef struct {
    float xpos;
    float ypos;
    float xvel;
    float yvel;
    int jumping;
    int h_dir;
    Tigr *curr_sprite;
} player_t;

typedef struct {
    int active;
    float xpos;
    float ypos;
    float xvel;
    float yvel;
    anim_t* anim;
    Tigr *curr_sprite;
} weapon_t;

/* Game Object */
typedef struct {
    Tigr *canvas;
    Tigr *screen;
    TigrFont *titlefont;
    TigrFont *regfont;

    player_t p;
    weapon_t batrs[MAX_BATRS];
    
} g_obj;

g_obj* game_init(Tigr* screen, Tigr* canvas);
void game_update(g_obj *g, float dt);
void game_debug_dump(g_obj *g);
void game_draw(Tigr* screen, Tigr* canvas, g_obj *g);
void game_free(g_obj *g);