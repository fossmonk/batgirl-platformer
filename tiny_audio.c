#define _CRT_SECURE_NO_WARNINGS
#include "tiny_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    int16_t* pData;
    uint32_t sampleCount; 
    int referenceCount;
} ta_pcm_buffer;

struct ta_sound_internal {
    ta_pcm_buffer* pBuffer;
    uint32_t playbackCursor;
    int isPlaying;
    int isLooping;
};

#define TA_MAX_SOUNDS 128
#define TA_SAMPLE_RATE 44100
#define TA_CHANNELS 2
#define TA_BUFFER_SAMPLES 2048 // ~46ms chunks at 44.1kHz

static ta_sound_internal* g_active_sounds[TA_MAX_SOUNDS];
static int g_initialized = 0;

/* --- Win32 Backend Driver --- */
#if defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>

#if defined(_MSC_VER)
#pragma comment(lib, "winmm.lib")
#endif

static HWAVEOUT g_hWaveOut = NULL;
static WAVEHDR g_waveHeaders[2];
static int16_t* g_mixBuffer = NULL;
static HANDLE g_audioThread = NULL;
static HANDLE g_audioEvent = NULL;
static int g_runAudioThread = 0;

static void ta_mix_audio(int16_t* pOutput, uint32_t frameCount) {
    memset(pOutput, 0, frameCount * TA_CHANNELS * sizeof(int16_t));

    for (int i = 0; i < TA_MAX_SOUNDS; ++i) {
        ta_sound_internal* s = g_active_sounds[i];
        if (!s || !s->isPlaying || !s->pBuffer) continue;

        ta_pcm_buffer* buf = s->pBuffer;
        uint32_t samplesToMix = frameCount * TA_CHANNELS;

        for (uint32_t f = 0; f < samplesToMix; ++f) {
            if (s->playbackCursor >= buf->sampleCount) {
                if (s->isLooping) {
                    s->playbackCursor = 0;
                } else {
                    s->isPlaying = 0;
                    break;
                }
            }

            int32_t mixed = pOutput[f] + buf->pData[s->playbackCursor++];
            if (mixed > 32767)  mixed = 32767;
            if (mixed < -32768) mixed = -32768;
            pOutput[f] = (int16_t)mixed;
        }
    }
}

static DWORD WINAPI ta_win32_audio_thread(LPVOID lpParam) {
    (void)lpParam;
    int currentBuffer = 0;
    
    while (g_runAudioThread) {
        // Wait for the OS backend to signal that a wave header buffer has completed playback
        WaitForSingleObject(g_audioEvent, INFINITE);
        if (!g_runAudioThread) break;

        WAVEHDR* hdr = &g_waveHeaders[currentBuffer];
        if (hdr->dwFlags & WHDR_DONE) {
            // Refill the buffer
            ta_mix_audio((int16_t*)hdr->lpData, TA_BUFFER_SAMPLES);
            waveOutWrite(g_hWaveOut, hdr, sizeof(WAVEHDR));
            currentBuffer = (currentBuffer + 1) % 2;
        }
    }
    return 0;
}
#endif

/* --- Core Engine APIs --- */

int ta_sound_init(void) {
    if (g_initialized) return TA_SUCCESS;
    memset(g_active_sounds, 0, sizeof(g_active_sounds));

#if defined(_WIN32)
    // Create synchronization event
    g_audioEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!g_audioEvent) return TA_ERROR;

    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = TA_CHANNELS;
    wfx.nSamplesPerSec = TA_SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    // Initialize with CALLBACK_EVENT flag so it responds to our signaling handle
    if (waveOutOpen(&g_hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)g_audioEvent, 0, CALLBACK_EVENT) != MMSYSERR_NOERROR) {
        CloseHandle(g_audioEvent);
        return TA_ERROR;
    }

    g_mixBuffer = (int16_t*)calloc(TA_BUFFER_SAMPLES * TA_CHANNELS * 2, sizeof(int16_t));
    
    // Prepare headers and prime the queue with silence so waveOut start immediately
    for (int i = 0; i < 2; ++i) {
        g_waveHeaders[i].lpData = (LPSTR)(g_mixBuffer + (i * TA_BUFFER_SAMPLES * TA_CHANNELS));
        g_waveHeaders[i].dwBufferLength = TA_BUFFER_SAMPLES * TA_CHANNELS * sizeof(int16_t);
        g_waveHeaders[i].dwFlags = 0;
        waveOutPrepareHeader(g_hWaveOut, &g_waveHeaders[i], sizeof(WAVEHDR));
        waveOutWrite(g_hWaveOut, &g_waveHeaders[i], sizeof(WAVEHDR));
    }

    g_runAudioThread = 1;
    g_audioThread = CreateThread(NULL, 0, ta_win32_audio_thread, NULL, 0, NULL);
#endif

    g_initialized = 1;
    return TA_SUCCESS;
}

static ta_pcm_buffer* ta_load_wav(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    char chunkId[4];
    uint32_t chunkSize;
    char format[4];

    if (fread(chunkId, 1, 4, f) != 4 || memcmp(chunkId, "RIFF", 4) != 0) { fclose(f); return NULL; }
    if (fread(&chunkSize, 4, 1, f) != 1) { fclose(f); return NULL; }
    if (fread(format, 1, 4, f) != 4 || memcmp(format, "WAVE", 4) != 0) { fclose(f); return NULL; }

    uint16_t numChannels = 0, bitsPerSample = 0;
    uint32_t sampleRate = 0;
    uint8_t* rawSourceData = NULL;
    uint32_t dataChunkSize = 0;

    while (fread(chunkId, 1, 4, f) == 4) {
        if (fread(&chunkSize, 4, 1, f) != 1) break;

        if (memcmp(chunkId, "fmt ", 4) == 0) {
            uint16_t audioFormat;
            if (fread(&audioFormat, 2, 1, f) != 1) break;
            if (fread(&numChannels, 2, 1, f) != 1) break;
            if (fread(&sampleRate, 4, 1, f) != 1) break;
            fseek(f, 6, SEEK_CUR); 
            if (fread(&bitsPerSample, 2, 1, f) != 1) break;
            if (chunkSize > 16) fseek(f, chunkSize - 16, SEEK_CUR);
        } 
        else if (memcmp(chunkId, "data", 4) == 0) {
            dataChunkSize = chunkSize;
            rawSourceData = (uint8_t*)malloc(dataChunkSize);
            if (fread(rawSourceData, 1, dataChunkSize, f) != dataChunkSize) {
                free(rawSourceData);
                rawSourceData = NULL;
            }
            break;
        } 
        else {
            fseek(f, chunkSize, SEEK_CUR);
        }
    }
    fclose(f);

    if (!rawSourceData || numChannels == 0 || sampleRate == 0) {
        if (rawSourceData) free(rawSourceData);
        return NULL;
    }

    uint32_t bytesPerSample = bitsPerSample / 8;
    uint32_t srcTotalFrames = dataChunkSize / (numChannels * bytesPerSample);
    uint32_t destTotalFrames = (uint32_t)((uint64_t)srcTotalFrames * TA_SAMPLE_RATE / sampleRate);
    uint32_t destSampleCount = destTotalFrames * TA_CHANNELS;
    
    int16_t* normalizedData = (int16_t*)malloc(destSampleCount * sizeof(int16_t));
    
    for (uint32_t i = 0; i < destTotalFrames; ++i) {
        uint32_t srcFrameIdx = (uint32_t)((uint64_t)i * sampleRate / TA_SAMPLE_RATE);
        if (srcFrameIdx >= srcTotalFrames) srcFrameIdx = srcTotalFrames - 1;

        int32_t sampleL = 0;
        int32_t sampleR = 0;

        uint32_t byteOffsetL = srcFrameIdx * numChannels * bytesPerSample;
        if (bitsPerSample == 8) {
            sampleL = ((int32_t)rawSourceData[byteOffsetL] - 128) << 8;
        } else if (bitsPerSample == 16) {
            int16_t value;
            memcpy(&value, &rawSourceData[byteOffsetL], 2);
            sampleL = value;
        }

        if (numChannels >= 2) {
            uint32_t byteOffsetR = byteOffsetL + bytesPerSample;
            if (bitsPerSample == 8) {
                sampleR = ((int32_t)rawSourceData[byteOffsetR] - 128) << 8;
            } else if (bitsPerSample == 16) {
                int16_t value;
                memcpy(&value, &rawSourceData[byteOffsetR], 2);
                sampleR = value;
            }
        } else {
            sampleR = sampleL;
        }

        normalizedData[i * 2 + 0] = (int16_t)sampleL;
        normalizedData[i * 2 + 1] = (int16_t)sampleR;
    }

    free(rawSourceData);

    ta_pcm_buffer* pcm = (ta_pcm_buffer*)malloc(sizeof(ta_pcm_buffer));
    pcm->pData = normalizedData;
    pcm->sampleCount = destSampleCount;
    pcm->referenceCount = 1;

    return pcm;
}

void ta_sound_load(ta_sound *s, const char *filename) {
    ta_pcm_buffer* buf = ta_load_wav(filename);
    if (!buf) {
        s->_internal = NULL;
        return;
    }

    s->_internal = (ta_sound_internal*)malloc(sizeof(ta_sound_internal));
    s->_internal->pBuffer = buf;
    s->_internal->playbackCursor = 0;
    s->_internal->isPlaying = 0;
    s->_internal->isLooping = 0;

    for (int i = 0; i < TA_MAX_SOUNDS; ++i) {
        if (!g_active_sounds[i]) {
            g_active_sounds[i] = s->_internal;
            break;
        }
    }
}

void ta_sound_load_copies(ta_sound *s, const char *filename, int num_copies) {
    if (num_copies <= 0) return;

    ta_sound_load(&s[0], filename);
    if (!s[0]._internal) return;

    ta_pcm_buffer* shared_buf = s[0]._internal->pBuffer;

    for (int i = 1; i < num_copies; ++i) {
        s[i]._internal = (ta_sound_internal*)malloc(sizeof(ta_sound_internal));
        s[i]._internal->pBuffer = shared_buf;
        shared_buf->referenceCount++;
        s[i]._internal->playbackCursor = 0;
        s[i]._internal->isPlaying = 0;
        s[i]._internal->isLooping = 0;

        for (int j = 0; j < TA_MAX_SOUNDS; ++j) {
            if (!g_active_sounds[j]) {
                g_active_sounds[j] = s[i]._internal;
                break;
            }
        }
    }
}

void ta_sound_play(ta_sound *s, int isloop) {
    if (!s || !s->_internal) return;
    s->_internal->playbackCursor = 0;
    s->_internal->isLooping = isloop;
    s->_internal->isPlaying = 1;
}

void ta_sound_pause(ta_sound *s) {
    if (!s || !s->_internal) return;
    s->_internal->isPlaying = 0;
}

void ta_sound_stop(ta_sound *s) {
    if (!s || !s->_internal) return;
    s->_internal->isPlaying = 0;
    s->_internal->playbackCursor = 0; // Reset back to the start
}

int ta_sound_is_playing(const ta_sound *s) {
    return (s && s->_internal) ? s->_internal->isPlaying : 0;
}

int ta_sound_is_looping(const ta_sound *s) {
    return (s && s->_internal) ? s->_internal->isLooping : 0;
}

void ta_sound_free(ta_sound *s) {
    if (!s || !s->_internal) return;

    for (int i = 0; i < TA_MAX_SOUNDS; ++i) {
        if (g_active_sounds[i] == s->_internal) {
            g_active_sounds[i] = NULL;
            break;
        }
    }

    ta_sound_internal* si = s->_internal;
    if (si->pBuffer) {
        si->pBuffer->referenceCount--;
        if (si->pBuffer->referenceCount <= 0) {
            free(si->pBuffer->pData);
            free(si->pBuffer);
        }
    }
    free(si);
    s->_internal = NULL;
}

void ta_sound_uninit(void) {
#if defined(_WIN32)
    g_runAudioThread = 0;
    if (g_audioEvent) {
        SetEvent(g_audioEvent); // Wake thread up to break loop
    }
    if (g_audioThread) {
        WaitForSingleObject(g_audioThread, INFINITE);
        CloseHandle(g_audioThread);
    }
    if (g_hWaveOut) {
        waveOutReset(g_hWaveOut);
        for(int i = 0; i < 2; ++i) waveOutUnprepareHeader(g_hWaveOut, &g_waveHeaders[i], sizeof(WAVEHDR));
        waveOutClose(g_hWaveOut);
    }
    if (g_audioEvent) {
        CloseHandle(g_audioEvent);
    }
    free(g_mixBuffer);
#endif
    g_initialized = 0;
}