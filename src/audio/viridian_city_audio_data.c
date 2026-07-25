#include "audio_data.h"

/*
 * Viridian City / Pewter City / Saffron City theme
 * Converted from pret/pokered audio/music/cities1.asm
 * pokered octave N -> standard octave (8-N)
 * GBA PSG period = 2048 - 131072/freq
 */

static const AudioCommand s_viridian_lead[] = {
    /*   0 */ { AUDIO_CMD_TEMPO, 144, 0, 0 },  /* tempo 144 */
    /*   1 */ { AUDIO_CMD_VIBRATO, 8, 2, 4 },  /* vibrato 8, 2, 4 */
    /*   2 */ { AUDIO_CMD_DUTY, 3, 0, 0 },  /* duty 3 */
    /*   3 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 5 },  /* note_type 12, 12, 5 */
    /*   4 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*   5 */ { AUDIO_CMD_NOTE, 1890, 3, 0 },  /* G#5 len=4 */
    /*   6 */ { AUDIO_CMD_NOTE, 1871, 3, 0 },  /* F#5 len=4 */
    /*   7 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*   8 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*   9 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /*  10 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /*  11 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  12 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  13 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /*  14 */ { AUDIO_CMD_NOTE, 1812, 3, 0 },  /* C#5 len=4 */
    /*  15 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /*  16 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  17 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /*  18 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  19 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  20 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /*  21 */ { AUDIO_CMD_NOTE_TYPE, 12, 10, 5 },  /* note_type 12, 10, 5 */
    /*  22 */ { AUDIO_CMD_NOTE, 1812, 3, 0 },  /* C#5 len=4 */
    /*  23 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /*  24 */ { AUDIO_CMD_NOTE, 1982, 5, 0 },  /* B_6 len=6 */
    /*  25 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  26 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  27 */ { AUDIO_CMD_NOTE, 1812, 3, 0 },  /* C#5 len=4 */
    /*  28 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /*  29 */ { AUDIO_CMD_NOTE, 1982, 3, 0 },  /* B_6 len=4 */
    /*  30 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 5 },  /* note_type 12, 12, 5 */
    /*  31 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  32 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /*  33 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /*  34 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  35 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /*  36 */ { AUDIO_CMD_NOTE, 1812, 3, 0 },  /* C#5 len=4 */
    /*  37 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /*  38 */ { AUDIO_CMD_NOTE, 1982, 3, 0 },  /* B_6 len=4 */
    /*  39 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  40 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  41 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /*  42 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  43 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  44 */ { AUDIO_CMD_NOTE, 1837, 5, 0 },  /* D#5 len=6 */
    /*  45 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  46 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /*  47 */ { AUDIO_CMD_NOTE, 1982, 3, 0 },  /* B_6 len=4 */
    /*  48 */ { AUDIO_CMD_NOTE_TYPE, 12, 10, 5 },  /* note_type 12, 10, 5 */
    /*  49 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  50 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  51 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /*  52 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /*  53 */ { AUDIO_CMD_NOTE, 1974, 3, 0 },  /* A_6 len=4 */
    /*  54 */ { AUDIO_CMD_NOTE, 1982, 3, 0 },  /* B_6 len=4 */
    /*  55 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /*  56 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  57 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  58 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /*  59 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  60 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /*  61 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  62 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /*  63 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 5 },  /* note_type 12, 12, 5 */
    /*  64 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /*  65 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  66 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /*  67 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  68 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /*  69 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /*  70 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /*  71 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /*  72 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /*  73 */ { AUDIO_CMD_NOTE, 1812, 3, 0 },  /* C#5 len=4 */
    /*  74 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /*  75 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  76 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /*  77 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  78 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  79 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  80 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /*  81 */ { AUDIO_CMD_NOTE_TYPE, 12, 10, 5 },  /* note_type 12, 10, 5 */
    /*  82 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  83 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /*  84 */ { AUDIO_CMD_NOTE, 1974, 1, 0 },  /* A_6 len=2 */
    /*  85 */ { AUDIO_CMD_NOTE, 1982, 5, 0 },  /* B_6 len=6 */
    /*  86 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  87 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  88 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /*  89 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /*  90 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /*  91 */ { AUDIO_CMD_NOTE, 1982, 3, 0 },  /* B_6 len=4 */
    /*  92 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 5 },  /* note_type 12, 12, 5 */
    /*  93 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  94 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /*  95 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /*  96 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /*  97 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /*  98 */ { AUDIO_CMD_NOTE, 1812, 3, 0 },  /* C#5 len=4 */
    /*  99 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 100 */ { AUDIO_CMD_NOTE, 1982, 3, 0 },  /* B_6 len=4 */
    /* 101 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 102 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 103 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 104 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 105 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 106 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /* 107 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 108 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 109 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 110 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 111 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 112 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 113 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 114 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /* 115 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 116 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 117 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 118 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 119 */ { AUDIO_CMD_NOTE_TYPE, 12, 10, 5 },  /* note_type 12, 10, 5 */
    /* 120 */ { AUDIO_CMD_NOTE, 1812, 3, 0 },  /* C#5 len=4 */
    /* 121 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 122 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 123 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 124 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /* 125 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 126 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /* 127 */ { AUDIO_CMD_NOTE_TYPE, 12, 11, 3 },  /* note_type 12, 11, 3 */
    /* 128 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 129 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 130 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 131 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /* 132 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 133 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 134 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 135 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 136 */ { AUDIO_CMD_NOTE, 1915, 1, 0 },  /* B_5 len=2 */
    /* 137 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 138 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 139 */ { AUDIO_CMD_NOTE, 1899, 3, 0 },  /* A_5 len=4 */
    /* 140 */ { AUDIO_CMD_NOTE, 1871, 3, 0 },  /* F#5 len=4 */
    /* 141 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 142 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 143 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 144 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 145 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 146 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /* 147 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 148 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 149 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 150 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 151 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 152 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 153 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /* 154 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 155 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 156 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 157 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 158 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 159 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 160 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 161 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 162 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 163 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 164 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 165 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 166 */ { AUDIO_CMD_NOTE, 1982, 3, 0 },  /* B_6 len=4 */
    /* 167 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 168 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 169 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 170 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 171 */ { AUDIO_CMD_LOOP, 2, 162, 0 },  /* loop 2x -> loop1 */
    /* 172 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 173 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 174 */ { AUDIO_CMD_NOTE, 1982, 3, 0 },  /* B_6 len=4 */
    /* 175 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 176 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /* 177 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 178 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 179 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 180 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 181 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 182 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 183 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 184 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 185 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 186 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /* 187 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 188 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 189 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 190 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 191 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 192 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 193 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 194 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 195 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 196 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 197 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 198 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 199 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 200 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 201 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /* 202 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 203 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 204 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 205 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 206 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 207 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 208 */ { AUDIO_CMD_NOTE, 1915, 1, 0 },  /* B_5 len=2 */
    /* 209 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 210 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 211 */ { AUDIO_CMD_NOTE, 1899, 1, 0 },  /* A_5 len=2 */
    /* 212 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /* 213 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 214 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 215 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 216 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 217 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 218 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 219 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 220 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 221 */ { AUDIO_CMD_NOTE, 1812, 1, 0 },  /* C#5 len=2 */
    /* 222 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 223 */ { AUDIO_CMD_NOTE, 1915, 1, 0 },  /* B_5 len=2 */
    /* 224 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 225 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 226 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 227 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /* 228 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /* 229 */ { AUDIO_CMD_NOTE, 1890, 1, 0 },  /* G#5 len=2 */
    /* 230 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /* 231 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 232 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 233 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 234 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 235 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /* 236 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /* 237 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 238 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 239 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /* 240 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 241 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 242 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 243 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 244 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 245 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 246 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 247 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 248 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 249 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 250 */ { AUDIO_CMD_NOTE, 1982, 1, 0 },  /* B_6 len=2 */
    /* 251 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 252 */ { AUDIO_CMD_NOTE, 1837, 1, 0 },  /* D#5 len=2 */
    /* 253 */ { AUDIO_CMD_NOTE_TYPE, 12, 11, 6 },  /* note_type 12, 11, 6 */
    /* 254 */ { AUDIO_CMD_NOTE, 1871, 7, 0 },  /* F#5 len=8 */
    /* 255 */ { AUDIO_CMD_NOTE, 1871, 3, 0 },  /* F#5 len=4 */
    /* 256 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /* 257 */ { AUDIO_CMD_NOTE, 1849, 7, 0 },  /* E_5 len=8 */
    /* 258 */ { AUDIO_CMD_NOTE_TYPE, 12, 8, 4 },  /* note_type 12, 8, 4 */
    /* 259 */ { AUDIO_CMD_OCTAVE, 2, 0, 0 },  /* octave 2 */
    /* 260 */ { AUDIO_CMD_NOTE, 1982, 3, 0 },  /* B_6 len=4 */
    /* 261 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /* 262 */ { AUDIO_CMD_NOTE, 1849, 1, 0 },  /* E_5 len=2 */
    /* 263 */ { AUDIO_CMD_NOTE, 1871, 1, 0 },  /* F#5 len=2 */
    /* 264 */ { AUDIO_CMD_LOOP, 0, 3, 0 },  /* loop 0x -> mainloop */
};

static const AudioCommand s_viridian_sub[] = {
    /*   0 */ { AUDIO_CMD_VIBRATO, 5, 1, 5 },  /* vibrato 5, 1, 5 */
    /*   1 */ { AUDIO_CMD_DUTY, 2, 0, 0 },  /* duty 2 */
    /*   2 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 3 },  /* note_type 12, 12, 3 */
    /*   3 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /*   4 */ { AUDIO_CMD_NOTE, 1783, 3, 0 },  /* B_4 len=4 */
    /*   5 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /*   6 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 4 },  /* note_type 12, 12, 4 */
    /*   7 */ { AUDIO_CMD_NOTE, 1732, 9, 0 },  /* G#4 len=10 */
    /*   8 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 3 },  /* note_type 12, 12, 3 */
    /*   9 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  10 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  11 */ { AUDIO_CMD_NOTE, 1783, 3, 0 },  /* B_4 len=4 */
    /*  12 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /*  13 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  14 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  15 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  16 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 4 },  /* note_type 12, 12, 4 */
    /*  17 */ { AUDIO_CMD_NOTE, 1694, 9, 0 },  /* F#4 len=10 */
    /*  18 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 5 },  /* note_type 12, 12, 5 */
    /*  19 */ { AUDIO_CMD_DUTY, 3, 0, 0 },  /* duty 3 */
    /*  20 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  21 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /*  22 */ { AUDIO_CMD_NOTE, 1837, 7, 0 },  /* D#5 len=8 */
    /*  23 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /*  24 */ { AUDIO_CMD_NOTE, 1871, 3, 0 },  /* F#5 len=4 */
    /*  25 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 3 },  /* note_type 12, 12, 3 */
    /*  26 */ { AUDIO_CMD_DUTY, 2, 0, 0 },  /* duty 2 */
    /*  27 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /*  28 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /*  29 */ { AUDIO_CMD_NOTE, 1732, 3, 0 },  /* G#4 len=4 */
    /*  30 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 4 },  /* note_type 12, 12, 4 */
    /*  31 */ { AUDIO_CMD_NOTE, 1694, 9, 0 },  /* F#4 len=10 */
    /*  32 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 3 },  /* note_type 12, 12, 3 */
    /*  33 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  34 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  35 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /*  36 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  37 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  38 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  39 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /*  40 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  41 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 4 },  /* note_type 12, 12, 4 */
    /*  42 */ { AUDIO_CMD_NOTE, 1650, 5, 0 },  /* E_4 len=6 */
    /*  43 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 5 },  /* note_type 12, 12, 5 */
    /*  44 */ { AUDIO_CMD_DUTY, 3, 0, 0 },  /* duty 3 */
    /*  45 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  46 */ { AUDIO_CMD_NOTE, 1812, 3, 0 },  /* C#5 len=4 */
    /*  47 */ { AUDIO_CMD_NOTE, 1837, 3, 0 },  /* D#5 len=4 */
    /*  48 */ { AUDIO_CMD_NOTE, 1849, 5, 0 },  /* E_5 len=6 */
    /*  49 */ { AUDIO_CMD_NOTE, 1871, 5, 0 },  /* F#5 len=6 */
    /*  50 */ { AUDIO_CMD_NOTE, 1890, 3, 0 },  /* G#5 len=4 */
    /*  51 */ { AUDIO_CMD_DUTY, 2, 0, 0 },  /* duty 2 */
    /*  52 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 3 },  /* note_type 12, 12, 3 */
    /*  53 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /*  54 */ { AUDIO_CMD_NOTE, 1783, 3, 0 },  /* B_4 len=4 */
    /*  55 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /*  56 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 4 },  /* note_type 12, 12, 4 */
    /*  57 */ { AUDIO_CMD_NOTE, 1732, 9, 0 },  /* G#4 len=10 */
    /*  58 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 3 },  /* note_type 12, 12, 3 */
    /*  59 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  60 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  61 */ { AUDIO_CMD_NOTE, 1783, 3, 0 },  /* B_4 len=4 */
    /*  62 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /*  63 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  64 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  65 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  66 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 4 },  /* note_type 12, 12, 4 */
    /*  67 */ { AUDIO_CMD_NOTE, 1694, 9, 0 },  /* F#4 len=10 */
    /*  68 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 5 },  /* note_type 12, 12, 5 */
    /*  69 */ { AUDIO_CMD_DUTY, 3, 0, 0 },  /* duty 3 */
    /*  70 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  71 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /*  72 */ { AUDIO_CMD_NOTE, 1837, 7, 0 },  /* D#5 len=8 */
    /*  73 */ { AUDIO_CMD_NOTE, 1849, 3, 0 },  /* E_5 len=4 */
    /*  74 */ { AUDIO_CMD_NOTE, 1871, 3, 0 },  /* F#5 len=4 */
    /*  75 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 3 },  /* note_type 12, 12, 3 */
    /*  76 */ { AUDIO_CMD_DUTY, 2, 0, 0 },  /* duty 2 */
    /*  77 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /*  78 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /*  79 */ { AUDIO_CMD_NOTE, 1732, 3, 0 },  /* G#4 len=4 */
    /*  80 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 4 },  /* note_type 12, 12, 4 */
    /*  81 */ { AUDIO_CMD_NOTE, 1694, 9, 0 },  /* F#4 len=10 */
    /*  82 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 3 },  /* note_type 12, 12, 3 */
    /*  83 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  84 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  85 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /*  86 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  87 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  88 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  89 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /*  90 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  91 */ { AUDIO_CMD_NOTE_TYPE, 12, 12, 4 },  /* note_type 12, 12, 4 */
    /*  92 */ { AUDIO_CMD_NOTE, 1650, 13, 0 },  /* E_4 len=14 */
    /*  93 */ { AUDIO_CMD_DUTY, 3, 0, 0 },  /* duty 3 */
    /*  94 */ { AUDIO_CMD_OCTAVE, 3, 0, 0 },  /* octave 3 */
    /*  95 */ { AUDIO_CMD_NOTE, 1849, 5, 0 },  /* E_5 len=6 */
    /*  96 */ { AUDIO_CMD_NOTE, 1871, 5, 0 },  /* F#5 len=6 */
    /*  97 */ { AUDIO_CMD_NOTE, 1890, 3, 0 },  /* G#5 len=4 */
    /*  98 */ { AUDIO_CMD_NOTE_TYPE, 12, 11, 7 },  /* note_type 12, 11, 7 */
    /*  99 */ { AUDIO_CMD_DUTY, 2, 0, 0 },  /* duty 2 */
    /* 100 */ { AUDIO_CMD_VIBRATO, 8, 1, 7 },  /* vibrato 8, 1, 7 */
    /* 101 */ { AUDIO_CMD_OCTAVE, 5, 0, 0 },  /* octave 5 */
    /* 102 */ { AUDIO_CMD_NOTE, 1102, 11, 0 },  /* C#3 len=12 */
    /* 103 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /* 104 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /* 105 */ { AUDIO_CMD_OCTAVE, 5, 0, 0 },  /* octave 5 */
    /* 106 */ { AUDIO_CMD_NOTE, 1253, 7, 0 },  /* E_3 len=8 */
    /* 107 */ { AUDIO_CMD_NOTE, 1339, 1, 0 },  /* F#3 len=2 */
    /* 108 */ { AUDIO_CMD_NOTE, 1253, 1, 0 },  /* E_3 len=2 */
    /* 109 */ { AUDIO_CMD_NOTE, 1205, 1, 0 },  /* D#3 len=2 */
    /* 110 */ { AUDIO_CMD_NOTE, 1102, 1, 0 },  /* C#3 len=2 */
    /* 111 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /* 112 */ { AUDIO_CMD_NOTE, 1783, 11, 0 },  /* B_4 len=12 */
    /* 113 */ { AUDIO_CMD_NOTE, 1732, 3, 0 },  /* G#4 len=4 */
    /* 114 */ { AUDIO_CMD_NOTE, 1783, 15, 0 },  /* B_4 len=16 */
    /* 115 */ { AUDIO_CMD_NOTE, 1694, 11, 0 },  /* F#4 len=12 */
    /* 116 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 117 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /* 118 */ { AUDIO_CMD_NOTE, 1783, 3, 0 },  /* B_4 len=4 */
    /* 119 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /* 120 */ { AUDIO_CMD_NOTE, 1732, 3, 0 },  /* G#4 len=4 */
    /* 121 */ { AUDIO_CMD_NOTE, 1694, 3, 0 },  /* F#4 len=4 */
    /* 122 */ { AUDIO_CMD_NOTE, 1732, 11, 0 },  /* G#4 len=12 */
    /* 123 */ { AUDIO_CMD_NOTE, 1650, 3, 0 },  /* E_4 len=4 */
    /* 124 */ { AUDIO_CMD_NOTE, 1783, 15, 0 },  /* B_4 len=16 */
    /* 125 */ { AUDIO_CMD_OCTAVE, 5, 0, 0 },  /* octave 5 */
    /* 126 */ { AUDIO_CMD_NOTE, 1102, 11, 0 },  /* C#3 len=12 */
    /* 127 */ { AUDIO_CMD_NOTE, 1205, 1, 0 },  /* D#3 len=2 */
    /* 128 */ { AUDIO_CMD_NOTE, 1253, 1, 0 },  /* E_3 len=2 */
    /* 129 */ { AUDIO_CMD_NOTE, 1339, 3, 0 },  /* F#3 len=4 */
    /* 130 */ { AUDIO_CMD_NOTE, 1253, 3, 0 },  /* E_3 len=4 */
    /* 131 */ { AUDIO_CMD_NOTE, 1205, 3, 0 },  /* D#3 len=4 */
    /* 132 */ { AUDIO_CMD_NOTE, 1102, 3, 0 },  /* C#3 len=4 */
    /* 133 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /* 134 */ { AUDIO_CMD_NOTE, 1783, 11, 0 },  /* B_4 len=12 */
    /* 135 */ { AUDIO_CMD_OCTAVE, 5, 0, 0 },  /* octave 5 */
    /* 136 */ { AUDIO_CMD_NOTE, 1102, 1, 0 },  /* C#3 len=2 */
    /* 137 */ { AUDIO_CMD_NOTE, 1205, 1, 0 },  /* D#3 len=2 */
    /* 138 */ { AUDIO_CMD_NOTE, 1102, 3, 0 },  /* C#3 len=4 */
    /* 139 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /* 140 */ { AUDIO_CMD_NOTE, 1783, 3, 0 },  /* B_4 len=4 */
    /* 141 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /* 142 */ { AUDIO_CMD_NOTE, 1732, 3, 0 },  /* G#4 len=4 */
    /* 143 */ { AUDIO_CMD_NOTE, 1750, 11, 0 },  /* A_4 len=12 */
    /* 144 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /* 145 */ { AUDIO_CMD_OCTAVE, 5, 0, 0 },  /* octave 5 */
    /* 146 */ { AUDIO_CMD_NOTE, 1046, 1, 0 },  /* C_3 len=2 */
    /* 147 */ { AUDIO_CMD_NOTE, 1046, 3, 0 },  /* C_3 len=4 */
    /* 148 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /* 149 */ { AUDIO_CMD_NOTE, 1783, 3, 0 },  /* B_4 len=4 */
    /* 150 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /* 151 */ { AUDIO_CMD_NOTE, 1694, 3, 0 },  /* F#4 len=4 */
    /* 152 */ { AUDIO_CMD_NOTE_TYPE, 12, 11, 7 },  /* note_type 12, 11, 7 */
    /* 153 */ { AUDIO_CMD_NOTE, 1750, 7, 0 },  /* A_4 len=8 */
    /* 154 */ { AUDIO_CMD_OCTAVE, 5, 0, 0 },  /* octave 5 */
    /* 155 */ { AUDIO_CMD_NOTE, 1046, 7, 0 },  /* C_3 len=8 */
    /* 156 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /* 157 */ { AUDIO_CMD_NOTE, 1783, 13, 0 },  /* B_4 len=14 */
    /* 158 */ { AUDIO_CMD_NOTE_TYPE, 12, 8, 4 },  /* note_type 12, 8, 4 */
    /* 159 */ { AUDIO_CMD_NOTE, 1732, 0, 0 },  /* G#4 len=1 */
    /* 160 */ { AUDIO_CMD_NOTE_TYPE, 12, 10, 4 },  /* note_type 12, 10, 4 */
    /* 161 */ { AUDIO_CMD_NOTE, 1750, 0, 0 },  /* A_4 len=1 */
    /* 162 */ { AUDIO_CMD_LOOP, 0, 0, 0 },  /* loop 0x -> mainloop */
};

static const AudioCommand s_viridian_wave[] = {
    /*   0 */ { AUDIO_CMD_NOTE_TYPE, 12, 1, 1 },  /* note_type 12, 1, 1 */
    /*   1 */ { AUDIO_CMD_VIBRATO, 0, 0, 0 },  /* vibrato 0, 0, 0 */
    /*   2 */ { AUDIO_CMD_OCTAVE, 4, 0, 0 },  /* octave 4 */
    /*   3 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*   4 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*   5 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*   6 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*   7 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*   8 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*   9 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  10 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  11 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  12 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  13 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  14 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  15 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  16 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  17 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  18 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  19 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  20 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  21 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  22 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  23 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  24 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  25 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  26 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  27 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  28 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  29 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  30 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  31 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  32 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  33 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  34 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  35 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /*  36 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /*  37 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  38 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  39 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  40 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /*  41 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  42 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  43 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  44 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /*  45 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  46 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  47 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  48 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /*  49 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  50 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  51 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  52 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  53 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  54 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  55 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  56 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  57 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  58 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  59 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  60 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  61 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  62 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  63 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  64 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  65 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  66 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  67 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /*  68 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  69 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  70 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  71 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  72 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  73 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  74 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  75 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  76 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  77 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  78 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  79 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  80 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /*  81 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  82 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  83 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  84 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  85 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  86 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  87 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  88 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  89 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  90 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  91 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  92 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  93 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  94 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  95 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /*  96 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /*  97 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /*  98 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /*  99 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /* 100 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /* 101 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 102 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 103 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 104 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /* 105 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 106 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 107 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 108 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /* 109 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 110 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 111 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 112 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /* 113 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 114 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 115 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 116 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 117 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 118 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 119 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 120 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 121 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 122 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /* 123 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 124 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 125 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 126 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 127 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 128 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 129 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /* 130 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 131 */ { AUDIO_CMD_VIBRATO, 8, 2, 5 },  /* vibrato 8, 2, 5 */
    /* 132 */ { AUDIO_CMD_NOTE, 1750, 7, 0 },  /* A_4 len=8 */
    /* 133 */ { AUDIO_CMD_NOTE, 1650, 7, 0 },  /* E_4 len=8 */
    /* 134 */ { AUDIO_CMD_NOTE, 1750, 7, 0 },  /* A_4 len=8 */
    /* 135 */ { AUDIO_CMD_NOTE, 1694, 7, 0 },  /* F#4 len=8 */
    /* 136 */ { AUDIO_CMD_NOTE, 1732, 7, 0 },  /* G#4 len=8 */
    /* 137 */ { AUDIO_CMD_NOTE, 1650, 7, 0 },  /* E_4 len=8 */
    /* 138 */ { AUDIO_CMD_NOTE, 1732, 11, 0 },  /* G#4 len=12 */
    /* 139 */ { AUDIO_CMD_NOTE, 1650, 3, 0 },  /* E_4 len=4 */
    /* 140 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 141 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 142 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /* 143 */ { AUDIO_CMD_NOTE, 1650, 3, 0 },  /* E_4 len=4 */
    /* 144 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 145 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /* 146 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 147 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 148 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 149 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /* 150 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /* 151 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 152 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /* 153 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 154 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 155 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 156 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 157 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 158 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 159 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 160 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 161 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 162 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 163 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 164 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 165 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 166 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 167 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /* 168 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /* 169 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 170 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 171 */ { AUDIO_CMD_NOTE, 1750, 7, 0 },  /* A_4 len=8 */
    /* 172 */ { AUDIO_CMD_NOTE, 1650, 7, 0 },  /* E_4 len=8 */
    /* 173 */ { AUDIO_CMD_NOTE, 1750, 7, 0 },  /* A_4 len=8 */
    /* 174 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /* 175 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /* 176 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 177 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 178 */ { AUDIO_CMD_NOTE, 1732, 7, 0 },  /* G#4 len=8 */
    /* 179 */ { AUDIO_CMD_NOTE, 1650, 7, 0 },  /* E_4 len=8 */
    /* 180 */ { AUDIO_CMD_NOTE, 1783, 3, 0 },  /* B_4 len=4 */
    /* 181 */ { AUDIO_CMD_NOTE, 1650, 3, 0 },  /* E_4 len=4 */
    /* 182 */ { AUDIO_CMD_NOTE, 1694, 3, 0 },  /* F#4 len=4 */
    /* 183 */ { AUDIO_CMD_NOTE, 1732, 3, 0 },  /* G#4 len=4 */
    /* 184 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 185 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /* 186 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 187 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 188 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 189 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 190 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /* 191 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /* 192 */ { AUDIO_CMD_NOTE, 1750, 3, 0 },  /* A_4 len=4 */
    /* 193 */ { AUDIO_CMD_NOTE, 1732, 3, 0 },  /* G#4 len=4 */
    /* 194 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 195 */ { AUDIO_CMD_NOTE, 1627, 1, 0 },  /* D#4 len=2 */
    /* 196 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /* 197 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 198 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 199 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 200 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 201 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 202 */ { AUDIO_CMD_REST, 0, 1, 0 },  /* rest 2 */
    /* 203 */ { AUDIO_CMD_NOTE, 1650, 1, 0 },  /* E_4 len=2 */
    /* 204 */ { AUDIO_CMD_NOTE, 1694, 0, 0 },  /* F#4 len=1 */
    /* 205 */ { AUDIO_CMD_NOTE, 1732, 0, 0 },  /* G#4 len=1 */
    /* 206 */ { AUDIO_CMD_NOTE, 1650, 0, 0 },  /* E_4 len=1 */
    /* 207 */ { AUDIO_CMD_NOTE, 1694, 0, 0 },  /* F#4 len=1 */
    /* 208 */ { AUDIO_CMD_NOTE, 1732, 3, 0 },  /* G#4 len=4 */
    /* 209 */ { AUDIO_CMD_NOTE, 1783, 1, 0 },  /* B_4 len=2 */
    /* 210 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /* 211 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 212 */ { AUDIO_CMD_NOTE, 1750, 1, 0 },  /* A_4 len=2 */
    /* 213 */ { AUDIO_CMD_NOTE, 1732, 1, 0 },  /* G#4 len=2 */
    /* 214 */ { AUDIO_CMD_NOTE, 1694, 1, 0 },  /* F#4 len=2 */
    /* 215 */ { AUDIO_CMD_LOOP, 0, 1, 0 },  /* loop 0x -> mainloop */
};

static const AudioCommand s_viridian_noise[] = {
    /*   0 */ { AUDIO_CMD_NOTE_TYPE, 12, 0, 0 },  /* drum_speed 12 */
    /*   1 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*   2 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*   3 */ { AUDIO_CMD_DRUM, 8, 3, 0 },  /* drum 8, len=4 */
    /*   4 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*   5 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*   6 */ { AUDIO_CMD_DRUM, 8, 1, 0 },  /* drum 8, len=2 */
    /*   7 */ { AUDIO_CMD_DRUM, 8, 1, 0 },  /* drum 8, len=2 */
    /*   8 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*   9 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  10 */ { AUDIO_CMD_DRUM, 8, 3, 0 },  /* drum 8, len=4 */
    /*  11 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  12 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  13 */ { AUDIO_CMD_DRUM, 8, 1, 0 },  /* drum 8, len=2 */
    /*  14 */ { AUDIO_CMD_DRUM, 8, 1, 0 },  /* drum 8, len=2 */
    /*  15 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  16 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  17 */ { AUDIO_CMD_DRUM, 8, 3, 0 },  /* drum 8, len=4 */
    /*  18 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  19 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  20 */ { AUDIO_CMD_DRUM, 8, 3, 0 },  /* drum 8, len=4 */
    /*  21 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  22 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  23 */ { AUDIO_CMD_DRUM, 8, 3, 0 },  /* drum 8, len=4 */
    /*  24 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  25 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  26 */ { AUDIO_CMD_DRUM, 8, 3, 0 },  /* drum 8, len=4 */
    /*  27 */ { AUDIO_CMD_LOOP, 2, 8, 0 },  /* loop 2x -> loop1 */
    /*  28 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  29 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  30 */ { AUDIO_CMD_DRUM, 8, 3, 0 },  /* drum 8, len=4 */
    /*  31 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  32 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  33 */ { AUDIO_CMD_DRUM, 8, 1, 0 },  /* drum 8, len=2 */
    /*  34 */ { AUDIO_CMD_DRUM, 8, 1, 0 },  /* drum 8, len=2 */
    /*  35 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  36 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  37 */ { AUDIO_CMD_DRUM, 7, 3, 0 },  /* drum 7, len=4 */
    /*  38 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  39 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  40 */ { AUDIO_CMD_DRUM, 7, 1, 0 },  /* drum 7, len=2 */
    /*  41 */ { AUDIO_CMD_DRUM, 6, 1, 0 },  /* drum 6, len=2 */
    /*  42 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  43 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  44 */ { AUDIO_CMD_DRUM, 7, 3, 0 },  /* drum 7, len=4 */
    /*  45 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  46 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  47 */ { AUDIO_CMD_DRUM, 7, 3, 0 },  /* drum 7, len=4 */
    /*  48 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  49 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  50 */ { AUDIO_CMD_DRUM, 7, 3, 0 },  /* drum 7, len=4 */
    /*  51 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  52 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  53 */ { AUDIO_CMD_DRUM, 7, 1, 0 },  /* drum 7, len=2 */
    /*  54 */ { AUDIO_CMD_DRUM, 7, 1, 0 },  /* drum 7, len=2 */
    /*  55 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  56 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  57 */ { AUDIO_CMD_DRUM, 7, 3, 0 },  /* drum 7, len=4 */
    /*  58 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  59 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  60 */ { AUDIO_CMD_DRUM, 7, 1, 0 },  /* drum 7, len=2 */
    /*  61 */ { AUDIO_CMD_DRUM, 6, 1, 0 },  /* drum 6, len=2 */
    /*  62 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  63 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  64 */ { AUDIO_CMD_DRUM, 7, 3, 0 },  /* drum 7, len=4 */
    /*  65 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  66 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  67 */ { AUDIO_CMD_DRUM, 7, 3, 0 },  /* drum 7, len=4 */
    /*  68 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  69 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  70 */ { AUDIO_CMD_DRUM, 7, 1, 0 },  /* drum 7, len=2 */
    /*  71 */ { AUDIO_CMD_DRUM, 7, 1, 0 },  /* drum 7, len=2 */
    /*  72 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  73 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  74 */ { AUDIO_CMD_DRUM, 7, 3, 0 },  /* drum 7, len=4 */
    /*  75 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  76 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  77 */ { AUDIO_CMD_DRUM, 7, 3, 0 },  /* drum 7, len=4 */
    /*  78 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  79 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  80 */ { AUDIO_CMD_DRUM, 7, 1, 0 },  /* drum 7, len=2 */
    /*  81 */ { AUDIO_CMD_DRUM, 6, 1, 0 },  /* drum 6, len=2 */
    /*  82 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  83 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  84 */ { AUDIO_CMD_DRUM, 6, 3, 0 },  /* drum 6, len=4 */
    /*  85 */ { AUDIO_CMD_DRUM, 6, 5, 0 },  /* drum 6, len=6 */
    /*  86 */ { AUDIO_CMD_DRUM, 8, 5, 0 },  /* drum 8, len=6 */
    /*  87 */ { AUDIO_CMD_DRUM, 8, 3, 0 },  /* drum 8, len=4 */
    /*  88 */ { AUDIO_CMD_LOOP, 0, 0, 0 },  /* loop 0x -> mainloop */
};

const AudioTrackData g_audio_viridian_city = {
    .channels = {
        { .commands = s_viridian_lead, .count = sizeof(s_viridian_lead) / sizeof(s_viridian_lead[0]), .loop_start = 3 },
        { .commands = s_viridian_sub, .count = sizeof(s_viridian_sub) / sizeof(s_viridian_sub[0]), .loop_start = 0 },
        { .commands = s_viridian_wave, .count = sizeof(s_viridian_wave) / sizeof(s_viridian_wave[0]), .loop_start = 1 },
        { .commands = s_viridian_noise, .count = sizeof(s_viridian_noise) / sizeof(s_viridian_noise[0]), .loop_start = 0 },
    },
    .wave_data = NULL,
};
