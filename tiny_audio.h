#ifndef TINY_AUDIO_H
#define TINY_AUDIO_H

/*

Stripped down WAV audio player from miniaudio library

*/

#ifdef __cplusplus
extern "C" {
#endif

#define TA_SUCCESS 0
#define TA_ERROR   -1

/* Opaque data structures for sound instances */
typedef struct ta_sound_internal ta_sound_internal;

typedef struct {
    ta_sound_internal* _internal;
} ta_sound;

/* Initialize the audio engine/backend */
int ta_sound_init(void);

/* Load a sound from a WAV file */
void ta_sound_load(ta_sound *s, const char *filename);

/* Load a sound and create identical, independent playback copies from it */
void ta_sound_load_copies(ta_sound *s, const char *filename, int num_copies);

/* Play / Control looping */
void ta_sound_play(ta_sound *s, int isloop);

/* Pause playback (leaves cursor at current position) */
void ta_sound_pause(ta_sound *s);

/* Stop playback completely (resets cursor to the beginning) */
void ta_sound_stop(ta_sound *s);

/* State Checkers */
int ta_sound_is_playing(const ta_sound *s);
int ta_sound_is_looping(const ta_sound *s);

/* Cleanup resources */
void ta_sound_free(ta_sound *s);
void ta_sound_uninit(void);

#ifdef __cplusplus
}
#endif

#endif /* TINY_AUDIO_H */