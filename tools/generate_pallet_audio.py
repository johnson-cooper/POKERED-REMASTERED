"""Convert the three Pallet Town channels from pokered into GBA data.

Channel routing:
  channels[0] = harmony  -> GBA square 1  (25% duty, plucky arpeggios)
  channels[1] = bass     -> GBA square 2  (50% duty, steady bass pulses)
  channels[2] = lead     -> GBA wave      (triangle waveform, smooth melody)
  channels[3] = empty

Pitch formula: period = 2048 - 131072/hz for all channels (GBA PSG formula).

pokered square channels play 1 standard octave higher than the same octave/note
in our formula, so octave_shift=+1 for Ch1 (lead) and Ch2 (harmony).
pokered Ch3 is a wave channel which plays at half-frequency of pokered square
for the same period, cancelling out the +1 shift, so octave_shift=0 for bass.
"""
import math
import re
from pathlib import Path

SRC = Path("refs/pokered/audio/music/pallettown.asm")
OUT = Path("src/audio/audio_data.c")

NOTE_SEMITONES = {"C_": 0, "C#": 1, "D_": 2, "D#": 3, "E_": 4,
                  "F_": 5, "F#": 6, "G_": 7, "G#": 8, "A_": 9,
                  "A#": 10, "B_": 11}

def period(octave, note):
    midi = (octave + 1) * 12 + NOTE_SEMITONES[note]
    hz = 440.0 * (2.0 ** ((midi - 69) / 12.0))
    return max(1, min(2047, round(2048.0 - 131072.0 / hz)))

def channel(text, number, octave_shift=0,
            force_duty=None, volume_fn=None, force_fade=None,
            insert_duty=None):
    """Parse a pokered channel block and return a flat event list.

    force_duty: if set, replace all duty_cycle values with this
    volume_fn:  if set, apply to each note_type volume
    force_fade: if set, override all note_type fades
    insert_duty: if set, prepend an AUDIO_CMD_DUTY with this value
    """
    start = text.index(f"Music_PalletTown_Ch{number}::")
    end = text.find("\nMusic_PalletTown_Ch", start + 1)
    block = text[start:] if end < 0 else text[start:end]

    octave = 4
    events = []

    if insert_duty is not None:
        events.append(("AUDIO_CMD_DUTY", insert_duty, 0, 0))

    for raw in block.splitlines():
        line = raw.split(";", 1)[0].strip()

        m = re.match(r"octave (\d+)", line)
        if m:
            octave = int(m.group(1))
            events.append(("AUDIO_CMD_OCTAVE", octave, 0, 0))
            continue

        m = re.match(r"note_type \d+,\s*(-?\d+),\s*(-?\d+)", line)
        if m:
            vol = max(0, min(15, int(m.group(1))))
            fade = int(m.group(2))
            if volume_fn is not None:
                vol = volume_fn(vol)
            if force_fade is not None:
                fade = force_fade
            events.append(("AUDIO_CMD_NOTE_TYPE", 12, vol, fade & 0xff))
            continue

        m = re.match(r"tempo (\d+)", line)
        if m:
            events.append(("AUDIO_CMD_TEMPO", int(m.group(1)), 0, 0))
            continue

        m = re.match(r"duty_cycle (\d+)", line)
        if m:
            duty = force_duty if force_duty is not None else int(m.group(1))
            events.append(("AUDIO_CMD_DUTY", duty, 0, 0))
            continue

        m = re.match(r"vibrato (\d+),\s*(\d+),\s*(\d+)", line)
        if m:
            events.append(("AUDIO_CMD_VIBRATO", int(m.group(1)), int(m.group(2)), int(m.group(3))))
            continue

        m = re.match(r"note ([A-G]#?_?),\s*(\d+)", line)
        if m:
            name, length = m.group(1), int(m.group(2))
            events.append(("AUDIO_CMD_NOTE", period(octave + octave_shift, name), length - 1, 0))
            continue

        m = re.match(r"rest (\d+)", line)
        if m:
            events.append(("AUDIO_CMD_REST", 0, int(m.group(1)) - 1, 0))
            continue

        m = re.match(r"sound_loop 0,", line)
        if m:
            events.append(("AUDIO_CMD_LOOP", 0, 0, 0))
            break

    return events


text = SRC.read_text()

ch_lead    = channel(text, 1, octave_shift=1)
ch_harmony = channel(text, 2, octave_shift=1,
                     force_duty=1,
                     volume_fn=lambda v: max(1, (v + 3) // 4))
ch_bass    = channel(text, 3, octave_shift=0,
                     insert_duty=2,
                     volume_fn=lambda _: 3,
                     force_fade=0)

with OUT.open("w", newline="\n") as f:
    f.write('#include "audio_data.h"\n\n')
    f.write('// Generated from refs/pokered/audio/music/pallettown.asm.\n')
    f.write('// All channels: period = 2048 - 131072/hz (GBA PSG formula).\n\n')

    for name, events in (("lead", ch_lead), ("harmony", ch_harmony), ("bass", ch_bass)):
        f.write(f"static const AudioCommand s_pallet_{name}[] = {{\n")
        for op, value, arg1, arg2 in events:
            f.write(f"    {{ {op}, {value:4d}, {arg1:3d}, {arg2:3d} }},\n")
        f.write("};\n\n")

    f.write("const AudioTrackData g_audio_pallet_town = {\n    .channels = {\n")
    for name in ("harmony", "bass", "lead"):
        f.write(f"        {{ s_pallet_{name},"
                f" sizeof(s_pallet_{name}) / sizeof(s_pallet_{name}[0]),"
                f" 0 }},\n")
    f.write("        { NULL, 0, 0 },\n")  # channels[3] unused
    f.write("    },\n    .wave_data = NULL,\n};\n")
