#pragma once
// Use libgba's types as the single source of truth.
// Our code uses u8/u16/u32/s8/s16/s32/bool8 — all provided by gba_types.h.
#include <gba_types.h>

typedef u8  bool8;
typedef u16 bool16;
typedef u32 bool32;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL  ((void*)0)
#endif

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))
