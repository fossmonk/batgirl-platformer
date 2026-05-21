#include "tigr/tigr.h"

g_obj* game_init(Tigr* screen, Tigr* canvas);
void game_update(g_obj *g, float dt);
void game_debug_dump(g_obj *g);
void free_game(g_obj *g);