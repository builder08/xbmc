/*
 * AAC Spectral Band Replication decoding data
 * Copyright (c) 2008-2009 Robert Swain ( rob opendot cl )
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

/**
 * @file libavcodec/aacsbrdata.h
 * AAC Spectral Band Replication decoding data
 * @author Robert Swain ( rob opendot cl )
 */

#ifndef AVCODEC_AACSBRDATA_H
#define AVCODEC_AACSBRDATA_H

#include <stdint.h>
#include "libavutil/mem.h"

///< Huffman tables for SBR

static const uint8_t t_huffman_env_1_5dB_bits[121] = {
    18, 18, 18, 18, 18, 18, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 17, 18, 16, 17, 18, 17,
    16, 16, 16, 16, 15, 14, 14, 13,
    13, 12, 11, 10,  9,  8,  7,  6,
     5,  4,  3,  2,  2,  3,  4,  5,
     6,  7,  8,  9, 10, 12, 13, 14,
    14, 15, 16, 17, 16, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19,
};

static const uint32_t t_huffman_env_1_5dB_codes[121] = {
    0x3ffd6, 0x3ffd7, 0x3ffd8, 0x3ffd9, 0x3ffda, 0x3ffdb, 0x7ffb8, 0x7ffb9,
    0x7ffba, 0x7ffbb, 0x7ffbc, 0x7ffbd, 0x7ffbe, 0x7ffbf, 0x7ffc0, 0x7ffc1,
    0x7ffc2, 0x7ffc3, 0x7ffc4, 0x7ffc5, 0x7ffc6, 0x7ffc7, 0x7ffc8, 0x7ffc9,
    0x7ffca, 0x7ffcb, 0x7ffcc, 0x7ffcd, 0x7ffce, 0x7ffcf, 0x7ffd0, 0x7ffd1,
    0x7ffd2, 0x7ffd3, 0x1ffe6, 0x3ffd4, 0x0fff0, 0x1ffe9, 0x3ffd5, 0x1ffe7,
    0x0fff1, 0x0ffec, 0x0ffed, 0x0ffee, 0x07ff4, 0x03ff9, 0x03ff7, 0x01ffa,
    0x01ff9, 0x00ffb, 0x007fc, 0x003fc, 0x001fd, 0x000fd, 0x0007d, 0x0003d,
    0x0001d, 0x0000d, 0x00005, 0x00001, 0x00000, 0x00004, 0x0000c, 0x0001c,
    0x0003c, 0x0007c, 0x000fc, 0x001fc, 0x003fd, 0x00ffa, 0x01ff8, 0x03ff6,
    0x03ff8, 0x07ff5, 0x0ffef, 0x1ffe8, 0x0fff2, 0x7ffd4, 0x7ffd5, 0x7ffd6,
    0x7ffd7, 0x7ffd8, 0x7ffd9, 0x7ffda, 0x7ffdb, 0x7ffdc, 0x7ffdd, 0x7ffde,
    0x7ffdf, 0x7ffe0, 0x7ffe1, 0x7ffe2, 0x7ffe3, 0x7ffe4, 0x7ffe5, 0x7ffe6,
    0x7ffe7, 0x7ffe8, 0x7ffe9, 0x7ffea, 0x7ffeb, 0x7ffec, 0x7ffed, 0x7ffee,
    0x7ffef, 0x7fff0, 0x7fff1, 0x7fff2, 0x7fff3, 0x7fff4, 0x7fff5, 0x7fff6,
    0x7fff7, 0x7fff8, 0x7fff9, 0x7fffa, 0x7fffb, 0x7fffc, 0x7fffd, 0x7fffe,
    0x7ffff,
};

static const uint8_t f_huffman_env_1_5dB_bits[121] = {
    19, 19, 20, 20, 20, 20, 20, 20,
    20, 19, 20, 20, 20, 20, 19, 20,
    19, 19, 20, 18, 20, 20, 20, 19,
    20, 20, 20, 19, 20, 19, 18, 19,
    18, 18, 17, 18, 17, 17, 17, 16,
    16, 16, 15, 15, 14, 13, 13, 12,
    12, 11, 10,  9,  9,  8,  7,  6,
     5,  4,  3,  2,  2,  3,  4,  5,
     6,  8,  8,  9, 10, 11, 11, 11,
    12, 12, 13, 13, 14, 14, 16, 16,
    17, 17, 18, 18, 18, 18, 18, 18,
    18, 20, 19, 20, 20, 20, 20, 20,
    20, 19, 20, 20, 20, 20, 19, 20,
    18, 20, 20, 19, 19, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20,
    20,
};

static const uint32_t f_huffman_env_1_5dB_codes[121] = {
    0x7ffe7, 0x7ffe8, 0xfffd2, 0xfffd3, 0xfffd4, 0xfffd5, 0xfffd6, 0xfffd7,
    0xfffd8, 0x7ffda, 0xfffd9, 0xfffda, 0xfffdb, 0xfffdc, 0x7ffdb, 0xfffdd,
    0x7ffdc, 0x7ffdd, 0xfffde, 0x3ffe4, 0xfffdf, 0xfffe0, 0xfffe1, 0x7ffde,
    0xfffe2, 0xfffe3, 0xfffe4, 0x7ffdf, 0xfffe5, 0x7ffe0, 0x3ffe8, 0x7ffe1,
    0x3ffe0, 0x3ffe9, 0x1ffef, 0x3ffe5, 0x1ffec, 0x1ffed, 0x1ffee, 0x0fff4,
    0x0fff3, 0x0fff0, 0x07ff7, 0x07ff6, 0x03ffa, 0x01ffa, 0x01ff9, 0x00ffa,
    0x00ff8, 0x007f9, 0x003fb, 0x001fc, 0x001fa, 0x000fb, 0x0007c, 0x0003c,
    0x0001c, 0x0000c, 0x00005, 0x00001, 0x00000, 0x00004, 0x0000d, 0x0001d,
    0x0003d, 0x000fa, 0x000fc, 0x001fb, 0x003fa, 0x007f8, 0x007fa, 0x007fb,
    0x00ff9, 0x00ffb, 0x01ff8, 0x01ffb, 0x03ff8, 0x03ff9, 0x0fff1, 0x0fff2,
    0x1ffea, 0x1ffeb, 0x3ffe1, 0x3ffe2, 0x3ffea, 0x3ffe3, 0x3ffe6, 0x3ffe7,
    0x3ffeb, 0xfffe6, 0x7ffe2, 0xfffe7, 0xfffe8, 0xfffe9, 0xfffea, 0xfffeb,
    0xfffec, 0x7ffe3, 0xfffed, 0xfffee, 0xfffef, 0xffff0, 0x7ffe4, 0xffff1,
    0x3ffec, 0xffff2, 0xffff3, 0x7ffe5, 0x7ffe6, 0xffff4, 0xffff5, 0xffff6,
    0xffff7, 0xffff8, 0xffff9, 0xffffa, 0xffffb, 0xffffc, 0xffffd, 0xffffe,
    0xfffff,
};

static const uint8_t t_huffman_env_bal_1_5dB_bits[49] = {
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 12, 11,  9,  7,  5,  3,
     1,  2,  4,  6,  8, 11, 12, 15,
    16, 16, 16, 16, 16, 16, 16, 17,
    17, 17, 17, 17, 17, 17, 17, 17,
    17,
};

static const uint32_t t_huffman_env_bal_1_5dB_codes[49] = {
    0x0ffe4, 0x0ffe5, 0x0ffe6, 0x0ffe7, 0x0ffe8, 0x0ffe9, 0x0ffea, 0x0ffeb,
    0x0ffec, 0x0ffed, 0x0ffee, 0x0ffef, 0x0fff0, 0x0fff1, 0x0fff2, 0x0fff3,
    0x0fff4, 0x0ffe2, 0x00ffc, 0x007fc, 0x001fe, 0x0007e, 0x0001e, 0x00006,
    0x00000, 0x00002, 0x0000e, 0x0003e, 0x000fe, 0x007fd, 0x00ffd, 0x07ff0,
    0x0ffe3, 0x0fff5, 0x0fff6, 0x0fff7, 0x0fff8, 0x0fff9, 0x0fffa, 0x1fff6,
    0x1fff7, 0x1fff8, 0x1fff9, 0x1fffa, 0x1fffb, 0x1fffc, 0x1fffd, 0x1fffe,
    0x1ffff,
};

static const uint8_t f_huffman_env_bal_1_5dB_bits[49] = {
    18, 18, 18, 18, 18, 18, 18, 18,
    18, 18, 18, 18, 18, 18, 18, 16,
    17, 14, 11, 11,  8,  7,  4,  2,
     1,  3,  5,  6,  9, 11, 12, 15,
    16, 18, 18, 18, 18, 18, 18, 18,
    18, 18, 18, 18, 18, 18, 18, 19,
    19,
};

static const uint32_t f_huffman_env_bal_1_5dB_codes[49] = {
    0x3ffe2, 0x3ffe3, 0x3ffe4, 0x3ffe5, 0x3ffe6, 0x3ffe7, 0x3ffe8, 0x3ffe9,
    0x3ffea, 0x3ffeb, 0x3ffec, 0x3ffed, 0x3ffee, 0x3ffef, 0x3fff0, 0x0fff7,
    0x1fff0, 0x03ffc, 0x007fe, 0x007fc, 0x000fe, 0x0007e, 0x0000e, 0x00002,
    0x00000, 0x00006, 0x0001e, 0x0003e, 0x001fe, 0x007fd, 0x00ffe, 0x07ffa,
    0x0fff6, 0x3fff1, 0x3fff2, 0x3fff3, 0x3fff4, 0x3fff5, 0x3fff6, 0x3fff7,
    0x3fff8, 0x3fff9, 0x3fffa, 0x3fffb, 0x3fffc, 0x3fffd, 0x3fffe, 0x7fffe,
    0x7ffff,
};

static const uint8_t t_huffman_env_3_0dB_bits[63] = {
    18, 18, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19, 17, 16, 16, 16, 14, 14, 14,
    13, 12, 11,  8,  6,  4,  2,  1,
     3,  5,  7,  9, 11, 13, 14, 14,
    15, 16, 17, 18, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 19,
};

static const uint32_t t_huffman_env_3_0dB_codes[63] = {
    0x3ffed, 0x3ffee, 0x7ffde, 0x7ffdf, 0x7ffe0, 0x7ffe1, 0x7ffe2, 0x7ffe3,
    0x7ffe4, 0x7ffe5, 0x7ffe6, 0x7ffe7, 0x7ffe8, 0x7ffe9, 0x7ffea, 0x7ffeb,
    0x7ffec, 0x1fff4, 0x0fff7, 0x0fff9, 0x0fff8, 0x03ffb, 0x03ffa, 0x03ff8,
    0x01ffa, 0x00ffc, 0x007fc, 0x000fe, 0x0003e, 0x0000e, 0x00002, 0x00000,
    0x00006, 0x0001e, 0x0007e, 0x001fe, 0x007fd, 0x01ffb, 0x03ff9, 0x03ffc,
    0x07ffa, 0x0fff6, 0x1fff5, 0x3ffec, 0x7ffed, 0x7ffee, 0x7ffef, 0x7fff0,
    0x7fff1, 0x7fff2, 0x7fff3, 0x7fff4, 0x7fff5, 0x7fff6, 0x7fff7, 0x7fff8,
    0x7fff9, 0x7fffa, 0x7fffb, 0x7fffc, 0x7fffd, 0x7fffe, 0x7ffff,
};

static const uint8_t f_huffman_env_3_0dB_bits[63] = {
    20, 20, 20, 20, 20, 20, 20, 18,
    19, 19, 19, 19, 18, 18, 20, 19,
    17, 18, 17, 16, 16, 15, 14, 12,
    11, 10,  9,  8,  6,  4,  2,  1,
     3,  5,  8,  9, 10, 11, 12, 13,
    14, 15, 15, 16, 16, 17, 17, 18,
    18, 18, 20, 19, 19, 19, 20, 19,
    19, 20, 20, 20, 20, 20, 20,
};

static const uint32_t f_huffman_env_3_0dB_codes[63] = {
    0xffff0, 0xffff1, 0xffff2, 0xffff3, 0xffff4, 0xffff5, 0xffff6, 0x3fff3,
    0x7fff5, 0x7ffee, 0x7ffef, 0x7fff6, 0x3fff4, 0x3fff2, 0xffff7, 0x7fff0,
    0x1fff5, 0x3fff0, 0x1fff4, 0x0fff7, 0x0fff6, 0x07ff8, 0x03ffb, 0x00ffd,
    0x007fd, 0x003fd, 0x001fd, 0x000fd, 0x0003e, 0x0000e, 0x00002, 0x00000,
    0x00006, 0x0001e, 0x000fc, 0x001fc, 0x003fc, 0x007fc, 0x00ffc, 0x01ffc,
    0x03ffa, 0x07ff9, 0x07ffa, 0x0fff8, 0x0fff9, 0x1fff6, 0x1fff7, 0x3fff5,
    0x3fff6, 0x3fff1, 0xffff8, 0x7fff1, 0x7fff2, 0x7fff3, 0xffff9, 0x7fff7,
    0x7fff4, 0xffffa, 0xffffb, 0xffffc, 0xffffd, 0xffffe, 0xfffff,
};

static const uint8_t t_huffman_env_bal_3_0dB_bits[25] = {
    13, 13, 13, 13, 13, 13, 13, 12,
     8,  7,  4,  3,  1,  2,  5,  6,
     9, 13, 13, 13, 13, 13, 13, 14,
    14,
};

static const uint16_t t_huffman_env_bal_3_0dB_codes[25] = {
    0x1ff2, 0x1ff3, 0x1ff4, 0x1ff5, 0x1ff6, 0x1ff7, 0x1ff8, 0x0ff8,
    0x00fe, 0x007e, 0x000e, 0x0006, 0x0000, 0x0002, 0x001e, 0x003e,
    0x01fe, 0x1ff9, 0x1ffa, 0x1ffb, 0x1ffc, 0x1ffd, 0x1ffe, 0x3ffe,
    0x3fff,
};

static const uint8_t f_huffman_env_bal_3_0dB_bits[25] = {
    13, 13, 13, 13, 13, 14, 14, 11,
     8,  7,  4,  2,  1,  3,  5,  6,
     9, 12, 13, 14, 14, 14, 14, 14,
    14,
};

static const uint16_t f_huffman_env_bal_3_0dB_codes[25] = {
    0x1ff7, 0x1ff8, 0x1ff9, 0x1ffa, 0x1ffb, 0x3ff8, 0x3ff9, 0x07fc,
    0x00fe, 0x007e, 0x000e, 0x0002, 0x0000, 0x0006, 0x001e, 0x003e,
    0x01fe, 0x0ffa, 0x1ff6, 0x3ffa, 0x3ffb, 0x3ffc, 0x3ffd, 0x3ffe,
    0x3fff,
};

static const uint8_t t_huffman_noise_3_0dB_bits[63] = {
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 11,  8,  6,  4,  3,  1,
     2,  5,  8, 10, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 14, 14,
};

static const uint16_t t_huffman_noise_3_0dB_codes[63] = {
    0x1fce, 0x1fcf, 0x1fd0, 0x1fd1, 0x1fd2, 0x1fd3, 0x1fd4, 0x1fd5,
    0x1fd6, 0x1fd7, 0x1fd8, 0x1fd9, 0x1fda, 0x1fdb, 0x1fdc, 0x1fdd,
    0x1fde, 0x1fdf, 0x1fe0, 0x1fe1, 0x1fe2, 0x1fe3, 0x1fe4, 0x1fe5,
    0x1fe6, 0x1fe7, 0x07f2, 0x00fd, 0x003e, 0x000e, 0x0006, 0x0000,
    0x0002, 0x001e, 0x00fc, 0x03f8, 0x1fcc, 0x1fe8, 0x1fe9, 0x1fea,
    0x1feb, 0x1fec, 0x1fcd, 0x1fed, 0x1fee, 0x1fef, 0x1ff0, 0x1ff1,
    0x1ff2, 0x1ff3, 0x1ff4, 0x1ff5, 0x1ff6, 0x1ff7, 0x1ff8, 0x1ff9,
    0x1ffa, 0x1ffb, 0x1ffc, 0x1ffd, 0x1ffe, 0x3ffe, 0x3fff,
};

static const uint8_t t_huffman_noise_bal_3_0dB_bits[25] = {
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 5, 2, 1, 3, 6, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8,
};

static const uint8_t t_huffman_noise_bal_3_0dB_codes[25] = {
    0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3,
    0xf4, 0xf5, 0x1c, 0x02, 0x00, 0x06, 0x3a, 0xf6,
    0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe,
    0xff,
};

static const int8_t sbr_offset[6][16] = {
    {-8, -7, -6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7}, //             fs_sbr  = 16000 Hz
    {-5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13}, //             fs_sbr  = 22050 Hz
    {-5, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13, 16}, //             fs_sbr  = 24000 Hz
    {-6, -4, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13, 16}, //             fs_sbr  = 32000 Hz
    {-4, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13, 16, 20}, // 44100 Hz <= fs_sbr <= 64000 Hz
    {-2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13, 16, 20, 24}, // 64000 Hz <  fs_sbr
};

///< window coefficients for analysis/synthesis QMF banks
static DECLARE_ALIGNED(16, float, sbr_qmf_window_ds)[320];
static DECLARE_ALIGNED(16, float, sbr_qmf_window_us)[640] = {
     0.0000000000, -0.0005525286, -0.0005617692, -0.0004947518,
    -0.0004875227, -0.0004893791, -0.0005040714, -0.0005226564,
    -0.0005466565, -0.0005677802, -0.0005870930, -0.0006132747,
    -0.0006312493, -0.0006540333, -0.0006777690, -0.0006941614,
    -0.0007157736, -0.0007255043, -0.0007440941, -0.0007490598,
    -0.0007681371, -0.0007724848, -0.0007834332, -0.0007779869,
    -0.0007803664, -0.0007801449, -0.0007757977, -0.0007630793,
    -0.0007530001, -0.0007319357, -0.0007215391, -0.0006917937,
    -0.0006650415, -0.0006341594, -0.0005946118, -0.0005564576,
    -0.0005145572, -0.0004606325, -0.0004095121, -0.0003501175,
    -0.0002896981, -0.0002098337, -0.0001446380, -0.0000617334,
     0.0000134949,  0.0001094383,  0.0002043017,  0.0002949531,
     0.0004026540,  0.0005107388,  0.0006239376,  0.0007458025,
     0.0008608443,  0.0009885988,  0.0011250155,  0.0012577884,
     0.0013902494,  0.0015443219,  0.0016868083,  0.0018348265,
     0.0019841140,  0.0021461583,  0.0023017254,  0.0024625616,
     0.0026201758,  0.0027870464,  0.0029469447,  0.0031125420,
     0.0032739613,  0.0034418874,  0.0036008268,  0.0037603922,
     0.0039207432,  0.0040819753,  0.0042264269,  0.0043730719,
     0.0045209852,  0.0046606460,  0.0047932560,  0.0049137603,
     0.0050393022,  0.0051407353,  0.0052461166,  0.0053471681,
     0.0054196775,  0.0054876040,  0.0055475714,  0.0055938023,
     0.0056220643,  0.0056455196,  0.0056389199,  0.0056266114,
     0.0055917128,  0.0055404363,  0.0054753783,  0.0053838975,
     0.0052715758,  0.0051382275,  0.0049839687,  0.0048109469,
     0.0046039530,  0.0043801861,  0.0041251642,  0.0038456408,
     0.0035401246,  0.0032091885,  0.0028446757,  0.0024508540,
     0.0020274176,  0.0015784682,  0.0010902329,  0.0005832264,
     0.0000276045, -0.0005464280, -0.0011568135, -0.0018039472,
    -0.0024826723, -0.0031933778, -0.0039401124, -0.0047222596,
    -0.0055337211, -0.0063792293, -0.0072615816, -0.0081798233,
    -0.0091325329, -0.0101150215, -0.0111315548, -0.0121849995,
     0.0132718220,  0.0143904666,  0.0155405553,  0.0167324712,
     0.0179433381,  0.0191872431,  0.0204531793,  0.0217467550,
     0.0230680169,  0.0244160992,  0.0257875847,  0.0271859429,
     0.0286072173,  0.0300502657,  0.0315017608,  0.0329754081,
     0.0344620948,  0.0359697560,  0.0374812850,  0.0390053679,
     0.0405349170,  0.0420649094,  0.0436097542,  0.0451488405,
     0.0466843027,  0.0482165720,  0.0497385755,  0.0512556155,
     0.0527630746,  0.0542452768,  0.0557173648,  0.0571616450,
     0.0585915683,  0.0599837480,  0.0613455171,  0.0626857808,
     0.0639715898,  0.0652247106,  0.0664367512,  0.0676075985,
     0.0687043828,  0.0697630244,  0.0707628710,  0.0717002673,
     0.0725682583,  0.0733620255,  0.0741003642,  0.0747452558,
     0.0753137336,  0.0758008358,  0.0761992479,  0.0764992170,
     0.0767093490,  0.0768173975,  0.0768230011,  0.0767204924,
     0.0765050718,  0.0761748321,  0.0757305756,  0.0751576255,
     0.0744664394,  0.0736406005,  0.0726774642,  0.0715826364,
     0.0703533073,  0.0689664013,  0.0674525021,  0.0657690668,
     0.0639444805,  0.0619602779,  0.0598166570,  0.0575152691,
     0.0550460034,  0.0524093821,  0.0495978676,  0.0466303305,
     0.0434768782,  0.0401458278,  0.0366418116,  0.0329583930,
     0.0290824006,  0.0250307561,  0.0207997072,  0.0163701258,
     0.0117623832,  0.0069636862,  0.0019765601, -0.0032086896,
    -0.0085711749, -0.0141288827, -0.0198834129, -0.0258227288,
    -0.0319531274, -0.0382776572, -0.0447806821, -0.0514804176,
    -0.0583705326, -0.0654409853, -0.0726943300, -0.0801372934,
    -0.0877547536, -0.0955533352, -0.1035329531, -0.1116826931,
    -0.1200077984, -0.1285002850, -0.1371551761, -0.1459766491,
    -0.1549607071, -0.1640958855, -0.1733808172, -0.1828172548,
    -0.1923966745, -0.2021250176, -0.2119735853, -0.2219652696,
    -0.2320690870, -0.2423016884, -0.2526480309, -0.2631053299,
    -0.2736634040, -0.2843214189, -0.2950716717, -0.3059098575,
    -0.3168278913, -0.3278113727, -0.3388722693, -0.3499914122,
     0.3611589903,  0.3723795546,  0.3836350013,  0.3949211761,
     0.4062317676,  0.4175696896,  0.4289119920,  0.4402553754,
     0.4515996535,  0.4629308085,  0.4742453214,  0.4855253091,
     0.4967708254,  0.5079817500,  0.5191234970,  0.5302240895,
     0.5412553448,  0.5522051258,  0.5630789140,  0.5738524131,
     0.5845403235,  0.5951123086,  0.6055783538,  0.6159109932,
     0.6261242695,  0.6361980107,  0.6461269695,  0.6559016302,
     0.6655139880,  0.6749663190,  0.6842353293,  0.6933282376,
     0.7022388719,  0.7109410426,  0.7194462634,  0.7277448900,
     0.7358211758,  0.7436827863,  0.7513137456,  0.7587080760,
     0.7658674865,  0.7727780881,  0.7794287519,  0.7858353120,
     0.7919735841,  0.7978466413,  0.8034485751,  0.8087695004,
     0.8138191270,  0.8185776004,  0.8230419890,  0.8272275347,
     0.8311038457,  0.8346937361,  0.8379717337,  0.8409541392,
     0.8436238281,  0.8459818469,  0.8480315777,  0.8497805198,
     0.8511971524,  0.8523047035,  0.8531020949,  0.8535720573,
     0.8537385600,
};

static const float sbr_noise_table[512][2] = {
{-0.99948153278296, -0.59483417516607}, { 0.97113454393991, -0.67528515225647},
{ 0.14130051758487, -0.95090983575689}, {-0.47005496701697, -0.37340549728647},
{ 0.80705063769351,  0.29653668284408}, {-0.38981478896926,  0.89572605717087},
{-0.01053049862020, -0.66959058036166}, {-0.91266367957293, -0.11522938140034},
{ 0.54840422910309,  0.75221367176302}, { 0.40009252867955, -0.98929400334421},
{-0.99867974711855, -0.88147068645358}, {-0.95531076805040,  0.90908757154593},
{-0.45725933317144, -0.56716323646760}, {-0.72929675029275, -0.98008272727324},
{ 0.75622801399036,  0.20950329995549}, { 0.07069442601050, -0.78247898470706},
{ 0.74496252926055, -0.91169004445807}, {-0.96440182703856, -0.94739918296622},
{ 0.30424629369539, -0.49438267012479}, { 0.66565033746925,  0.64652935542491},
{ 0.91697008020594,  0.17514097332009}, {-0.70774918760427,  0.52548653416543},
{-0.70051415345560, -0.45340028808763}, {-0.99496513054797, -0.90071908066973},
{ 0.98164490790123, -0.77463155528697}, {-0.54671580548181, -0.02570928536004},
{-0.01689629065389,  0.00287506445732}, {-0.86110349531986,  0.42548583726477},
{-0.98892980586032, -0.87881132267556}, { 0.51756627678691,  0.66926784710139},
{-0.99635026409640, -0.58107730574765}, {-0.99969370862163,  0.98369989360250},
{ 0.55266258627194,  0.59449057465591}, { 0.34581177741673,  0.94879421061866},
{ 0.62664209577999, -0.74402970906471}, {-0.77149701404973, -0.33883658042801},
{-0.91592244254432,  0.03687901376713}, {-0.76285492357887, -0.91371867919124},
{ 0.79788337195331, -0.93180971199849}, { 0.54473080610200, -0.11919206037186},
{-0.85639281671058,  0.42429854760451}, {-0.92882402971423,  0.27871809078609},
{-0.11708371046774, -0.99800843444966}, { 0.21356749817493, -0.90716295627033},
{-0.76191692573909,  0.99768118356265}, { 0.98111043100884, -0.95854459734407},
{-0.85913269895572,  0.95766566168880}, {-0.93307242253692,  0.49431757696466},
{ 0.30485754879632, -0.70540034357529}, { 0.85289650925190,  0.46766131791044},
{ 0.91328082618125, -0.99839597361769}, {-0.05890199924154,  0.70741827819497},
{ 0.28398686150148,  0.34633555702188}, { 0.95258164539612, -0.54893416026939},
{-0.78566324168507, -0.75568541079691}, {-0.95789495447877, -0.20423194696966},
{ 0.82411158711197,  0.96654618432562}, {-0.65185446735885, -0.88734990773289},
{-0.93643603134666,  0.99870790442385}, { 0.91427159529618, -0.98290505544444},
{-0.70395684036886,  0.58796798221039}, { 0.00563771969365,  0.61768196727244},
{ 0.89065051931895,  0.52783352697585}, {-0.68683707712762,  0.80806944710339},
{ 0.72165342518718, -0.69259857349564}, {-0.62928247730667,  0.13627037407335},
{ 0.29938434065514, -0.46051329682246}, {-0.91781958879280, -0.74012716684186},
{ 0.99298717043688,  0.40816610075661}, { 0.82368298622748, -0.74036047190173},
{-0.98512833386833, -0.99972330709594}, {-0.95915368242257, -0.99237800466040},
{-0.21411126572790, -0.93424819052545}, {-0.68821476106884, -0.26892306315457},
{ 0.91851997982317,  0.09358228901785}, {-0.96062769559127,  0.36099095133739},
{ 0.51646184922287, -0.71373332873917}, { 0.61130721139669,  0.46950141175917},
{ 0.47336129371299, -0.27333178296162}, { 0.90998308703519,  0.96715662938132},
{ 0.44844799194357,  0.99211574628306}, { 0.66614891079092,  0.96590176169121},
{ 0.74922239129237, -0.89879858826087}, {-0.99571588506485,  0.52785521494349},
{ 0.97401082477563, -0.16855870075190}, { 0.72683747733879, -0.48060774432251},
{ 0.95432193457128,  0.68849603408441}, {-0.72962208425191, -0.76608443420917},
{-0.85359479233537,  0.88738125901579}, {-0.81412430338535, -0.97480768049637},
{-0.87930772356786,  0.74748307690436}, {-0.71573331064977, -0.98570608178923},
{ 0.83524300028228,  0.83702537075163}, {-0.48086065601423, -0.98848504923531},
{ 0.97139128574778,  0.80093621198236}, { 0.51992825347895,  0.80247631400510},
{-0.00848591195325, -0.76670128000486}, {-0.70294374303036,  0.55359910445577},
{-0.95894428168140, -0.43265504344783}, { 0.97079252950321,  0.09325857238682},
{-0.92404293670797,  0.85507704027855}, {-0.69506469500450,  0.98633412625459},
{ 0.26559203620024,  0.73314307966524}, { 0.28038443336943,  0.14537913654427},
{-0.74138124825523,  0.99310339807762}, {-0.01752795995444, -0.82616635284178},
{-0.55126773094930, -0.98898543862153}, { 0.97960898850996, -0.94021446752851},
{-0.99196309146936,  0.67019017358456}, {-0.67684928085260,  0.12631491649378},
{ 0.09140039465500, -0.20537731453108}, {-0.71658965751996, -0.97788200391224},
{ 0.81014640078925,  0.53722648362443}, { 0.40616991671205, -0.26469008598449},
{-0.67680188682972,  0.94502052337695}, { 0.86849774348749, -0.18333598647899},
{-0.99500381284851, -0.02634122068550}, { 0.84329189340667,  0.10406957462213},
{-0.09215968531446,  0.69540012101253}, { 0.99956173327206, -0.12358542001404},
{-0.79732779473535, -0.91582524736159}, { 0.96349973642406,  0.96640458041000},
{-0.79942778496547,  0.64323902822857}, {-0.11566039853896,  0.28587846253726},
{-0.39922954514662,  0.94129601616966}, { 0.99089197565987, -0.92062625581587},
{ 0.28631285179909, -0.91035047143603}, {-0.83302725605608, -0.67330410892084},
{ 0.95404443402072,  0.49162765398743}, {-0.06449863579434,  0.03250560813135},
{-0.99575054486311,  0.42389784469507}, {-0.65501142790847,  0.82546114655624},
{-0.81254441908887, -0.51627234660629}, {-0.99646369485481,  0.84490533520752},
{ 0.00287840603348,  0.64768261158166}, { 0.70176989408455, -0.20453028573322},
{ 0.96361882270190,  0.40706967140989}, {-0.68883758192426,  0.91338958840772},
{-0.34875585502238,  0.71472290693300}, { 0.91980081243087,  0.66507455644919},
{-0.99009048343881,  0.85868021604848}, { 0.68865791458395,  0.55660316809678},
{-0.99484402129368, -0.20052559254934}, { 0.94214511408023, -0.99696425367461},
{-0.67414626793544,  0.49548221180078}, {-0.47339353684664, -0.85904328834047},
{ 0.14323651387360, -0.94145598222488}, {-0.29268293575672,  0.05759224927952},
{ 0.43793861458754, -0.78904969892724}, {-0.36345126374441,  0.64874435357162},
{-0.08750604656825,  0.97686944362527}, {-0.96495267812511, -0.53960305946511},
{ 0.55526940659947,  0.78891523734774}, { 0.73538215752630,  0.96452072373404},
{-0.30889773919437, -0.80664389776860}, { 0.03574995626194, -0.97325616900959},
{ 0.98720684660488,  0.48409133691962}, {-0.81689296271203, -0.90827703628298},
{ 0.67866860118215,  0.81284503870856}, {-0.15808569732583,  0.85279555024382},
{ 0.80723395114371, -0.24717418514605}, { 0.47788757329038, -0.46333147839295},
{ 0.96367554763201,  0.38486749303242}, {-0.99143875716818, -0.24945277239809},
{ 0.83081876925833, -0.94780851414763}, {-0.58753191905341,  0.01290772389163},
{ 0.95538108220960, -0.85557052096538}, {-0.96490920476211, -0.64020970923102},
{-0.97327101028521,  0.12378128133110}, { 0.91400366022124,  0.57972471346930},
{-0.99925837363824,  0.71084847864067}, {-0.86875903507313, -0.20291699203564},
{-0.26240034795124, -0.68264554369108}, {-0.24664412953388, -0.87642273115183},
{ 0.02416275806869,  0.27192914288905}, { 0.82068619590515, -0.85087787994476},
{ 0.88547373760759, -0.89636802901469}, {-0.18173078152226, -0.26152145156800},
{ 0.09355476558534,  0.54845123045604}, {-0.54668414224090,  0.95980774020221},
{ 0.37050990604091, -0.59910140383171}, {-0.70373594262891,  0.91227665827081},
{-0.34600785879594, -0.99441426144200}, {-0.68774481731008, -0.30238837956299},
{-0.26843291251234,  0.83115668004362}, { 0.49072334613242, -0.45359708737775},
{ 0.38975993093975,  0.95515358099121}, {-0.97757125224150,  0.05305894580606},
{-0.17325552859616, -0.92770672250494}, { 0.99948035025744,  0.58285545563426},
{-0.64946246527458,  0.68645507104960}, {-0.12016920576437, -0.57147322153312},
{-0.58947456517751, -0.34847132454388}, {-0.41815140454465,  0.16276422358861},
{ 0.99885650204884,  0.11136095490444}, {-0.56649614128386, -0.90494866361587},
{ 0.94138021032330,  0.35281916733018}, {-0.75725076534641,  0.53650549640587},
{ 0.20541973692630, -0.94435144369918}, { 0.99980371023351,  0.79835913565599},
{ 0.29078277605775,  0.35393777921520}, {-0.62858772103030,  0.38765693387102},
{ 0.43440904467688, -0.98546330463232}, {-0.98298583762390,  0.21021524625209},
{ 0.19513029146934, -0.94239832251867}, {-0.95476662400101,  0.98364554179143},
{ 0.93379635304810, -0.70881994583682}, {-0.85235410573336, -0.08342347966410},
{-0.86425093011245, -0.45795025029466}, { 0.38879779059045,  0.97274429344593},
{ 0.92045124735495, -0.62433652524220}, { 0.89162532251878,  0.54950955570563},
{-0.36834336949252,  0.96458298020975}, { 0.93891760988045, -0.89968353740388},
{ 0.99267657565094, -0.03757034316958}, {-0.94063471614176,  0.41332338538963},
{ 0.99740224117019, -0.16830494996370}, {-0.35899413170555, -0.46633226649613},
{ 0.05237237274947, -0.25640361602661}, { 0.36703583957424, -0.38653265641875},
{ 0.91653180367913, -0.30587628726597}, { 0.69000803499316,  0.90952171386132},
{-0.38658751133527,  0.99501571208985}, {-0.29250814029851,  0.37444994344615},
{-0.60182204677608,  0.86779651036123}, {-0.97418588163217,  0.96468523666475},
{ 0.88461574003963,  0.57508405276414}, { 0.05198933055162,  0.21269661669964},
{-0.53499621979720,  0.97241553731237}, {-0.49429560226497,  0.98183865291903},
{-0.98935142339139, -0.40249159006933}, {-0.98081380091130, -0.72856895534041},
{-0.27338148835532,  0.99950922447209}, { 0.06310802338302, -0.54539587529618},
{-0.20461677199539, -0.14209977628489}, { 0.66223843141647,  0.72528579940326},
{-0.84764345483665,  0.02372316801261}, {-0.89039863483811,  0.88866581484602},
{ 0.95903308477986,  0.76744927173873}, { 0.73504123909879, -0.03747203173192},
{-0.31744434966056, -0.36834111883652}, {-0.34110827591623,  0.40211222807691},
{ 0.47803883714199, -0.39423219786288}, { 0.98299195879514,  0.01989791390047},
{-0.30963073129751, -0.18076720599336}, { 0.99992588229018, -0.26281872094289},
{-0.93149731080767, -0.98313162570490}, { 0.99923472302773, -0.80142993767554},
{-0.26024169633417, -0.75999759855752}, {-0.35712514743563,  0.19298963768574},
{-0.99899084509530,  0.74645156992493}, { 0.86557171579452,  0.55593866696299},
{ 0.33408042438752,  0.86185953874709}, { 0.99010736374716,  0.04602397576623},
{-0.66694269691195, -0.91643611810148}, { 0.64016792079480,  0.15649530836856},
{ 0.99570534804836,  0.45844586038111}, {-0.63431466947340,  0.21079116459234},
{-0.07706847005931, -0.89581437101329}, { 0.98590090577724,  0.88241721133981},
{ 0.80099335254678, -0.36851896710853}, { 0.78368131392666,  0.45506999802597},
{ 0.08707806671691,  0.80938994918745}, {-0.86811883080712,  0.39347308654705},
{-0.39466529740375, -0.66809432114456}, { 0.97875325649683, -0.72467840967746},
{-0.95038560288864,  0.89563219587625}, { 0.17005239424212,  0.54683053962658},
{-0.76910792026848, -0.96226617549298}, { 0.99743281016846,  0.42697157037567},
{ 0.95437383549973,  0.97002324109952}, { 0.99578905365569, -0.54106826257356},
{ 0.28058259829990, -0.85361420634036}, { 0.85256524470573, -0.64567607735589},
{-0.50608540105128, -0.65846015480300}, {-0.97210735183243, -0.23095213067791},
{ 0.95424048234441, -0.99240147091219}, {-0.96926570524023,  0.73775654896574},
{ 0.30872163214726,  0.41514960556126}, {-0.24523839572639,  0.63206633394807},
{-0.33813265086024, -0.38661779441897}, {-0.05826828420146, -0.06940774188029},
{-0.22898461455054,  0.97054853316316}, {-0.18509915019881,  0.47565762892084},
{-0.10488238045009, -0.87769947402394}, {-0.71886586182037,  0.78030982480538},
{ 0.99793873738654,  0.90041310491497}, { 0.57563307626120, -0.91034337352097},
{ 0.28909646383717,  0.96307783970534}, { 0.42188998312520,  0.48148651230437},
{ 0.93335049681047, -0.43537023883588}, {-0.97087374418267,  0.86636445711364},
{ 0.36722871286923,  0.65291654172961}, {-0.81093025665696,  0.08778370229363},
{-0.26240603062237, -0.92774095379098}, { 0.83996497984604,  0.55839849139647},
{-0.99909615720225, -0.96024605713970}, { 0.74649464155061,  0.12144893606462},
{-0.74774595569805, -0.26898062008959}, { 0.95781667469567, -0.79047927052628},
{ 0.95472308713099, -0.08588776019550}, { 0.48708332746299,  0.99999041579432},
{ 0.46332038247497,  0.10964126185063}, {-0.76497004940162,  0.89210929242238},
{ 0.57397389364339,  0.35289703373760}, { 0.75374316974495,  0.96705214651335},
{-0.59174397685714, -0.89405370422752}, { 0.75087906691890, -0.29612672982396},
{-0.98607857336230,  0.25034911730023}, {-0.40761056640505, -0.90045573444695},
{ 0.66929266740477,  0.98629493401748}, {-0.97463695257310, -0.00190223301301},
{ 0.90145509409859,  0.99781390365446}, {-0.87259289048043,  0.99233587353666},
{-0.91529461447692, -0.15698707534206}, {-0.03305738840705, -0.37205262859764},
{ 0.07223051368337, -0.88805001733626}, { 0.99498012188353,  0.97094358113387},
{-0.74904939500519,  0.99985483641521}, { 0.04585228574211,  0.99812337444082},
{-0.89054954257993, -0.31791913188064}, {-0.83782144651251,  0.97637632547466},
{ 0.33454804933804, -0.86231516800408}, {-0.99707579362824,  0.93237990079441},
{-0.22827527843994,  0.18874759397997}, { 0.67248046289143, -0.03646211390569},
{-0.05146538187944, -0.92599700120679}, { 0.99947295749905,  0.93625229707912},
{ 0.66951124390363,  0.98905825623893}, {-0.99602956559179, -0.44654715757688},
{ 0.82104905483590,  0.99540741724928}, { 0.99186510988782,  0.72023001312947},
{-0.65284592392918,  0.52186723253637}, { 0.93885443798188, -0.74895312615259},
{ 0.96735248738388,  0.90891816978629}, {-0.22225968841114,  0.57124029781228},
{-0.44132783753414, -0.92688840659280}, {-0.85694974219574,  0.88844532719844},
{ 0.91783042091762, -0.46356892383970}, { 0.72556974415690, -0.99899555770747},
{-0.99711581834508,  0.58211560180426}, { 0.77638976371966,  0.94321834873819},
{ 0.07717324253925,  0.58638399856595}, {-0.56049829194163,  0.82522301569036},
{ 0.98398893639988,  0.39467440420569}, { 0.47546946844938,  0.68613044836811},
{ 0.65675089314631,  0.18331637134880}, { 0.03273375457980, -0.74933109564108},
{-0.38684144784738,  0.51337349030406}, {-0.97346267944545, -0.96549364384098},
{-0.53282156061942, -0.91423265091354}, { 0.99817310731176,  0.61133572482148},
{-0.50254500772635, -0.88829338134294}, { 0.01995873238855,  0.85223515096765},
{ 0.99930381973804,  0.94578896296649}, { 0.82907767600783, -0.06323442598128},
{-0.58660709669728,  0.96840773806582}, {-0.17573736667267, -0.48166920859485},
{ 0.83434292401346, -0.13023450646997}, { 0.05946491307025,  0.20511047074866},
{ 0.81505484574602, -0.94685947861369}, {-0.44976380954860,  0.40894572671545},
{-0.89746474625671,  0.99846578838537}, { 0.39677256130792, -0.74854668609359},
{-0.07588948563079,  0.74096214084170}, { 0.76343198951445,  0.41746629422634},
{-0.74490104699626,  0.94725911744610}, { 0.64880119792759,  0.41336660830571},
{ 0.62319537462542, -0.93098313552599}, { 0.42215817594807, -0.07712787385208},
{ 0.02704554141885, -0.05417518053666}, { 0.80001773566818,  0.91542195141039},
{-0.79351832348816, -0.36208897989136}, { 0.63872359151636,  0.08128252493444},
{ 0.52890520960295,  0.60048872455592}, { 0.74238552914587,  0.04491915291044},
{ 0.99096131449250, -0.19451182854402}, {-0.80412329643109, -0.88513818199457},
{-0.64612616129736,  0.72198674804544}, { 0.11657770663191, -0.83662833815041},
{-0.95053182488101, -0.96939905138082}, {-0.62228872928622,  0.82767262846661},
{ 0.03004475787316, -0.99738896333384}, {-0.97987214341034,  0.36526129686425},
{-0.99986980746200, -0.36021610299715}, { 0.89110648599879, -0.97894250343044},
{ 0.10407960510582,  0.77357793811619}, { 0.95964737821728, -0.35435818285502},
{ 0.50843233159162,  0.96107691266205}, { 0.17006334670615, -0.76854025314829},
{ 0.25872675063360,  0.99893303933816}, {-0.01115998681937,  0.98496019742444},
{-0.79598702973261,  0.97138411318894}, {-0.99264708948101, -0.99542822402536},
{-0.99829663752818,  0.01877138824311}, {-0.70801016548184,  0.33680685948117},
{-0.70467057786826,  0.93272777501857}, { 0.99846021905254, -0.98725746254433},
{-0.63364968534650, -0.16473594423746}, {-0.16258217500792, -0.95939125400802},
{-0.43645594360633, -0.94805030113284}, {-0.99848471702976,  0.96245166923809},
{-0.16796458968998, -0.98987511890470}, {-0.87979225745213, -0.71725725041680},
{ 0.44183099021786, -0.93568974498761}, { 0.93310180125532, -0.99913308068246},
{-0.93941931782002, -0.56409379640356}, {-0.88590003188677,  0.47624600491382},
{ 0.99971463703691, -0.83889954253462}, {-0.75376385639978,  0.00814643438625},
{ 0.93887685615875, -0.11284528204636}, { 0.85126435782309,  0.52349251543547},
{ 0.39701421446381,  0.81779634174316}, {-0.37024464187437, -0.87071656222959},
{-0.36024828242896,  0.34655735648287}, {-0.93388812549209, -0.84476541096429},
{-0.65298804552119, -0.18439575450921}, { 0.11960319006843,  0.99899346780168},
{ 0.94292565553160,  0.83163906518293}, { 0.75081145286948, -0.35533223142265},
{ 0.56721979748394, -0.24076836414499}, { 0.46857766746029, -0.30140233457198},
{ 0.97312313923635, -0.99548191630031}, {-0.38299976567017,  0.98516909715427},
{ 0.41025800019463,  0.02116736935734}, { 0.09638062008048,  0.04411984381457},
{-0.85283249275397,  0.91475563922421}, { 0.88866808958124, -0.99735267083226},
{-0.48202429536989, -0.96805608884164}, { 0.27572582416567,  0.58634753335832},
{-0.65889129659168,  0.58835634138583}, { 0.98838086953732,  0.99994349600236},
{-0.20651349620689,  0.54593044066355}, {-0.62126416356920, -0.59893681700392},
{ 0.20320105410437, -0.86879180355289}, {-0.97790548600584,  0.96290806999242},
{ 0.11112534735126,  0.21484763313301}, {-0.41368337314182,  0.28216837680365},
{ 0.24133038992960,  0.51294362630238}, {-0.66393410674885, -0.08249679629081},
{-0.53697829178752, -0.97649903936228}, {-0.97224737889348,  0.22081333579837},
{ 0.87392477144549, -0.12796173740361}, { 0.19050361015753,  0.01602615387195},
{-0.46353441212724, -0.95249041539006}, {-0.07064096339021, -0.94479803205886},
{-0.92444085484466, -0.10457590187436}, {-0.83822593578728, -0.01695043208885},
{ 0.75214681811150, -0.99955681042665}, {-0.42102998829339,  0.99720941999394},
{-0.72094786237696, -0.35008961934255}, { 0.78843311019251,  0.52851398958271},
{ 0.97394027897442, -0.26695944086561}, { 0.99206463477946, -0.57010120849429},
{ 0.76789609461795, -0.76519356730966}, {-0.82002421836409, -0.73530179553767},
{ 0.81924990025724,  0.99698425250579}, {-0.26719850873357,  0.68903369776193},
{-0.43311260380975,  0.85321815947490}, { 0.99194979673836,  0.91876249766422},
{-0.80692001248487, -0.32627540663214}, { 0.43080003649976, -0.21919095636638},
{ 0.67709491937357, -0.95478075822906}, { 0.56151770568316, -0.70693811747778},
{ 0.10831862810749, -0.08628837174592}, { 0.91229417540436, -0.65987351408410},
{-0.48972893932274,  0.56289246362686}, {-0.89033658689697, -0.71656563987082},
{ 0.65269447475094,  0.65916004833932}, { 0.67439478141121, -0.81684380846796},
{-0.47770832416973, -0.16789556203025}, {-0.99715979260878, -0.93565784007648},
{-0.90889593602546,  0.62034397054380}, {-0.06618622548177, -0.23812217221359},
{ 0.99430266919728,  0.18812555317553}, { 0.97686402381843, -0.28664534366620},
{ 0.94813650221268, -0.97506640027128}, {-0.95434497492853, -0.79607978501983},
{-0.49104783137150,  0.32895214359663}, { 0.99881175120751,  0.88993983831354},
{ 0.50449166760303, -0.85995072408434}, { 0.47162891065108, -0.18680204049569},
{-0.62081581361840,  0.75000676218956}, {-0.43867015250812,  0.99998069244322},
{ 0.98630563232075, -0.53578899600662}, {-0.61510362277374, -0.89515019899997},
{-0.03841517601843, -0.69888815681179}, {-0.30102157304644, -0.07667808922205},
{ 0.41881284182683,  0.02188098922282}, {-0.86135454941237,  0.98947480909359},
{ 0.67226861393788, -0.13494389011014}, {-0.70737398842068, -0.76547349325992},
{ 0.94044946687963,  0.09026201157416}, {-0.82386352534327,  0.08924768823676},
{-0.32070666698656,  0.50143421908753}, { 0.57593163224487, -0.98966422921509},
{-0.36326018419965,  0.07440243123228}, { 0.99979044674350, -0.14130287347405},
{-0.92366023326932, -0.97979298068180}, {-0.44607178518598, -0.54233252016394},
{ 0.44226800932956,  0.71326756742752}, { 0.03671907158312,  0.63606389366675},
{ 0.52175424682195, -0.85396826735705}, {-0.94701139690956, -0.01826348194255},
{-0.98759606946049,  0.82288714303073}, { 0.87434794743625,  0.89399495655433},
{-0.93412041758744,  0.41374052024363}, { 0.96063943315511,  0.93116709541280},
{ 0.97534253457837,  0.86150930812689}, { 0.99642466504163,  0.70190043427512},
{-0.94705089665984, -0.29580042814306}, { 0.91599807087376, -0.98147830385781},
};

#endif /* AVCODEC_AACSBRDATA_H */
