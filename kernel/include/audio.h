#pragma once
#include "kernel.h"

/*
 * Audio subsystem (Phase 2)
 * - Mixer and PCM API
 * - HDA/AC97 driver stubs
 */

typedef struct audio_stream audio_stream_t;

typedef struct {
    u32 sample_rate; /* Hz */
    u16 channels;    /* 1 mono, 2 stereo */
    u16 bits;        /* 16/24/32 */
} pcm_params_t;

void audio_init(void);

/* PCM */
audio_stream_t* audio_pcm_open(const pcm_params_t* p);
int audio_pcm_write(audio_stream_t* s, const void* frames, u32 frame_count);
int audio_pcm_close(audio_stream_t* s);