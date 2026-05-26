#ifndef _SOUND_H_
#define _SOUND_H_

#include "miniaudio.h"

typedef ma_sound sound_t;

void sound_play(ma_sound *s, int isloop);
void sound_pause(ma_sound *s);
void sound_load(ma_sound *s, char *filename);
void sound_load_copies(ma_sound *s, char *filename, int num_copies);
int sound_init();

#endif