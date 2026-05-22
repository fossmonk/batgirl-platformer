#include "tigr/tigr.h"
#include "anim.h"

#define MAX_BATRS                       (3)
#define MAX_DRAGONS                     (2)
#define MAX_FIREBALLS_PER_DRAGON        (5)
#define MAX_FIREBALLS                   (MAX_DRAGONS * MAX_FIREBALLS_PER_DRAGON)

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
    TigrFont *titlefont;
    TigrFont *regfont;

    player_t p;
    obj_t batrs[MAX_BATRS];
    npc_t dragons[MAX_DRAGONS];
    obj_t fireballs[MAX_FIREBALLS];

    // handle mouse
    int mouse_prev_buttons;
    int mouse_buttons;
    int mousex;
    int mousey;

    int game_over;
    
} game_t;

game_t* game_init(Tigr* screen, Tigr* canvas);
void game_update(game_t *g, float dt);
void game_debug_dump(game_t *g);
void game_draw(Tigr* screen, Tigr* canvas, game_t *g);
void game_over_draw(Tigr* screen, Tigr* canvas, game_t *g);
void game_free(game_t *g);