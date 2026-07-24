"""Convert the four title screen channels from pokered into GBA data.

Channel routing:
  channels[0] = Ch1 -> GBA square 1  (75% duty, lead melody + harmony)
  channels[1] = Ch2 -> GBA square 2  (25% duty, counter-melody)
  channels[2] = Ch3 -> GBA wave      (wave0 sine-like, bass)
  channels[3] = Ch4 -> GBA noise     (drum channel)

Pitch formula: period = 2048 - 131072/hz for square and wave (same on GBA).

pokered square (Ch1, Ch2): octave_shift=+1 (pokered square plays 1 octave
  above our formula's same octave due to GB engine shift arithmetic).
pokered wave (Ch3): octave_shift=0 (wave plays at half-freq of square in GB,
  which cancels the +1 shift relative to our formula).
"""
import math
import re
from pathlib import Path

SRC = Path("refs/pokered/audio/music/titlescreen.asm")
OUT = Path("src/audio/title_audio_data.c")

NOTE_SEMITONES = {"C_": 0, "C#": 1, "D_": 2, "D#": 3, "E_": 4,
                  "F_": 5, "F#": 6, "G_": 7, "G#": 8, "A_": 9,
                  "A#": 10, "B_": 11}

def period(octave, note):
    midi = (octave + 1) * 12 + NOTE_SEMITONES[note]
    hz = 440.0 * (2.0 ** ((midi - 69) / 12.0))
    return max(1, min(2047, round(2048.0 - 131072.0 / hz)))

# Drum instrument -> (REG_SOUND4CNT_L, NR43)
# From refs/pokered/audio/sfx/noise_instrument0N_3.asm
# noise_note args: pitch, volume, fade, NR43
# REG_SOUND4CNT_L = (vol<<12) | (0 if fade>=0 else 0x0800) | (|fade|<<8)
DRUM_INSTR = {
    1: (0xC100, 0x33),  # vol=12, fade=1(decrease), NR43=0x33
    2: (0xB100, 0x33),  # vol=11
    3: (0xA100, 0x33),  # vol=10
    4: (0x8100, 0x33),  # vol=8
    5: (0x8400, 0x37),  # vol=8, fade=4(decrease), NR43=0x37 (drum roll, use first step)
}

def flatten_channel(text, ch_num, octave_shift=0, is_noise=False):
    """Parse a title screen channel, flatten subroutine calls, expand counted loops.

    Returns (events, loop_start) where loop_start is the command index that
    AUDIO_CMD_LOOP should jump back to (position of .mainloop body start).
    """
    ch_label = f"Music_TitleScreen_Ch{ch_num}::"
    start = text.index(ch_label)
    end_label = f"Music_TitleScreen_Ch{ch_num + 1}::"
    block = text[start:text.index(end_label)] if end_label in text else text[start:]
    lines = block.splitlines()

    # Map label names to their line indices (the label declaration line itself)
    label_pos = {}
    for i, line in enumerate(lines):
        m = re.match(r'\s*(\.\w+)\s*:', line)
        if m:
            label_pos[m.group(1)] = i

    events = []
    loop_start = [0]
    state = {'octave': 4}

    def process(start_line, end_line):
        """Process lines[start_line:end_line], inlining subs, expanding loops."""
        i = start_line
        while i < end_line:
            raw = lines[i].strip()
            line = raw.split(';', 1)[0].strip()
            i += 1
            if not line:
                continue
            # skip label declarations (both Music_* and .sub*)
            if re.match(r'\w[\w_]*::', line) or re.match(r'\.\w+\s*:', line):
                continue

            m = re.match(r'octave (\d+)$', line)
            if m:
                state['octave'] = int(m.group(1))
                continue

            m = re.match(r'note_type (\d+),\s*(-?\d+),\s*(-?\d+)$', line)
            if m:
                speed = int(m.group(1))
                vol = max(0, min(15, int(m.group(2))))
                fade = int(m.group(3))
                if not is_noise:
                    events.append(('AUDIO_CMD_NOTE_TYPE', speed, vol, fade & 0xff))
                else:
                    # drum_speed masquerading as note_type - only update speed
                    events.append(('AUDIO_CMD_NOTE_TYPE', speed, 0, 0))
                continue

            m = re.match(r'drum_speed (\d+)$', line)
            if m:
                events.append(('AUDIO_CMD_NOTE_TYPE', int(m.group(1)), 0, 0))
                continue

            m = re.match(r'duty_cycle (\d+)$', line)
            if m:
                if not is_noise:
                    events.append(('AUDIO_CMD_DUTY', int(m.group(1)), 0, 0))
                continue

            m = re.match(r'tempo (\d+)$', line)
            if m:
                events.append(('AUDIO_CMD_TEMPO', int(m.group(1)), 0, 0))
                continue

            m = re.match(r'vibrato (\d+),\s*(\d+),\s*(\d+)$', line)
            if m:
                if not is_noise:
                    events.append(('AUDIO_CMD_VIBRATO', int(m.group(1)), int(m.group(2)), int(m.group(3))))
                continue

            # ignore master volume command
            if re.match(r'volume \d+,\s*\d+$', line):
                continue

            m = re.match(r'note ([A-G]#?_?),\s*(\d+)$', line)
            if m:
                name, length = m.group(1), int(m.group(2))
                if not is_noise:
                    events.append(('AUDIO_CMD_NOTE',
                                   period(state['octave'] + octave_shift, name),
                                   length - 1, 0))
                continue

            m = re.match(r'rest (\d+)$', line)
            if m:
                events.append(('AUDIO_CMD_REST', 0, int(m.group(1)) - 1, 0))
                continue

            m = re.match(r'drum_note (\d+),\s*(\d+)$', line)
            if m:
                instr, length = int(m.group(1)), int(m.group(2))
                cnt_l, nr43 = DRUM_INSTR[instr]
                events.append(('AUDIO_CMD_DRUM', cnt_l, length - 1, nr43))
                continue

            m = re.match(r'sound_call (\.\w+)$', line)
            if m:
                sub = m.group(1)
                if sub in label_pos:
                    process(label_pos[sub] + 1, len(lines))
                continue

            m = re.match(r'sound_loop 0,\s*\.\w+$', line)
            if m:
                events.append(('AUDIO_CMD_LOOP', 0, 0, 0))
                return  # stop; rest of block is subroutine definitions

            m = re.match(r'sound_loop (\d+),\s*(\.\w+)$', line)
            if m:
                n, target = int(m.group(1)), m.group(2)
                if n > 0 and target in label_pos:
                    body_start = label_pos[target] + 1
                    body_end = i - 1  # sound_loop line index (i already incremented)
                    for _ in range(n):
                        process(body_start, body_end)
                continue

            # pitch_slide: skip (emit subsequent note at its target pitch)
            if re.match(r'pitch_slide', line):
                continue

            if line == 'sound_ret':
                return

    # Process header (before .mainloop) then mark loop_start
    if '.mainloop' in label_pos:
        ml_line = label_pos['.mainloop']
        process(1, ml_line)      # header: tempo, vibrato, duty, intro notes
        loop_start[0] = len(events)
        process(ml_line + 1, len(lines))
    else:
        process(1, len(lines))
        events.append(('AUDIO_CMD_LOOP', 0, 0, 0))

    return events, loop_start[0]


# wave0 waveform (pokered instrument 0, used by title screen Ch3)
# Samples (32 x 4-bit): 0,2,4,6,8,10,12,14,15,15,15,14,14,13,13,12,
#                       12,11,10,9,8,7,6,5,4,4,3,3,2,2,1,1
# GBA WAVE_RAM: 8 u16s, high byte read first, high nibble = first sample
WAVE0 = [
    0x0246, 0x8ACE, 0xFFFE, 0xEDDC,
    0xCBA9, 0x8765, 0x4433, 0x2211,
]

text = SRC.read_text()

ch1, ch1_loop = flatten_channel(text, 1, octave_shift=1)
ch2, ch2_loop = flatten_channel(text, 2, octave_shift=1)
ch3, ch3_loop = flatten_channel(text, 3, octave_shift=0)
ch4, ch4_loop = flatten_channel(text, 4, is_noise=True)

channels = [
    ('ch1', ch1, ch1_loop),
    ('ch2', ch2, ch2_loop),
    ('ch3', ch3, ch3_loop),
    ('ch4', ch4, ch4_loop),
]

with OUT.open('w', newline='\n') as f:
    f.write('#include "audio_data.h"\n\n')
    f.write('// Generated from refs/pokered/audio/music/titlescreen.asm.\n')
    f.write('// Subroutines inlined; counted loops expanded; pitch_slide skipped.\n\n')

    for name, evts, _ in channels:
        f.write(f'static const AudioCommand s_title_{name}[] = {{\n')
        for op, value, arg1, arg2 in evts:
            f.write(f'    {{ {op}, {value:#6x}, {arg1:3d}, {arg2:#4x} }},\n')
        f.write('};\n\n')

    f.write('static const u16 s_title_wave0[8] = {\n')
    for w in WAVE0:
        f.write(f'    {w:#06x},\n')
    f.write('};\n\n')

    f.write('const AudioTrackData g_audio_title_screen = {\n    .channels = {\n')
    for name, evts, ls in channels:
        count = len(evts)
        f.write(f'        {{ s_title_{name}, {count}, {ls} }},\n')
    f.write('    },\n    .wave_data = s_title_wave0,\n};\n')
