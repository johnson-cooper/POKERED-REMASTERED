#pragma once

#include "types.h"

typedef enum {
    AUDIO_MUSIC_NONE = 0,
    AUDIO_MUSIC_PALLET_TOWN,
    AUDIO_MUSIC_TITLE_SCREEN,
    AUDIO_MUSIC_TRAINER_BATTLE,
    AUDIO_MUSIC_DEFEATED_TRAINER,
    AUDIO_MUSIC_ROUTES_1,
    AUDIO_MUSIC_OAKS_LAB,
    AUDIO_MUSIC_INTRO_BATTLE,
    AUDIO_MUSIC_MEET_PROF_OAK,
} AudioMusicId;

typedef enum {
    AUDIO_SFX_SELECT = 0,
    AUDIO_SFX_CONFIRM,
    AUDIO_SFX_CANCEL,
    AUDIO_SFX_TEXT,
    AUDIO_SFX_DENIED,
    AUDIO_SFX_START,
    AUDIO_SFX_BATTLE_SELECT,
    AUDIO_SFX_BATTLE_CONFIRM,
    AUDIO_SFX_WARP_OUT,
    AUDIO_SFX_WARP_IN,
    AUDIO_SFX_PAUSE_OPEN,
    AUDIO_SFX_PAUSE_CLOSE,
    AUDIO_SFX_CRY_BULBASAUR,
    AUDIO_SFX_CRY_CHARMANDER,
    AUDIO_SFX_CRY_SQUIRTLE,
} AudioSfxId;

void audio_init(void);
void audio_update(void);
void audio_music_play(AudioMusicId id);
void audio_music_stop(void);
AudioMusicId audio_music_current(void);
void audio_sfx_play(AudioSfxId id);
void audio_sfx_set_battle_intro(bool8 active);
