#include "tigr/tigr.h"
#include "anim.h"

#define MAX_BATRS   (5)
#define MAX_DRAGONS (2)

/* Player object */
typedef struct {
    float xpos;
    float ypos;
    float xvel;
    float yvel;

    int jumping;
    int h_dir;

    anim_t *curr_anim;
    Tigr *curr_sprite;

    int health;
} player_t;

typedef struct {
    int active;
    float xpos;
    float ypos;
    float xvel;
    float yvel;

    anim_t *curr_anim;
    Tigr *curr_sprite;
} weapon_t;

typedef struct {
    int active;
    float xpos;
    float ypos;
    float xvel;
    float yvel;

    anim_t *curr_anim;
    Tigr *curr_sprite;

    int health;
} npc_t;


/* Game Object */
typedef struct {
    Tigr *canvas;
    Tigr *screen;
    TigrFont *titlefont;
    TigrFont *regfont;

    player_t p;
    weapon_t batrs[MAX_BATRS];
    npc_t dragons[MAX_DRAGONS];

    // handle mouse
    int mouse_prev_buttons;
    int mouse_buttons;
    int mousex;
    int mousey;
    
} g_obj;

g_obj* game_init(Tigr* screen, Tigr* canvas);
void game_update(g_obj *g, float dt);
void game_debug_dump(g_obj *g);
void game_draw(Tigr* screen, Tigr* canvas, g_obj *g);
void game_free(g_obj *g);