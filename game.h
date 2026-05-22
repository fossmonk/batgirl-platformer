#include "tigr/tigr.h"
#include "config.h"
#include "anim.h"

// Generic object
typedef struct {
    float xpos;
    float ypos;
    float xvel;
    float yvel;
    
    anim_t *curr_anim;
    Tigr *curr_sprite;
    
    int active;
} obj_t;

// Player object
typedef struct {
    float xpos;
    float ypos;
    float xvel;
    float yvel;
    
    anim_t *curr_anim;
    Tigr *curr_sprite;

    int health;
    int dying;
    int score;
    
    int jumping;
    int h_dir;

} player_t;

// NPC object
typedef struct {
    float xpos;
    float ypos;
    float xvel;
    float yvel;
    
    anim_t *curr_anim;
    Tigr *curr_sprite;
    
    int active;
    int health;
    int dying;
} npc_t;


/* Game Object */
typedef struct {
    Tigr *canvas;
    Tigr *screen;
    TigrFont *h1font;
    TigrFont *h2font;
    TigrFont *textfont;

    player_t p;
    obj_t batrs[MAX_BATRS];
    npc_t dragons[MAX_DRAGONS];
    obj_t fireballs[MAX_FIREBALLS];
    npc_t spiky[MAX_SPIKY];

    // handle mouse
    int mouse_prev_buttons;
    int mouse_buttons;
    int mousex;
    int mousey;

    int game_over;
    
} game_t;

void game_start_wait(Tigr* s, game_t *g);
game_t* game_init(Tigr* screen, Tigr* canvas);
void game_update(game_t *g, float dt);
void game_debug_dump(game_t *g);
void game_draw(Tigr* screen, Tigr* canvas, game_t *g);
void game_over_draw(Tigr* screen, Tigr* canvas, game_t *g);
void game_free(game_t *g);