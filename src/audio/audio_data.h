#pragma once

#include "types.h"

typedef enum {
    AUDIO_CMD_NOTE,
    AUDIO_CMD_REST,
    AUDIO_CMD_NOTE_TYPE,
    AUDIO_CMD_OCTAVE,
    AUDIO_CMD_DUTY,
    AUDIO_CMD_TEMPO,
    AUDIO_CMD_VIBRATO,
    AUDIO_CMD_LOOP,
    AUDIO_CMD_DRUM,   // noise hit: value=REG_SOUND4CNT_L, arg1=length-1, arg2=NR43
} AudioCommandType;

typedef struct {
    AudioCommandType type;
    u16 value;
    u8 arg1;
    u8 arg2;
} AudioCommand;

typedef struct {
    const AudioCommand *commands;
    u16 count;
    u16 loop_start;
} AudioChannelData;

typedef struct {
    AudioChannelData channels[4];
    const u16 *wave_data;   // 8 u16s for wave RAM, or NULL for triangle
} AudioTrackData;

extern const AudioTrackData g_audio_pallet_town;
extern const AudioTrackData g_audio_title_screen;
extern const AudioTrackData g_audio_trainer_battle;
extern const AudioTrackData g_audio_defeated_trainer;
extern const AudioTrackData g_audio_routes1;
extern const AudioTrackData g_audio_oakslab;
extern const AudioTrackData g_audio_intro_battle;
extern const AudioTrackData g_audio_meet_prof_oak;
extern const AudioTrackData g_audio_wild_battle;
