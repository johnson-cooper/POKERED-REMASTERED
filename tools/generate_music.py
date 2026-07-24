"""Convert pokered music ASM files to GBA AudioCommand data.

Generates trainer_battle, defeated_trainer, routes1, and oakslab.

Channel routing (matches pokered):
  Ch1 -> GBA square 1   octave_shift=+1  (pokered sq plays 1 oct above our formula)
  Ch2 -> GBA square 2   octave_shift=+1
  Ch3 -> GBA wave        octave_shift=0   (wave half-freq cancels the +1 shift)
  Ch4 -> GBA noise       is_noise=True    (drum_note / drum_speed)

Skips: toggle_perfect_pitch, volume, pitch_slide (not implemented in our engine).
"""
import re
from pathlib import Path

REFS = Path("refs/pokered/audio")
OUT = Path("src/audio")

NOTE_SEMITONES = {
    "C_": 0, "C#": 1, "D_": 2, "D#": 3, "E_": 4,
    "F_": 5, "F#": 6, "G_": 7, "G#": 8, "A_": 9,
    "A#": 10, "B_": 11,
}


def period(octave, note):
    midi = (octave + 1) * 12 + NOTE_SEMITONES[note]
    hz = 440.0 * (2.0 ** ((midi - 69) / 12.0))
    return max(1, min(2047, round(2048.0 - 131072.0 / hz)))


def load_drum_instruments():
    """Read first noise_note from each noise_instrument*_3.asm."""
    drum = {}
    for f in sorted((REFS / "sfx").glob("noise_instrument*_3.asm")):
        m = re.search(r"noise_instrument(\d+)_3", f.name)
        if not m:
            continue
        num = int(m.group(1))
        text = f.read_text()
        m2 = re.search(r"noise_note\s+\d+,\s*(\d+),\s*(\d+),\s*(\d+)", text)
        if m2:
            vol, fade, nr43 = int(m2.group(1)), int(m2.group(2)), int(m2.group(3))
            # REG_SOUND4CNT_L: vol in bits 12-15, fade pace in bits 8-10.
            # Decrease direction = bit 11 clear.
            cnt_l = (vol << 12) | (fade << 8)
            drum[num] = (cnt_l, nr43)
    return drum


DRUM_INSTR = load_drum_instruments()


def flatten_channel(text, ch_label, octave_shift=0, is_noise=False):
    """Flatten one channel into a list of AudioCommands.

    Returns (events, loop_start) where loop_start is the command index
    AUDIO_CMD_LOOP jumps to (.mainloop body start).
    """
    start = text.index(ch_label)
    next_ch = re.search(r"\nMusic_\w+_Ch\d+::", text[start + 1:])
    block = text[start: start + 1 + next_ch.start()] if next_ch else text[start:]
    lines = block.splitlines()

    label_pos = {}
    for i, line in enumerate(lines):
        m = re.match(r"\s*(\.\w+)\s*:", line)
        if m:
            label_pos[m.group(1)] = i

    events = []
    loop_start = [0]
    state = {"octave": 4}

    def process(start_line, end_line):
        i = start_line
        while i < end_line:
            raw = lines[i].strip()
            line = raw.split(";", 1)[0].strip()
            i += 1
            if not line:
                continue
            # skip label declarations
            if re.match(r"\w[\w_]*::", line) or re.match(r"\.\w+\s*:", line):
                continue

            if line in ("toggle_perfect_pitch", "sound_ret"):
                if line == "sound_ret":
                    return
                continue

            if re.match(r"volume \d+,\s*\d+$", line):
                continue

            if re.match(r"pitch_slide", line):
                continue

            m = re.match(r"octave (\d+)$", line)
            if m:
                state["octave"] = int(m.group(1))
                continue

            m = re.match(r"note_type (\d+),\s*(-?\d+),\s*(-?\d+)$", line)
            if m:
                speed = int(m.group(1))
                vol = max(0, min(15, int(m.group(2))))
                fade = int(m.group(3))
                if not is_noise:
                    events.append(("AUDIO_CMD_NOTE_TYPE", speed, vol, fade & 0xFF))
                else:
                    events.append(("AUDIO_CMD_NOTE_TYPE", speed, 0, 0))
                continue

            m = re.match(r"drum_speed (\d+)$", line)
            if m:
                events.append(("AUDIO_CMD_NOTE_TYPE", int(m.group(1)), 0, 0))
                continue

            m = re.match(r"duty_cycle (\d+)$", line)
            if m:
                if not is_noise:
                    events.append(("AUDIO_CMD_DUTY", int(m.group(1)), 0, 0))
                continue

            m = re.match(r"tempo (\d+)$", line)
            if m:
                events.append(("AUDIO_CMD_TEMPO", int(m.group(1)), 0, 0))
                continue

            m = re.match(r"vibrato (\d+),\s*(\d+),\s*(\d+)$", line)
            if m:
                if not is_noise:
                    events.append(("AUDIO_CMD_VIBRATO",
                                   int(m.group(1)), int(m.group(2)), int(m.group(3))))
                continue

            m = re.match(r"note ([A-G]#?_?),\s*(\d+)$", line)
            if m:
                name, length = m.group(1), int(m.group(2))
                if not is_noise:
                    events.append(("AUDIO_CMD_NOTE",
                                   period(state["octave"] + octave_shift, name),
                                   length - 1, 0))
                continue

            m = re.match(r"rest (\d+)$", line)
            if m:
                events.append(("AUDIO_CMD_REST", 0, int(m.group(1)) - 1, 0))
                continue

            m = re.match(r"drum_note (\d+),\s*(\d+)$", line)
            if m:
                instr, length = int(m.group(1)), int(m.group(2))
                if instr in DRUM_INSTR:
                    cnt_l, nr43 = DRUM_INSTR[instr]
                    events.append(("AUDIO_CMD_DRUM", cnt_l, length - 1, nr43))
                continue

            m = re.match(r"sound_call (\.\w+)$", line)
            if m:
                sub = m.group(1)
                if sub in label_pos:
                    process(label_pos[sub] + 1, len(lines))
                continue

            m = re.match(r"sound_loop 0,\s*\.\w+$", line)
            if m:
                events.append(("AUDIO_CMD_LOOP", 0, 0, 0))
                return

            m = re.match(r"sound_loop (\d+),\s*(\.\w+)$", line)
            if m:
                n, target = int(m.group(1)), m.group(2)
                if n > 0 and target in label_pos:
                    body_start = label_pos[target] + 1
                    body_end = i - 1
                    for _ in range(n):
                        process(body_start, body_end)
                continue

    if ".mainloop" in label_pos:
        ml_line = label_pos[".mainloop"]
        process(1, ml_line)
        loop_start[0] = len(events)
        process(ml_line + 1, len(lines))
    else:
        process(1, len(lines))
        events.append(("AUDIO_CMD_LOOP", 0, 0, 0))

    return events, loop_start[0]


def write_track(out_path, c_var, src_comment, channels_data):
    """Write a C source file with AudioCommand arrays and an AudioTrackData struct."""
    with out_path.open("w", newline="\n") as f:
        f.write('#include "audio_data.h"\n\n')
        f.write(f"// Generated from {src_comment}.\n")
        f.write("// Subroutines inlined; counted loops expanded; pitch_slide skipped.\n\n")

        for ch_name, evts, _ in channels_data:
            if not evts:
                continue
            f.write(f"static const AudioCommand s_{c_var}_{ch_name}[] = {{\n")
            for op, value, arg1, arg2 in evts:
                f.write(f"    {{ {op}, {value:#6x}, {arg1:3d}, {arg2:#4x} }},\n")
            f.write("};\n\n")

        f.write(f"const AudioTrackData g_audio_{c_var} = {{\n    .channels = {{\n")
        for ch_name, evts, ls in channels_data:
            if evts:
                count = len(evts)
                f.write(f"        {{ s_{c_var}_{ch_name}, {count}, {ls} }},\n")
            else:
                f.write(f"        {{ NULL, 0, 0 }},\n")
        f.write("    },\n    .wave_data = NULL,\n};\n")


# ── Trainer Battle (3 channels: sq1, sq2, wave) ──────────────────────────────
text = (REFS / "music/trainerbattle.asm").read_text()
tb_ch1, tb_ch1_loop = flatten_channel(text, "Music_TrainerBattle_Ch1::", octave_shift=1)
tb_ch2, tb_ch2_loop = flatten_channel(text, "Music_TrainerBattle_Ch2::", octave_shift=1)
tb_ch3, tb_ch3_loop = flatten_channel(text, "Music_TrainerBattle_Ch3::", octave_shift=0)

write_track(
    OUT / "trainer_battle_audio_data.c",
    "trainer_battle",
    "refs/pokered/audio/music/trainerbattle.asm",
    [
        ("ch1", tb_ch1, tb_ch1_loop),
        ("ch2", tb_ch2, tb_ch2_loop),
        ("ch3", tb_ch3, tb_ch3_loop),
        ("ch4", [], 0),   # no noise channel
    ],
)
print(f"trainer_battle: ch1={len(tb_ch1)} ch2={len(tb_ch2)} ch3={len(tb_ch3)}")

# ── Defeated Trainer (3 channels: sq1, sq2, wave) ────────────────────────────
text = (REFS / "music/defeatedtrainer.asm").read_text()
dt_ch1, dt_ch1_loop = flatten_channel(text, "Music_DefeatedTrainer_Ch1::", octave_shift=1)
dt_ch2, dt_ch2_loop = flatten_channel(text, "Music_DefeatedTrainer_Ch2::", octave_shift=1)
dt_ch3, dt_ch3_loop = flatten_channel(text, "Music_DefeatedTrainer_Ch3::", octave_shift=0)

write_track(
    OUT / "defeated_trainer_audio_data.c",
    "defeated_trainer",
    "refs/pokered/audio/music/defeatedtrainer.asm",
    [
        ("ch1", dt_ch1, dt_ch1_loop),
        ("ch2", dt_ch2, dt_ch2_loop),
        ("ch3", dt_ch3, dt_ch3_loop),
        ("ch4", [], 0),
    ],
)
print(f"defeated_trainer: ch1={len(dt_ch1)} ch2={len(dt_ch2)} ch3={len(dt_ch3)}")

# ── Routes 1 (4 channels: sq1, sq2, wave, noise) ─────────────────────────────
text = (REFS / "music/routes1.asm").read_text()
r1_ch1, r1_ch1_loop = flatten_channel(text, "Music_Routes1_Ch1::", octave_shift=1)
r1_ch2, r1_ch2_loop = flatten_channel(text, "Music_Routes1_Ch2::", octave_shift=1)
r1_ch3, r1_ch3_loop = flatten_channel(text, "Music_Routes1_Ch3::", octave_shift=0)
r1_ch4, r1_ch4_loop = flatten_channel(text, "Music_Routes1_Ch4::", is_noise=True)

write_track(
    OUT / "routes1_audio_data.c",
    "routes1",
    "refs/pokered/audio/music/routes1.asm",
    [
        ("ch1", r1_ch1, r1_ch1_loop),
        ("ch2", r1_ch2, r1_ch2_loop),
        ("ch3", r1_ch3, r1_ch3_loop),
        ("ch4", r1_ch4, r1_ch4_loop),
    ],
)
print(f"routes1: ch1={len(r1_ch1)} ch2={len(r1_ch2)} ch3={len(r1_ch3)} ch4={len(r1_ch4)}")

# ── Oak's Lab (3 channels: sq1, sq2, wave) ───────────────────────────────────
text = (REFS / "music/oakslab.asm").read_text()
ol_ch1, ol_ch1_loop = flatten_channel(text, "Music_OaksLab_Ch1::", octave_shift=1)
ol_ch2, ol_ch2_loop = flatten_channel(text, "Music_OaksLab_Ch2::", octave_shift=1)
ol_ch3, ol_ch3_loop = flatten_channel(text, "Music_OaksLab_Ch3::", octave_shift=0)

write_track(
    OUT / "oakslab_audio_data.c",
    "oakslab",
    "refs/pokered/audio/music/oakslab.asm",
    [
        ("ch1", ol_ch1, ol_ch1_loop),
        ("ch2", ol_ch2, ol_ch2_loop),
        ("ch3", ol_ch3, ol_ch3_loop),
        ("ch4", [], 0),
    ],
)
print(f"oakslab: ch1={len(ol_ch1)} ch2={len(ol_ch2)} ch3={len(ol_ch3)}")

# ── Opening cutscene ─────────────────────────────────────────────────────────
text = (REFS / "music/introbattle.asm").read_text()
ib_ch1, ib_ch1_loop = flatten_channel(text, "Music_IntroBattle_Ch1::", octave_shift=1)
ib_ch2, ib_ch2_loop = flatten_channel(text, "Music_IntroBattle_Ch2::", octave_shift=1)
ib_ch3, ib_ch3_loop = flatten_channel(text, "Music_IntroBattle_Ch3::", octave_shift=0)
write_track(
    OUT / "intro_battle_audio_data.c",
    "intro_battle",
    "refs/pokered/audio/music/introbattle.asm",
    [("ch1", ib_ch1, ib_ch1_loop), ("ch2", ib_ch2, ib_ch2_loop),
     ("ch3", ib_ch3, ib_ch3_loop), ("ch4", [], 0)],
)
print(f"intro_battle: ch1={len(ib_ch1)} ch2={len(ib_ch2)} ch3={len(ib_ch3)}")

# ── Professor Oak encounter cue ─────────────────────────────────────────────
# This is the track used when Oak appears in the Pallet Town opening scene.
text = (REFS / "music/meetprofoak.asm").read_text()
mpo_ch1, mpo_ch1_loop = flatten_channel(text, "Music_MeetProfOak_Ch1::", octave_shift=1)
mpo_ch2, mpo_ch2_loop = flatten_channel(text, "Music_MeetProfOak_Ch2::", octave_shift=1)
mpo_ch3, mpo_ch3_loop = flatten_channel(text, "Music_MeetProfOak_Ch3::", octave_shift=0)
write_track(
    OUT / "meet_prof_oak_audio_data.c",
    "meet_prof_oak",
    "refs/pokered/audio/music/meetprofoak.asm",
    [("ch1", mpo_ch1, mpo_ch1_loop), ("ch2", mpo_ch2, mpo_ch2_loop),
     ("ch3", mpo_ch3, mpo_ch3_loop), ("ch4", [], 0)],
)
print(f"meet_prof_oak: ch1={len(mpo_ch1)} ch2={len(mpo_ch2)} ch3={len(mpo_ch3)}")
