#include "audio.h"
#include "audio_data.h"
#include "gba.h"
#include <gba_sound.h>

typedef struct {
    u16 position;
    u8 remaining;
    u8 fractional;
    u8 speed;
    u8 volume;
    u8 duty;
    s8 fade;
    u16 tempo;
    u16 period;
    u16 base_period;
    u8 vibrato_delay;
    u8 vibrato_extent;
    u8 vibrato_rate;
    u8 vibrato_counter;
    bool8 vibrato_up;
} MusicChannelState;

typedef struct {
    const AudioTrackData *track;
    MusicChannelState channels[4];
    bool8 active;
} MusicState;

static MusicState s_music;
static AudioMusicId s_music_id;
static u8 s_sfx_frames;
static u8 s_sfx_channel;
static AudioSfxId s_sfx_id;
static bool8 s_battle_intro_sfx_suppressed;

static const AudioTrackData *audio_track_for_id(AudioMusicId id) {
    if (id == AUDIO_MUSIC_PALLET_TOWN)      return &g_audio_pallet_town;
    if (id == AUDIO_MUSIC_TITLE_SCREEN)     return &g_audio_title_screen;
    if (id == AUDIO_MUSIC_TRAINER_BATTLE)   return &g_audio_trainer_battle;
    if (id == AUDIO_MUSIC_DEFEATED_TRAINER) return &g_audio_defeated_trainer;
    if (id == AUDIO_MUSIC_ROUTES_1)         return &g_audio_routes1;
    if (id == AUDIO_MUSIC_OAKS_LAB)         return &g_audio_oakslab;
    if (id == AUDIO_MUSIC_INTRO_BATTLE)     return &g_audio_intro_battle;
    if (id == AUDIO_MUSIC_MEET_PROF_OAK)    return &g_audio_meet_prof_oak;
    if (id == AUDIO_MUSIC_WILD_BATTLE)      return &g_audio_wild_battle;
    return NULL;
}

static void audio_stop_channels(void) {
    REG_SOUND1CNT_L = 0;
    REG_SOUND1CNT_H = 0;
    REG_SOUND2CNT_L = 0;
    REG_SOUND2CNT_H = 0;
    REG_SOUND3CNT_L = 0;
    REG_SOUND4CNT_L = 0;
    REG_SOUND4CNT_H = 0;
}

static void audio_write_square(u8 channel, u16 period, u8 volume, u8 duty, s8 fade) {
    u16 control = (u16)(((volume & 0xF) << 12) |
                        ((fade < 0) ? 0x0800u : 0u) |
                        (u16)(((fade < 0) ? (u8)(-fade) : (u8)fade) & 7u) << 8 |
                        ((duty & 3u) << 6));
    if (channel == 0) {
        REG_SOUND1CNT_H = control;
        REG_SOUND1CNT_X = (u16)((period & 0x7FF) | 0x8000);
    } else {
        REG_SOUND2CNT_L = control;
        REG_SOUND2CNT_H = (u16)((period & 0x7FF) | 0x8000);
    }
}

static const u16 s_triangle[8] = {
    0x0123, 0x4567, 0x89AB, 0xCDEF,
    0xFEDC, 0xBA98, 0x7654, 0x3210,
};

static void audio_wave_load(const u16 *wave_data) {
    const u16 *src = wave_data ? wave_data : s_triangle;
    REG_SOUND3CNT_L = SOUND3_STOP | SOUND3_SETBANK(1);
    for (u8 i = 0; i < 8; i++)
        WAVE_RAM[i] = src[i];
    REG_SOUND3CNT_L = SOUND3_STOP | SOUND3_SETBANK(0);
}

static void audio_write_wave(u16 period, u8 volume) {
    (void)volume;
    REG_SOUND3CNT_L = SOUND3_PLAY;
    REG_SOUND3CNT_H = (u16)(2u << 13);
    REG_SOUND3CNT_X = (u16)((period & 0x7FF) | 0x8000);
}

void audio_init(void) {
    REG_SOUNDBIAS = 0x0200;
    REG_SOUNDCNT_X = SNDSTAT_ENABLE;
    REG_SOUNDCNT_L = (u16)(DMGSNDCTRL_LVOL(7) | DMGSNDCTRL_RVOL(7) |
                           DMGSNDCTRL_LSQR1 | DMGSNDCTRL_LSQR2 |
                           DMGSNDCTRL_LTRI | DMGSNDCTRL_LNOISE |
                           DMGSNDCTRL_RSQR1 | DMGSNDCTRL_RSQR2 |
                           DMGSNDCTRL_RTRI | DMGSNDCTRL_RNOISE);
    REG_SOUNDCNT_H = DSOUNDCTRL_DMG25;
    audio_stop_channels();
    s_music.track = NULL;
    s_music.active = FALSE;
    s_music_id = AUDIO_MUSIC_NONE;
    s_sfx_frames = 0;
    s_sfx_channel = 1;
    s_sfx_id = AUDIO_SFX_SELECT;
    s_battle_intro_sfx_suppressed = FALSE;
}

static void music_channel_reset(MusicChannelState *ch) {
    ch->position = 0;
    ch->remaining = 0;
    ch->fractional = 0;
    ch->speed = 12;
    ch->volume = 12;
    ch->duty = 2;
    ch->fade = 0;
    ch->tempo = 160;
    ch->period = 0;
    ch->base_period = 0;
    ch->vibrato_delay = 0;
    ch->vibrato_extent = 0;
    ch->vibrato_rate = 0;
    ch->vibrato_counter = 0;
    ch->vibrato_up = TRUE;
}

void audio_music_stop(void) {
    s_music.track = NULL;
    s_music.active = FALSE;
    s_music_id = AUDIO_MUSIC_NONE;
    for (u8 i = 0; i < 4; i++) music_channel_reset(&s_music.channels[i]);
    audio_stop_channels();
}

AudioMusicId audio_music_current(void) {
    return s_music_id;
}

void audio_music_play(AudioMusicId id) {
    if (id == s_music_id && s_music.active) return;
    const AudioTrackData *track = audio_track_for_id(id);
    if (!track) {
        audio_music_stop();
        return;
    }
    s_music_id = id;
    s_music.track = track;
    s_music.active = TRUE;
    for (u8 i = 0; i < 4; i++) music_channel_reset(&s_music.channels[i]);
    audio_wave_load(track->wave_data);
}

void audio_sfx_play(AudioSfxId id) {
    // UI SFX use the second pulse voice. The noise channel's raw NR43 values
    // are useful for drums, but sound like harsh static for short UI clicks.
    u16 period = 1700;
    s_sfx_channel = 1;

    if (s_battle_intro_sfx_suppressed)
        return;
    s_sfx_id = id;
    switch (id) {
    case AUDIO_SFX_SELECT:  period = 1700; s_sfx_frames = 2; break;
    case AUDIO_SFX_CONFIRM: period = 1850; s_sfx_frames = 4; break;
    case AUDIO_SFX_CANCEL:  period = 1550; s_sfx_frames = 3; break;
    case AUDIO_SFX_TEXT:    period = 1650; s_sfx_frames = 1; break;
    case AUDIO_SFX_DENIED:  period = 1450; s_sfx_frames = 5; break;
    case AUDIO_SFX_START:   period = 1900; s_sfx_frames = 6; break;
    // Battle menu uses a lower register so it reads as the normal menu click
    // against the trainer-battle arrangement instead of a piercing blip.
    case AUDIO_SFX_BATTLE_SELECT:
        period = 1700;
        s_sfx_frames = 2;
        s_sfx_channel = 0;
        break;
    case AUDIO_SFX_BATTLE_CONFIRM:
        period = 1850;
        s_sfx_frames = 4;
        s_sfx_channel = 0;
        break;
    case AUDIO_SFX_WARP_OUT:       period = 1850; s_sfx_frames = 10; break;
    case AUDIO_SFX_WARP_IN:        period = 1350; s_sfx_frames = 10; break;
    case AUDIO_SFX_PAUSE_OPEN:     period = 2050; s_sfx_frames = 8; break;
    case AUDIO_SFX_PAUSE_CLOSE:    period = 1450; s_sfx_frames = 6; break;
    case AUDIO_SFX_CRY_BULBASAUR:  period = 1200; s_sfx_frames = 24; break;
    case AUDIO_SFX_CRY_CHARMANDER: period = 1500; s_sfx_frames = 22; break;
    case AUDIO_SFX_CRY_SQUIRTLE:   period = 1750; s_sfx_frames = 20; break;
    case AUDIO_SFX_CRY_WILD:       period = 1320; s_sfx_frames = 20; break;
    }
    // Fixed 1/4-duty, no-envelope pulse for every menu effect. Battle SFX
    // intentionally share the normal UI sound character rather than using a
    // raw noise-channel click.
    if (s_sfx_channel == 0) {
        REG_SOUND1CNT_H = 0x8040;
        REG_SOUND1CNT_X = (u16)(period | 0x8000);
    } else {
        REG_SOUND2CNT_L = 0x8040;
        REG_SOUND2CNT_H = (u16)(period | 0x8000);
    }
}

void audio_sfx_set_battle_intro(bool8 active) {
    s_battle_intro_sfx_suppressed = active;
}

static void audio_restore_sfx_voice(void) {
    u8 channel = s_sfx_channel;
    if (s_music.active && s_music.channels[channel].period) {
        MusicChannelState *state = &s_music.channels[channel];
        u16 control = (u16)(((state->volume & 0xF) << 12) |
                            ((state->fade < 0) ? 0x0800u : 0u) |
                            (u16)(((state->fade < 0) ? (u8)(-state->fade) :
                                   (u8)state->fade) & 7u) << 8 |
                            ((state->duty & 3u) << 6));
        if (channel == 0) {
            REG_SOUND1CNT_H = control;
            REG_SOUND1CNT_X = (u16)(state->period & 0x7FF);
        } else {
            REG_SOUND2CNT_L = control;
            REG_SOUND2CNT_H = (u16)(state->period & 0x7FF);
        }
    } else {
        if (channel == 0) REG_SOUND1CNT_H = 0;
        else REG_SOUND2CNT_L = 0;
    }
}

static void audio_update_sfx_voice(void) {
    if (!s_sfx_frames) return;
    u16 period = 1700;
    if (s_sfx_id == AUDIO_SFX_WARP_OUT)
        period = (u16)(1850 - (u16)(10 - s_sfx_frames) * 45);
    else if (s_sfx_id == AUDIO_SFX_WARP_IN)
        period = (u16)(1350 + (u16)(10 - s_sfx_frames) * 45);
    else if (s_sfx_id == AUDIO_SFX_PAUSE_OPEN)
        period = (u16)(2050 - (u16)(8 - s_sfx_frames) * 35);
    else if (s_sfx_id == AUDIO_SFX_PAUSE_CLOSE)
        period = (u16)(1450 + (u16)(6 - s_sfx_frames) * 35);
    else if (s_sfx_id == AUDIO_SFX_CRY_BULBASAUR)
        period = (u16)(1200 + (u16)(24 - s_sfx_frames) * 24);
    else if (s_sfx_id == AUDIO_SFX_CRY_CHARMANDER)
        period = (u16)(1500 - (u16)(22 - s_sfx_frames) * 30);
    else if (s_sfx_id == AUDIO_SFX_CRY_SQUIRTLE)
        period = (u16)(1750 + (u16)(20 - s_sfx_frames) * 28);
    else if (s_sfx_id == AUDIO_SFX_CRY_WILD)
        period = (u16)(1320 - (u16)(20 - s_sfx_frames) * 18);
    else
        return;
    if (s_sfx_channel == 0)
        REG_SOUND1CNT_X = (u16)(period | 0x8000);
    else
        REG_SOUND2CNT_H = (u16)(period | 0x8000);
}

static void audio_apply_vibrato(u8 channel, MusicChannelState *state) {
    if (state->vibrato_extent == 0 || state->vibrato_rate == 0) return;
    if (state->vibrato_delay) {
        state->vibrato_delay--;
        return;
    }
    if (state->vibrato_counter) {
        state->vibrato_counter--;
        return;
    }
    state->vibrato_counter = state->vibrato_rate;
    state->vibrato_up = !state->vibrato_up;
    s16 offset = state->vibrato_up ? state->vibrato_extent : -(s16)state->vibrato_extent;
    state->period = (u16)((s16)state->base_period + offset);
    if (channel == 0)
        REG_SOUND1CNT_X = (u16)(state->period & 0x7FF);
    else if (channel == 1)
        REG_SOUND2CNT_H = (u16)(state->period & 0x7FF);
    else if (channel == 2)
        REG_SOUND3CNT_X = (u16)(state->period & 0x7FF);
    /* channel 3 (noise): no frequency register */
}

static void audio_play_note_or_rest(u8 channel, MusicChannelState *state,
                                    const AudioCommand *command) {
    u16 delay = (u16)((command->arg1 + 1) * state->speed * state->tempo);
    delay = (u16)(delay + state->fractional);
    state->remaining = (u8)(delay >> 8);
    state->fractional = (u8)delay;

    if (command->type == AUDIO_CMD_REST) {
        state->period = 0;
        if (channel == 0) REG_SOUND1CNT_H = 0;
        else if (channel == 1) REG_SOUND2CNT_L = 0;
        else if (channel == 2) REG_SOUND3CNT_L = SOUND3_STOP;
        else REG_SOUND4CNT_L = 0;
    } else {
        state->base_period = command->value;
        state->period = command->value;
        if (channel < 2)
            audio_write_square(channel, state->period, state->volume,
                               state->duty, state->fade);
        else if (channel == 2)
            audio_write_wave(state->period, state->volume);
        /* channel 3 NOTE: use AUDIO_CMD_DRUM instead */
    }
}

void audio_update(void) {
    if (s_sfx_frames) {
        audio_update_sfx_voice();
        if (--s_sfx_frames == 0)
            audio_restore_sfx_voice();
    }

    if (!s_music.active || !s_music.track) return;
    for (u8 channel = 0; channel < 4; channel++) {
        const AudioChannelData *data = &s_music.track->channels[channel];
        MusicChannelState *state = &s_music.channels[channel];
        // SFX temporarily owns square channel 2. Without this guard, the
        // trainer music can rewrite SOUND2 during the same VBlank and turn a
        // short menu click into a clipped, high-pitched partial note.
        if ((channel == s_sfx_channel) && s_sfx_frames)
            continue;
        if (data->count == 0) continue;
        // Pokered keeps the high byte of the 16-bit note delay. A counter of
        // one means the next note is due on this update; waiting an extra
        // frame here makes channels with more notes drift behind the others.
        if (state->remaining > 1) {
            state->remaining--;
            audio_apply_vibrato(channel, state);
            continue;
        }
        state->remaining = 0;
        while (TRUE) {
            const AudioCommand *command = &data->commands[state->position++];
            switch (command->type) {
            case AUDIO_CMD_NOTE_TYPE:
                state->speed = (u8)command->value;
                state->volume = command->arg1;
                state->fade = (s8)command->arg2;
                break;
            case AUDIO_CMD_DUTY:
                state->duty = (u8)command->value;
                break;
            case AUDIO_CMD_OCTAVE:
                break;
            case AUDIO_CMD_TEMPO:
                for (u8 j = 0; j < 4; j++)
                    s_music.channels[j].tempo = command->value;
                state->fractional = 0;
                break;
            case AUDIO_CMD_VIBRATO:
                state->vibrato_delay = (u8)command->value;
                state->vibrato_extent = command->arg1;
                state->vibrato_rate = command->arg2;
                break;
            case AUDIO_CMD_LOOP:
                state->position = data->loop_start;
                break;
            case AUDIO_CMD_NOTE:
            case AUDIO_CMD_REST:
                audio_play_note_or_rest(channel, state, command);
                return;
            case AUDIO_CMD_DRUM:
                {
                    u16 delay = (u16)((command->arg1 + 1) * state->speed * state->tempo);
                    delay = (u16)(delay + state->fractional);
                    state->remaining = (u8)(delay >> 8);
                    state->fractional = (u8)delay;
                    REG_SOUND4CNT_L = command->value;
                    REG_SOUND4CNT_H = (u16)(0x8000u | command->arg2);
                }
                return;
            }
            if (state->position >= data->count)
                state->position = data->loop_start;
        }
    }
}
