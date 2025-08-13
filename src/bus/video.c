/*
 * SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <string.h>
#include "cartridge.h"
#include "processor.h"
#include "video.h"

typedef enum {
    CGBL_COLOR_WHITE = 0,
    CGBL_COLOR_GREY_LIGHT,
    CGBL_COLOR_GREY_DARK,
    CGBL_COLOR_BLACK,
    CGBL_COLOR_MAX,
} cgbl_color_e;

typedef enum {
    CGBL_STATE_HBLANK = 0,
    CGBL_STATE_VBLANK,
    CGBL_STATE_SEARCH,
    CGBL_STATE_TRANSFER,
    CGBL_STATE_MAX,
} cgbl_state_e;

static const struct {
    uint8_t hash;
    char disambiguation;
    uint16_t background[CGBL_COLOR_MAX];
    uint16_t object[2][CGBL_COLOR_MAX];
} PALETTE[] = {
    { 0x01, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0x0C, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0x0D, 'E', { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { 0x7FFF, 0x6E31, 0x454A, 0x0000 }, }, },
    { 0x0D, 'R', { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0x10, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0x14, '\0', { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x15, '\0', { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, }, },
    { 0x16, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0x17, '\0', { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0x18, 'I', { 0x7FFF, 0x1BEF, 0x6180, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x18, 'K', { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0x19, '\0', { 0x7FFF, 0x027F, 0x001F, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x1D, '\0', { 0x7E74, 0x03FF, 0x0180, 0x0000, }, { { 0x299F, 0x001A, 0x000C, 0x0000, }, { 0x299F, 0x001A, 0x000C, 0x0000, }, }, },
    { 0x27, 'B', { 0x7E74, 0x03FF, 0x0180, 0x0000, }, { { 0x299F, 0x001A, 0x000C, 0x0000, }, { 0x7C00, 0x7FFF, 0x3FFF, 0x7E00, }, }, },
    { 0x27, 'N', { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0x28, 'A', { 0x0000, 0x4200, 0x037F, 0x7FFF, }, { { 0x0000, 0x4200, 0x037F, 0x7FFF, }, { 0x0000, 0x4200, 0x037F, 0x7FFF, }, }, },
    { 0x28, 'F', { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x29, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0x34, '\0', { 0x7FFF, 0x03EF, 0x01D6, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x35, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0x36, '\0', { 0x036A, 0x021F, 0x03FF, 0x7FFF, }, { { 0x7FFF, 0x7FFF, 0x7E8C, 0x7C00, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x39, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0x3C, '\0', { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x3D, '\0', { 0x7FFF, 0x03EA, 0x011F, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x3E, '\0', { 0x7FFF, 0x027F, 0x001F, 0x0000, }, { { 0x7FFF, 0x027F, 0x001F, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0x3F, '\0', { 0x7FFF, 0x1BEF, 0x6180, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x43, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0x46, 'E', { 0x7ED6, 0x4BFF, 0x2175, 0x0000, }, { { 0x0000, 0x7FFF, 0x421F, 0x1CF2, }, { 0x0000, 0x7FFF, 0x421F, 0x1CF2, }, }, },
    { 0x46, 'R', { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { { 0x03FF, 0x001F, 0x000C, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0x49, '\0', { 0x7E74, 0x03FF, 0x0180, 0x0000, }, { { 0x299F, 0x001A, 0x000C, 0x0000, }, { 0x7C00, 0x7FFF, 0x3FFF, 0x7E00, }, }, },
    { 0x4B, '\0', { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x4E, '\0', { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x3FFF, 0x7E00, 0x001F, }, }, },
    { 0x52, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0x58, '\0', { 0x7FFF, 0x5294, 0x294A, 0x0000, }, { { 0x7FFF, 0x5294, 0x294A, 0x0000, }, { 0x7FFF, 0x5294, 0x294A, 0x0000, }, }, },
    { 0x59, '\0', { 0x7FFF, 0x42B5, 0x3DC8, 0x0000, }, { { 0x7FFF, 0x01DF, 0x0112, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0x5C, '\0', { 0x7E74, 0x03FF, 0x0180, 0x0000, }, { { 0x299F, 0x001A, 0x000C, 0x0000, }, { 0x7C00, 0x7FFF, 0x3FFF, 0x7E00, }, }, },
    { 0x5D, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0x61, 'A', { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0x61, 'E', { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0x66, 'E', { 0x7FFF, 0x03EF, 0x01D6, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x66, 'L', { 0x7FFF, 0x1BEF, 0x6180, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x67, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0x68, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0x69, '\0', { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0x6A, 'I', { 0x7FFF, 0x03EA, 0x011F, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x6A, 'K', { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0x6B, '\0', { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0x6D, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0x6F, '\0', { 0x7FFF, 0x033F, 0x0193, 0x0000, }, { { 0x7FFF, 0x033F, 0x0193, 0x0000, }, { 0x7FFF, 0x033F, 0x0193, 0x0000, }, }, },
    { 0x70, '\0', { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { { 0x7FFF, 0x03E0, 0x0206, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0x71, '\0', { 0x7FFF, 0x027F, 0x001F, 0x0000, }, { { 0x7FFF, 0x027F, 0x001F, 0x0000, }, { 0x7FFF, 0x027F, 0x001F, 0x0000, }, }, },
    { 0x75, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0x86, '\0', { 0x4FFF, 0x7ED2, 0x3A4C, 0x1CE0, }, { { 0x231F, 0x035F, 0x00F2, 0x0009, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x88, '\0', { 0x7E74, 0x03FF, 0x0180, 0x0000, }, { { 0x7E74, 0x03FF, 0x0180, 0x0000, }, { 0x7E74, 0x03FF, 0x0180, 0x0000, }, }, },
    { 0x8B, '\0', { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0x8C, '\0', { 0x4FFF, 0x7ED2, 0x3A4C, 0x1CE0, }, { { 0x4FFF, 0x7ED2, 0x3A4C, 0x1CE0, }, { 0x4FFF, 0x7ED2, 0x3A4C, 0x1CE0, }, }, },
    { 0x90, '\0', { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x92, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0x95, '\0', { 0x7FFF, 0x03EA, 0x011F, 0x0000, }, { { 0x7FFF, 0x03EA, 0x011F, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0x97, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0x99, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0x9A, '\0', { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0x9C, '\0', { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { 0x231F, 0x035F, 0x00F2, 0x0009, }, }, },
    { 0x9D, '\0', { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0xA2, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0xA5, 'A', { 0x0000, 0x4200, 0x037F, 0x7FFF, }, { { 0x0000, 0x4200, 0x037F, 0x7FFF, }, { 0x0000, 0x4200, 0x037F, 0x7FFF, }, }, },
    { 0xA5, 'R', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0xA8, '\0', { 0x4FFF, 0x7ED2, 0x3A4C, 0x1CE0, }, { { 0x231F, 0x035F, 0x00F2, 0x0009, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0xAA, '\0', { 0x7FFF, 0x1BEF, 0x6180, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x1BEF, 0x6180, 0x0000, }, }, },
    { 0xB3, 'B', { 0x7E74, 0x03FF, 0x0180, 0x0000, }, { { 0x299F, 0x001A, 0x000C, 0x0000, }, { 0x7C00, 0x7FFF, 0x3FFF, 0x7E00, }, }, },
    { 0xB3, 'R', { 0x7FFF, 0x03EA, 0x011F, 0x0000, }, { { 0x7FFF, 0x03EA, 0x011F, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0xB3, 'U', { 0x7FFF, 0x42B5, 0x3DC8, 0x0000, }, { { 0x7FFF, 0x01DF, 0x0112, 0x0000, }, { 0x7FFF, 0x01DF, 0x0112, 0x0000, }, }, },
    { 0xB7, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0xBD, '\0', { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0xBF, ' ', { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0xBF, 'C', { 0x03ED, 0x7FFF, 0x255F, 0x0000, }, { { 0x7FFF, 0x7FFF, 0x7E8C, 0x7C00, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0xC6, ' ', { 0x7FFF, 0x1BEF, 0x6180, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0xC6, 'A', { 0x7FFF, 0x42B5, 0x3DC8, 0x0000, }, { { 0x7FFF, 0x01DF, 0x0112, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0xC9, '\0', { 0x67FF, 0x77AC, 0x1A13, 0x2D6B, }, { { 0x7FFF, 0x01DF, 0x0112, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0xCE, '\0', { 0x03ED, 0x7FFF, 0x255F, 0x0000, }, { { 0x7FFF, 0x7FFF, 0x7E8C, 0x7C00, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0xD1, '\0', { 0x03ED, 0x7FFF, 0x255F, 0x0000, }, { { 0x7FFF, 0x7FFF, 0x7E8C, 0x7C00, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0xD3, 'I', { 0x7FFF, 0x42B5, 0x3DC8, 0x0000, }, { { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0xD3, 'R', { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x6E31, 0x454A, 0x0000, }, }, },
    { 0xDB, '\0', { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, }, },
    { 0xE0, '\0', { 0x7FFF, 0x027F, 0x001F, 0x0000, }, { { 0x7FFF, 0x027F, 0x001F, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0xE8, '\0', { 0x0000, 0x4200, 0x037F, 0x7FFF, }, { { 0x0000, 0x4200, 0x037F, 0x7FFF, }, { 0x0000, 0x4200, 0x037F, 0x7FFF, }, }, },
    { 0xF0, '\0', { 0x03ED, 0x7FFF, 0x255F, 0x0000, }, { { 0x7FFF, 0x7FFF, 0x7E8C, 0x7C00, }, { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, }, },
    { 0xF2, '\0', { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { { 0x7FFF, 0x03FF, 0x001F, 0x0000, }, { 0x7FFF, 0x7EEB, 0x001F, 0x7C00, }, }, },
    { 0xF4, ' ', { 0x7FFF, 0x03EF, 0x01D6, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
    { 0xF4, '-', { 0x7FFF, 0x1BEF, 0x6180, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0xF6, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, }, },
    { 0xF7, '\0', { 0x7FFF, 0x32BF, 0x00D0, 0x0000, }, { { 0x7FFF, 0x1BEF, 0x0200, 0x0000, }, { 0x7FFF, 0x7E8C, 0x7C00, 0x0000, }, }, },
    { 0xFF, '\0', { 0x7FFF, 0x027F, 0x001F, 0x0000, }, { { 0x7FFF, 0x027F, 0x001F, 0x0000, }, { 0x7FFF, 0x027F, 0x001F, 0x0000, }, }, },
    { 0x00, '\0', { 0x7FFF, 0x1BEF, 0x6180, 0x0000, }, { { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, { 0x7FFF, 0x421F, 0x1CF2, 0x0000, }, }, },
};

typedef struct {
    uint8_t palette : 3;
    uint8_t bank : 1;
    uint8_t : 1;
    uint8_t flip_x : 1;
    uint8_t flip_y : 1;
    uint8_t priority : 1;
} cgbl_background_t;

typedef struct {
    uint8_t y;
    uint8_t x;
    uint8_t id;
    struct {
        uint8_t palette : 3;
        uint8_t bank : 1;
        uint8_t palette_dmg : 1;
        uint8_t flip_x : 1;
        uint8_t flip_y : 1;
        uint8_t priority : 1;
    } attribute;
} cgbl_object_t;

typedef struct {
    uint8_t index;
    const cgbl_object_t *object;
} cgbl_object_entry_t;

typedef union {
    struct {
        uint8_t white : 2;
        uint8_t grey_light : 2;
        uint8_t grey_dark : 2;
        uint8_t black : 2;
    };
    uint8_t raw;
} cgbl_palette_t;

static struct {
    bool shown;
    struct {
        union {
            struct {
                uint8_t address : 6;
                uint8_t : 1;
                uint8_t increment : 1;
            };
            uint8_t raw;
        } control;
        struct {
            uint16_t cgb[8][CGBL_COLOR_MAX];
            const uint16_t *dmg;
            cgbl_palette_t palette;
        } color;
    } background;
    union {
        struct {
            uint8_t background_enabled : 1;
            uint8_t object_enabled : 1;
            uint8_t object_size : 1;
            uint8_t background_map : 1;
            uint8_t background_data : 1;
            uint8_t window_enabled : 1;
            uint8_t window_map : 1;
            uint8_t enabled : 1;
        };
        uint8_t raw;
    } control;
    struct {
        uint8_t coincidence;
        uint16_t x;
        uint8_t y;
    } line;
    struct {
        cgbl_object_t ram[CGBL_VIDEO_RAM_OBJECT_WIDTH];
        union {
            struct {
                uint8_t address : 6;
                uint8_t : 1;
                uint8_t increment : 1;
            };
            uint8_t raw;
        } control;
        struct {
            uint16_t cgb[8][CGBL_COLOR_MAX];
            const uint16_t *dmg[2];
            cgbl_palette_t palette[2];
        } color;
        struct {
            uint8_t count;
            cgbl_object_entry_t entry[10];
        } shown;
    } object;
    struct {
        bool priority[CGBL_VIDEO_HEIGHT][CGBL_VIDEO_WIDTH];
        uint16_t data[CGBL_VIDEO_HEIGHT][CGBL_VIDEO_WIDTH];
        cgbl_color_e color[CGBL_VIDEO_HEIGHT][CGBL_VIDEO_WIDTH];
    } pixel;
    struct {
        uint8_t data[2][CGBL_VIDEO_RAM_WIDTH];
        union {
            struct {
                uint8_t select : 1;
            };
            uint8_t raw;
        } bank;
    } ram;
    struct {
        uint8_t x;
        uint8_t y;
    } scroll;
    union {
        struct {
            uint8_t state : 2;
            uint8_t coincidence : 1;
            uint8_t interrupt_hblank : 1;
            uint8_t interrupt_vblank : 1;
            uint8_t interrupt_search : 1;
            uint8_t interrupt_coincidence : 1;
        };
        uint8_t raw;
    } status;
    struct {
        bool active;
        uint16_t offset;
        union {
            struct {
                uint8_t length : 7;
                uint8_t hblank : 1;
            };
            uint8_t raw;
        } control;
        union {
            struct {
                uint8_t low;
                uint8_t high;
            };
            uint16_t word;
        } destination;
        union {
            struct {
                uint8_t low;
                uint8_t high;
            };
            uint16_t word;
        } source;
        struct {
            uint8_t address;
            uint8_t delay;
            uint16_t destination;
            uint16_t source;
        } object;
    } transfer;
    struct {
        uint8_t counter;
        uint8_t x;
        uint8_t y;
    } window;
} video = {};

static cgbl_color_e cgbl_video_cgb_background_color(cgbl_background_t **const background, uint8_t map, uint8_t x, uint8_t y)
{
    uint16_t address = (map ? 0x1C00 : 0x1800) + (32 * ((y / 8) & 31)) + ((x / 8) & 31);
    *background = (cgbl_background_t *)&video.ram.data[1][address];
    if ((*background)->flip_x)
    {
        x = 7 - x;
    }
    if ((*background)->flip_y)
    {
        y = 7 - y;
    }
    if (video.control.background_data)
    {
        address = (16 * video.ram.data[0][address]) + (2 * (y & 7));
    }
    else
    {
        address = (16 * (int8_t)video.ram.data[0][address]) + (2 * (y & 7)) + 0x1000;
    }
    x = 1 << (7 - (x & 7));
    return ((video.ram.data[(*background)->bank][address + 1] & x) ? 2 : 0) + ((video.ram.data[(*background)->bank][address] & x) ? 1 : 0);
}

static void cgbl_video_cgb_background_render(void)
{
    for (uint8_t index = 0; index < CGBL_VIDEO_WIDTH; ++index)
    {
        cgbl_background_t *background = NULL;
        cgbl_color_e color = CGBL_COLOR_WHITE;
        uint8_t map = 0, x = index, y = video.line.y;
        if (video.control.window_enabled && (video.window.x <= 166) && (video.window.y <= 143) && ((video.window.x - 7) <= x) && (video.window.y <= y))
        {
            map = video.control.window_map;
            x -= (video.window.x - 7);
            y = video.window.counter - video.window.y;
        }
        else
        {
            map = video.control.background_map;
            x += video.scroll.x;
            y += video.scroll.y;
        }
        color = cgbl_video_cgb_background_color(&background, map, x, y);
        video.pixel.color[video.line.y][index] = color;
        video.pixel.data[video.line.y][index] = video.background.color.cgb[background->palette][color];
        video.pixel.priority[video.line.y][index] = background->priority;
    }
}

static cgbl_color_e cgbl_video_cgb_object_color(const cgbl_object_t *object, uint8_t x, uint8_t y)
{
    uint16_t address = 0;
    uint8_t id = object->id;
    if (video.control.object_size)
    {
        if (object->attribute.flip_y)
        {
            if ((y - (object->y - 16)) < 8)
            {
                id |= 1;
            }
            else
            {
                id &= 0xFE;
            }
        }
        else if ((y - (object->y - 16)) < 8)
        {
            id &= 0xFE;
        }
        else
        {
            id |= 1;
        }
    }
    y = (y - object->y) & 7;
    if (object->attribute.flip_x)
    {
        x = 7 - x;
    }
    if (object->attribute.flip_y)
    {
        y = 7 - y;
    }
    address = (16 * id) + (2 * y);
    x = 1 << (7 - x);
    return ((video.ram.data[object->attribute.bank][address + 1] & x) ? 2 : 0) + ((video.ram.data[object->attribute.bank][address] & x) ? 1 : 0);
}

static void cgbl_video_cgb_object_render(void)
{
    if (video.object.shown.count)
    {
        uint8_t y = video.line.y;
        for (int32_t index = (video.object.shown.count - 1); index >= 0; index--)
        {
            const cgbl_object_t *object = video.object.shown.entry[index].object;
            for (uint8_t x = 0; x < 8; ++x)
            {
                cgbl_color_e color = CGBL_COLOR_WHITE;
                if ((object->x < 8) && (x < (8 - object->x)))
                {
                    continue;
                }
                else if ((object->x >= 160) && (x >= (8 - (object->x - 160))))
                {
                    break;
                }
                color = cgbl_video_cgb_object_color(object, x, y);
                if (color && (!video.control.background_enabled || !video.pixel.color[y][object->x + x - 8]
                    || (!video.pixel.priority[y][object->x + x - 8] && !object->attribute.priority)))
                {
                    video.pixel.data[y][object->x + x - 8] = video.object.color.cgb[object->attribute.palette][color];
                }
            }
        }
    }
}

static void cgbl_video_cgb_object_search(void)
{
    uint8_t size = video.control.object_size ? 16 : 8, y = video.line.y;
    video.object.shown.count = 0;
    for (uint8_t index = 0; index < CGBL_VIDEO_RAM_OBJECT_WIDTH; ++index)
    {
        const cgbl_object_t *object = &video.object.ram[index];
        if ((y < (object->y - 16 + size)) && (y >= (object->y - 16)))
        {
            cgbl_object_entry_t *entry = &video.object.shown.entry[video.object.shown.count++];
            entry->object = object;
            entry->index = index;
        }
        if (video.object.shown.count >= 10)
        {
            break;
        }
    }
}

static cgbl_color_e cgbl_video_dmg_palette_color(const cgbl_palette_t *const palette, cgbl_color_e color)
{
    cgbl_color_e result = CGBL_COLOR_WHITE;
    switch (color)
    {
        case CGBL_COLOR_WHITE:
            result = palette->white;
            break;
        case CGBL_COLOR_GREY_LIGHT:
            result = palette->grey_light;
            break;
        case CGBL_COLOR_GREY_DARK:
            result = palette->grey_dark;
            break;
        case CGBL_COLOR_BLACK:
            result = palette->black;
            break;
        default:
            break;
    }
    return result;
}

static void cgbl_video_dmg_palette_reset(void)
{
    char disambiguation = '\0';
    uint8_t hash = 0, index = 0;
    hash = cgbl_cartridge_palette_hash(&disambiguation);
    for (; index < CGBL_LENGTH(PALETTE) - 1; ++index)
    {
        if (hash == PALETTE[index].hash)
        {
            if (PALETTE[index].disambiguation && (disambiguation != PALETTE[index].disambiguation))
            {
                continue;
            }
            break;
        }
    }
    video.background.color.dmg = PALETTE[index].background;
    video.object.color.dmg[0] = PALETTE[index].object[0];
    video.object.color.dmg[1] = PALETTE[index].object[1];
}

static cgbl_color_e cgbl_video_dmg_background_color(uint8_t map, uint8_t x, uint8_t y)
{
    uint16_t address = (map ? 0x1C00 : 0x1800) + (32 * ((y / 8) & 31)) + ((x / 8) & 31);
    if (video.control.background_data)
    {
        address = (16 * video.ram.data[0][address]) + (2 * (y & 7));
    }
    else
    {
        address = (16 * (int8_t)video.ram.data[0][address]) + (2 * (y & 7)) + 0x1000;
    }
    x = 1 << (7 - (x & 7));
    return ((video.ram.data[0][address + 1] & x) ? 2 : 0) + ((video.ram.data[0][address] & x) ? 1 : 0);
}

static void cgbl_video_dmg_background_render(void)
{
    for (uint8_t index = 0; index < CGBL_VIDEO_WIDTH; ++index)
    {
        cgbl_color_e color = CGBL_COLOR_WHITE;
        uint8_t map = 0, x = index, y = video.line.y;
        if (video.control.window_enabled && (video.window.x <= 166) && (video.window.y <= 143) && ((video.window.x - 7) <= x) && (video.window.y <= y))
        {
            map = video.control.window_map;
            x -= (video.window.x - 7);
            y = video.window.counter - video.window.y;
        }
        else
        {
            map = video.control.background_map;
            x += video.scroll.x;
            y += video.scroll.y;
        }
        color = cgbl_video_dmg_palette_color(&video.background.color.palette, cgbl_video_dmg_background_color(map, x, y));
        video.pixel.color[video.line.y][index] = color;
        video.pixel.data[video.line.y][index] = video.background.color.dmg[color];
    }
}

static cgbl_color_e cgbl_video_dmg_object_color(const cgbl_object_t *object, uint8_t x, uint8_t y)
{
    uint16_t address = 0;
    uint8_t id = object->id;
    if (video.control.object_size)
    {
        if (object->attribute.flip_y)
        {
            if ((y - (object->y - 16)) < 8)
            {
                id |= 1;
            }
            else
            {
                id &= 0xFE;
            }
        }
        else if ((y - (object->y - 16)) < 8)
        {
            id &= 0xFE;
        }
        else
        {
            id |= 1;
        }
    }
    y = (y - object->y) & 7;
    if (object->attribute.flip_x)
    {
        x = 7 - x;
    }
    if (object->attribute.flip_y)
    {
        y = 7 - y;
    }
    address = (16 * id) + (2 * y);
    x = 1 << (7 - x);
    return ((video.ram.data[0][address + 1] & x) ? 2 : 0) + ((video.ram.data[0][address] & x) ? 1 : 0);
}

static int cgbl_video_dmg_object_comparator(const void *first, const void *second)
{
    int result = 0;
    const cgbl_object_entry_t *entry[] = { first, second };
    if (entry[0]->object->x == entry[1]->object->x)
    {
        result = entry[1]->index - entry[0]->index;
    }
    else
    {
        result = entry[1]->object->x - entry[0]->object->x;
    }
    return result;
}

static void cgbl_video_dmg_object_render(void)
{
    uint8_t y = video.line.y;
    for (uint32_t index = 0; index < video.object.shown.count; ++index)
    {
        const cgbl_object_t *object = video.object.shown.entry[index].object;
        for (uint8_t x = 0; x < 8; ++x)
        {
            cgbl_color_e color = CGBL_COLOR_WHITE;
            if ((object->x < 8) && (x < (8 - object->x)))
            {
                continue;
            }
            else if ((object->x >= 160) && (x >= (8 - (object->x - 160))))
            {
                break;
            }
            if ((color = cgbl_video_dmg_object_color(object, x, y)) && (!object->attribute.priority || !video.pixel.color[y][object->x + x - 8]))
            {
                color = cgbl_video_dmg_palette_color(&video.object.color.palette[object->attribute.palette_dmg], color);
                video.pixel.data[y][object->x + x - 8] = video.object.color.dmg[object->attribute.palette_dmg][color];
            }
        }
    }
}

static void cgbl_video_dmg_object_search(void)
{
    video.object.shown.count = 0;
    uint8_t size = video.control.object_size ? 16 : 8, y = video.line.y;
    for (uint8_t index = 0; index < CGBL_VIDEO_RAM_OBJECT_WIDTH; ++index)
    {
        const cgbl_object_t *object = &video.object.ram[index];
        if ((y >= (object->y - 16)) && (y < (object->y - 16 + size)))
        {
            cgbl_object_entry_t *entry = &video.object.shown.entry[video.object.shown.count++];
            entry->object = object;
            entry->index = index;
        }
        if (video.object.shown.count >= 10)
        {
            break;
        }
    }
    if (video.object.shown.count > 1)
    {
        qsort(video.object.shown.entry, video.object.shown.count, sizeof (*video.object.shown.entry), cgbl_video_dmg_object_comparator);
    }
}

static void cgbl_video_coincidence(void)
{
    if (video.control.enabled)
    {
        bool coincidence = (!video.line.coincidence && (video.line.y == 153)) || (video.line.coincidence == video.line.y);
        if (coincidence && !video.status.coincidence && video.status.interrupt_coincidence)
        {
            cgbl_processor_signal(CGBL_INTERRUPT_SCREEN);
        }
        video.status.coincidence = coincidence;
    }
}

static void cgbl_video_transfer_hblank(void)
{
    if (!cgbl_processor_halted())
    {
        uint16_t destination = video.transfer.destination.word + video.transfer.offset, source = video.transfer.source.word + video.transfer.offset;
        for (uint16_t length = 0; length < 16; ++length)
        {
            video.ram.data[(cgbl_bus_mode() == CGBL_MODE_CGB) ? video.ram.bank.select : 0][destination++] = cgbl_bus_read(source++);
        }
        video.transfer.offset += 16;
        if (!--video.transfer.control.length)
        {
            video.transfer.active = false;
            video.transfer.offset = 0;
            video.transfer.control.raw = 0xFF;
        }
    }
}

static void cgbl_video_transfer_immediate(void)
{
    uint16_t destination = video.transfer.destination.word, source = video.transfer.source.word;
    for (uint16_t length = 0; length < (video.transfer.control.length * 16); ++length)
    {
        video.ram.data[(cgbl_bus_mode() == CGBL_MODE_CGB) ? video.ram.bank.select : 0][destination++] = cgbl_bus_read(source++);
    }
    video.transfer.control.raw = 0xFF;
}

static void cgbl_video_transfer_objects(void)
{
    if (video.transfer.object.destination)
    {
        if (!video.transfer.object.delay)
        {
            video.transfer.object.delay = 4;
            ((uint8_t *)video.object.ram)[video.transfer.object.destination++ - CGBL_VIDEO_RAM_OBJECT_BEGIN] = cgbl_bus_read(video.transfer.object.source++);
            if (video.transfer.object.destination > CGBL_VIDEO_RAM_OBJECT_END)
            {
                video.transfer.object.delay = 0;
                video.transfer.object.destination = 0;
                video.transfer.object.source = 0;
                return;
            }
        }
        --video.transfer.object.delay;
    }
}

static void cgbl_video_hblank(void)
{
    video.status.state = CGBL_STATE_HBLANK;
    if (video.control.enabled)
    {
        if ((cgbl_bus_mode() == CGBL_MODE_CGB) && video.transfer.active)
        {
            cgbl_video_transfer_hblank();
        }
        if (video.status.interrupt_hblank)
        {
            cgbl_processor_signal(CGBL_INTERRUPT_SCREEN);
        }
    }
}

static void cgbl_video_search(void)
{
    video.status.state = CGBL_STATE_SEARCH;
    if (video.control.enabled)
    {
        if (video.control.object_enabled)
        {
            if ((cgbl_bus_mode() == CGBL_MODE_CGB) && (cgbl_bus_priority() == CGBL_PRIORITY_CGB))
            {
                cgbl_video_cgb_object_search();
            }
            else
            {
                cgbl_video_dmg_object_search();
            }
        }
        if (video.status.interrupt_search)
        {
            cgbl_processor_signal(CGBL_INTERRUPT_SCREEN);
        }
    }
}

static void cgbl_video_transfer(void)
{
    video.status.state = CGBL_STATE_TRANSFER;
    if (video.control.enabled && video.shown)
    {
        cgbl_mode_e mode = cgbl_bus_mode();
        if (mode == CGBL_MODE_CGB)
        {
            cgbl_video_cgb_background_render();
        }
        else if (video.control.background_enabled)
        {
            cgbl_video_dmg_background_render();
        }
        if (video.control.object_enabled)
        {
            if (mode == CGBL_MODE_CGB)
            {
                cgbl_video_cgb_object_render();
            }
            else
            {
                cgbl_video_dmg_object_render();
            }
        }
    }
}

static void cgbl_video_vblank(void)
{
    video.status.state = CGBL_STATE_VBLANK;
    if (video.control.enabled)
    {
        if (video.status.interrupt_vblank)
        {
            cgbl_processor_signal(CGBL_INTERRUPT_SCREEN);
        }
        cgbl_processor_signal(CGBL_INTERRUPT_VBLANK);
    }
}

const uint16_t (*cgbl_video_color(void))[CGBL_VIDEO_HEIGHT][CGBL_VIDEO_WIDTH]
{
    return &video.pixel.data;
}

uint8_t cgbl_video_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case CGBL_VIDEO_CONTROL:
            result = video.control.raw;
            if (!video.control.enabled)
            {
                video.shown = false;
            }
            break;
        case CGBL_VIDEO_LINE_Y:
            result = video.line.y;
            break;
        case CGBL_VIDEO_LINE_Y_COINCIDENCE:
            result = video.line.coincidence;
            break;
        case CGBL_VIDEO_PALETTE_BACKGROUND:
            result = video.background.color.palette.raw;
            break;
        case CGBL_VIDEO_PALETTE_BACKGROUND_CONTROL:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                result = video.background.control.raw;
            }
            break;
        case CGBL_VIDEO_PALETTE_BACKGROUND_DATA:
            if ((cgbl_bus_mode() == CGBL_MODE_CGB) && (!video.control.enabled || (video.status.state < CGBL_STATE_TRANSFER)))
            {
                result = ((uint8_t *)video.background.color.cgb)[video.background.control.address];
            }
            break;
        case CGBL_VIDEO_PALETTE_OBJECT_0:
            result = video.object.color.palette[0].raw;
            break;
        case CGBL_VIDEO_PALETTE_OBJECT_1:
            result = video.object.color.palette[1].raw;
            break;
        case CGBL_VIDEO_PALETTE_OBJECT_CONTROL:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                result = video.object.control.raw;
            }
            break;
        case CGBL_VIDEO_PALETTE_OBJECT_DATA:
            if ((cgbl_bus_mode() == CGBL_MODE_CGB) && (!video.control.enabled || (video.status.state < CGBL_STATE_TRANSFER)))
            {
                result = ((uint8_t *)video.object.color.cgb)[video.object.control.address];
            }
            break;
        case CGBL_VIDEO_RAM_BEGIN ... CGBL_VIDEO_RAM_END:
            if (!video.control.enabled || (video.status.state < CGBL_STATE_TRANSFER))
            {
                result = video.ram.data[(cgbl_bus_mode() == CGBL_MODE_CGB) ? video.ram.bank.select : 0][address - CGBL_VIDEO_RAM_BEGIN];
            }
            break;
        case CGBL_VIDEO_RAM_OBJECT_BEGIN ... CGBL_VIDEO_RAM_OBJECT_END:
            if (!video.control.enabled || (video.status.state < CGBL_STATE_SEARCH))
            {
                result = ((uint8_t *)video.object.ram)[address - CGBL_VIDEO_RAM_OBJECT_BEGIN];
            }
            break;
        case CGBL_VIDEO_RAM_SELECT:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                result = video.ram.bank.raw;
            }
            break;
        case CGBL_VIDEO_SCROLL_X:
            result = video.scroll.x;
            break;
        case CGBL_VIDEO_SCROLL_Y:
            result = video.scroll.y;
            break;
        case CGBL_VIDEO_STATUS:
            result = video.status.raw;
            break;
        case CGBL_VIDEO_TRANSFER_CONTROL:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                result = video.transfer.active ? 0x80 : 0;
            }
            break;
        case CGBL_VIDEO_TRANSFER_OBJECTS:
            result = video.transfer.object.address;
            break;
        case CGBL_VIDEO_WINDOW_X:
            result = video.window.x;
            break;
        case CGBL_VIDEO_WINDOW_Y:
            result = video.window.y;
            break;
        default:
            break;
    }
    return result;
}

void cgbl_video_reset(void)
{
    memset(&video, 0, sizeof (video));
    cgbl_video_dmg_palette_reset();
    video.ram.bank.raw = 0xFE;
    video.status.raw = 0x80 | CGBL_STATE_SEARCH;
}

cgbl_error_e cgbl_video_step(void)
{
    cgbl_error_e result = CGBL_SUCCESS;
    cgbl_video_coincidence();
    for (uint8_t cycle = 0; cycle < ((cgbl_bus_speed() == CGBL_SPEED_DOUBLE) ? 2 : 1); ++cycle)
    {
        cgbl_video_transfer_objects();
    }
    if (video.line.y < 144)
    {
        if (!video.line.x)
        {
            cgbl_video_search();
        }
        else if (video.line.x == 80)
        {
            cgbl_video_transfer();
        }
        else if (video.line.x == 240)
        {
            cgbl_video_hblank();
        }
    }
    else if ((video.line.y == 144) && !video.line.x)
    {
        cgbl_video_vblank();
    }
    if (++video.line.x == 456)
    {
        video.line.x = 0;
        if ((video.window.x <= 166) && (video.window.y <= 143))
        {
            ++video.window.counter;
        }
        if (++video.line.y == 154)
        {
            video.line.y = 0;
            video.shown = true;
            video.window.counter = 0;
            result = CGBL_QUIT;
        }
    }
    return result;
}

void cgbl_video_write(uint16_t address, uint8_t data)
{
    switch (address)
    {
        case CGBL_VIDEO_CONTROL:
            video.control.raw = data;
            if (!video.control.enabled)
            {
                video.shown = false;
            }
            break;
        case CGBL_VIDEO_LINE_Y_COINCIDENCE:
            video.line.coincidence = data;
            break;
        case CGBL_VIDEO_PALETTE_BACKGROUND:
            video.background.color.palette.raw = data;
            break;
        case CGBL_VIDEO_PALETTE_BACKGROUND_CONTROL:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                video.background.control.raw = (data & 0xBF) | 0x40;
            }
            break;
        case CGBL_VIDEO_PALETTE_BACKGROUND_DATA:
            if ((cgbl_bus_mode() == CGBL_MODE_CGB) && (!video.control.enabled || (video.status.state < CGBL_STATE_TRANSFER)))
            {
                ((uint8_t *)video.background.color.cgb)[video.background.control.address] = data;
                if (video.background.control.increment)
                {
                    ++video.background.control.address;
                }
            }
            break;
        case CGBL_VIDEO_PALETTE_OBJECT_0:
            video.object.color.palette[0].raw = data;
            break;
        case CGBL_VIDEO_PALETTE_OBJECT_1:
            video.object.color.palette[1].raw = data;
            break;
        case CGBL_VIDEO_PALETTE_OBJECT_CONTROL:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                video.object.control.raw = (data & 0xBF) | 0x40;
            }
            break;
        case CGBL_VIDEO_PALETTE_OBJECT_DATA:
            if ((cgbl_bus_mode() == CGBL_MODE_CGB) && (!video.control.enabled || (video.status.state < CGBL_STATE_TRANSFER)))
            {
                ((uint8_t *)video.object.color.cgb)[video.object.control.address] = data;
                if (video.object.control.increment)
                {
                    ++video.object.control.address;
                }
            }
            break;
        case CGBL_VIDEO_RAM_BEGIN ... CGBL_VIDEO_RAM_END:
            if (!video.control.enabled || (video.status.state < CGBL_STATE_TRANSFER))
            {
                video.ram.data[(cgbl_bus_mode() == CGBL_MODE_CGB) ? video.ram.bank.select : 0][address - CGBL_VIDEO_RAM_BEGIN] = data;
            }
            break;
        case CGBL_VIDEO_RAM_OBJECT_BEGIN ... CGBL_VIDEO_RAM_OBJECT_END:
            if (!video.control.enabled || (video.status.state < CGBL_STATE_SEARCH))
            {
                ((uint8_t *)video.object.ram)[address - CGBL_VIDEO_RAM_OBJECT_BEGIN] = data;
            }
            break;
        case CGBL_VIDEO_RAM_SELECT:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                video.ram.bank.raw = (data & 1) | 0xFE;
            }
            break;
        case CGBL_VIDEO_SCROLL_X:
            video.scroll.x = data;
            break;
        case CGBL_VIDEO_SCROLL_Y:
            video.scroll.y = data;
            break;
        case CGBL_VIDEO_STATUS:
            video.status.raw = (data & 0x78) | 0x80;
            break;
        case CGBL_VIDEO_TRANSFER_CONTROL:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                bool hblank = video.transfer.control.hblank;
                video.transfer.control.raw = data;
                if (video.transfer.control.hblank && !video.transfer.active)
                {
                    video.transfer.active = true;
                }
                else
                {
                    if (hblank && video.transfer.active)
                    {
                        video.transfer.active = false;
                        video.transfer.offset = 0;
                        video.transfer.control.raw = 0xFF;
                    }
                    else
                    {
                        cgbl_video_transfer_immediate();
                    }
                }
            }
            break;
        case CGBL_VIDEO_TRANSFER_DESTINATION_HIGH:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                video.transfer.destination.high = data & 0x1F;
            }
            break;
        case CGBL_VIDEO_TRANSFER_DESTINATION_LOW:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                video.transfer.destination.low = data & 0xF0;
            }
            break;
        case CGBL_VIDEO_TRANSFER_OBJECTS:
            video.transfer.object.address = data;
            video.transfer.object.delay = 4;
            video.transfer.object.destination = CGBL_VIDEO_RAM_OBJECT_BEGIN;
            video.transfer.object.source = video.transfer.object.address << 8;
            break;
        case CGBL_VIDEO_TRANSFER_SOURCE_HIGH:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                video.transfer.source.high = data;
            }
            break;
        case CGBL_VIDEO_TRANSFER_SOURCE_LOW:
            if (cgbl_bus_mode() == CGBL_MODE_CGB)
            {
                video.transfer.source.low = data & 0xF0;
            }
            break;
        case CGBL_VIDEO_WINDOW_X:
            video.window.x = data;
            break;
        case CGBL_VIDEO_WINDOW_Y:
            video.window.y = data;
            break;
        default:
            break;
    }
}
