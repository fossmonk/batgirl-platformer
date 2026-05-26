#include <stdio.h>
#define MINIAUDIO_IMPLEMENTATION
#include "sound.h"

ma_engine engine;

void sound_play(ma_sound *s, int isloop) {
    ma_sound_start(s);
    ma_sound_set_looping(s, isloop);
}

void sound_pause(ma_sound *s) {
    ma_sound_stop(s);
}

void sound_load(ma_sound *s, char *filename) {
    ma_result res = ma_sound_init_from_file(&engine, filename, 0, NULL, NULL, s);
    if (res != MA_SUCCESS) {
        return;
    }
}

void sound_load_copies(ma_sound *s, char *filename, int num_copies) {
    ma_result res = ma_sound_init_from_file(&engine, filename, 0, NULL, NULL, &s[0]);
    if (res != MA_SUCCESS) {
        return;
    }
    for(int i = 1; i < num_copies; i++) {
        res = ma_sound_init_copy(&engine, &s[0], 0, NULL, &s[i]);
        if (res != MA_SUCCESS) {
            return;
        }
    }
}

int sound_init() {
    ma_result result = ma_engine_init(NULL, &engine);
    return (int)result;
}