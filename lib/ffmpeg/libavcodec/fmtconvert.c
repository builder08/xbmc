/*
 * Format Conversion Utils
 * Copyright (c) 2000, 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
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

#include "avcodec.h"
#include "fmtconvert.h"

static void int32_to_float_fmul_scalar_c(float *dst, const int *src, float mul, int len){
    int i;
    for(i=0; i<len; i++)
        dst[i] = src[i] * mul;
}

static av_always_inline int float_to_int16_one(const float *src){
    return av_clip_int16(lrintf(*src));
}

static void float_to_int16_c(int16_t *dst, const float *src, long len)
{
    int i;
    for(i=0; i<len; i++)
        dst[i] = float_to_int16_one(src+i);
}

static void float_to_int16_interleave_c(int16_t *dst, const float **src,
                                        long len, int channels)
{
    int i,j,c;
    if(channels==2){
        for(i=0; i<len; i++){
            dst[2*i]   = float_to_int16_one(src[0]+i);
            dst[2*i+1] = float_to_int16_one(src[1]+i);
        }
    }else{
        for(c=0; c<channels; c++)
            for(i=0, j=c; i<len; i++, j+=channels)
                dst[j] = float_to_int16_one(src[c]+i);
    }
}

av_cold void ff_fmt_convert_init(FmtConvertContext *c, AVCodecContext *avctx)
{
    c->int32_to_float_fmul_scalar = int32_to_float_fmul_scalar_c;
    c->float_to_int16             = float_to_int16_c;
    c->float_to_int16_interleave  = float_to_int16_interleave_c;

    if (ARCH_ARM) ff_fmt_convert_init_arm(c, avctx);
    if (ARCH_PPC) ff_fmt_convert_init_ppc(c, avctx);
    if (HAVE_MMX) ff_fmt_convert_init_x86(c, avctx);
}
