/*
 * Generate a file for hardcoded tables
 *
 * Copyright (c) 2009 Reimar Döffinger <Reimar.Doeffinger@gmx.de>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <inttypes.h>
#include "tableprint.h"

WRITE_1D_FUNC(int8_t,   "%3"PRIi8, 15)
WRITE_1D_FUNC(uint8_t,  "0x%02"PRIx8, 15)
WRITE_1D_FUNC(uint16_t, "0x%08"PRIx16, 7)
WRITE_1D_FUNC(uint32_t, "0x%08"PRIx32, 7)
WRITE_1D_FUNC(float,    "%.18e", 3)

WRITE_2D_FUNC(int8_t)
WRITE_2D_FUNC(uint8_t)
WRITE_2D_FUNC(uint32_t)
WRITE_2D_FUNC(float)

void write_fileheader(void) {
    printf("/* This file was generated by libavcodec/tableprint */\n");
    printf("#include <stdint.h>\n");
}
