/*
 * AC-3 Audio Decoder
 * This code was developed as part of Google Summer of Code 2006.
 * E-AC-3 support was added as part of Google Summer of Code 2007.
 *
 * Copyright (c) 2006 Kartikey Mahendra BHATT (bhattkm at gmail dot com).
 * Copyright (c) 2007-2008 Bartlomiej Wolowiec <bartek.wolowiec@gmail.com>
 * Copyright (c) 2007 Justin Ruggles <justin.ruggles@gmail.com>
 *
 * Portions of this code are derived from liba52
 * http://liba52.sourceforge.net
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <string.h>

#include "libavutil/crc.h"
#include "ac3_parser.h"
#include "ac3dec.h"
#include "ac3dec_data.h"

/** Large enough for maximum possible frame size when the specification limit is ignored */
#define AC3_FRAME_BUFFER_SIZE 32768

/**
 * table for ungrouping 3 values in 7 bits.
 * used for exponents and bap=2 mantissas
 */
static uint8_t ungroup_3_in_7_bits_tab[128][3];


/** tables for ungrouping mantissas */
static int b1_mantissas[32][3];
static int b2_mantissas[128][3];
static int b3_mantissas[8];
static int b4_mantissas[128][2];
static int b5_mantissas[16];

/**
 * Quantization table: levels for symmetric. bits for asymmetric.
 * reference: Table 7.18 Mapping of bap to Quantizer
 */
static const uint8_t quantization_tab[16] = {
    0, 3, 5, 7, 11, 15,
    5, 6, 7, 8, 9, 10, 11, 12, 14, 16
};

/** dynamic range table. converts codes to scale factors. */
static float dynamic_range_tab[256];

/** Adjustments in dB gain */
#define LEVEL_PLUS_3DB          1.4142135623730950
#define LEVEL_PLUS_1POINT5DB    1.1892071150027209
#define LEVEL_MINUS_1POINT5DB   0.8408964152537145
#define LEVEL_MINUS_3DB         0.7071067811865476
#define LEVEL_MINUS_4POINT5DB   0.5946035575013605
#define LEVEL_MINUS_6DB         0.5000000000000000
#define LEVEL_MINUS_9DB         0.3535533905932738
#define LEVEL_ZERO              0.0000000000000000
#define LEVEL_ONE               1.0000000000000000

static const float gain_levels[9] = {
    LEVEL_PLUS_3DB,
    LEVEL_PLUS_1POINT5DB,
    LEVEL_ONE,
    LEVEL_MINUS_1POINT5DB,
    LEVEL_MINUS_3DB,
    LEVEL_MINUS_4POINT5DB,
    LEVEL_MINUS_6DB,
    LEVEL_ZERO,
    LEVEL_MINUS_9DB
};

/**
 * Table for center mix levels
 * reference: Section 5.4.2.4 cmixlev
 */
static const uint8_t center_levels[4] = { 4, 5, 6, 5 };

/**
 * Table for surround mix levels
 * reference: Section 5.4.2.5 surmixlev
 */
static const uint8_t surround_levels[4] = { 4, 6, 7, 6 };

/**
 * Table for default stereo downmixing coefficients
 * reference: Section 7.8.2 Downmixing Into Two Channels
 */
static const uint8_t ac3_default_coeffs[8][5][2] = {
    { { 2, 7 }, { 7, 2 },                               },
    { { 4, 4 },                                         },
    { { 2, 7 }, { 7, 2 },                               },
    { { 2, 7 }, { 5, 5 }, { 7, 2 },                     },
    { { 2, 7 }, { 7, 2 }, { 6, 6 },                     },
    { { 2, 7 }, { 5, 5 }, { 7, 2 }, { 8, 8 },           },
    { { 2, 7 }, { 7, 2 }, { 6, 7 }, { 7, 6 },           },
    { { 2, 7 }, { 5, 5 }, { 7, 2 }, { 6, 7 }, { 7, 6 }, },
};

/**
 * Symmetrical Dequantization
 * reference: Section 7.3.3 Expansion of Mantissas for Symmetrical Quantization
 *            Tables 7.19 to 7.23
 */
static inline int
symmetric_dequant(int code, int levels)
{
    return ((code - (levels >> 1)) << 24) / levels;
}

/*
 * Initialize tables at runtime.
 */
static av_cold void ac3_tables_init(void)
{
    int i;

    /* generate table for ungrouping 3 values in 7 bits
       reference: Section 7.1.3 Exponent Decoding */
    for(i=0; i<128; i++) {
        ungroup_3_in_7_bits_tab[i][0] =  i / 25;
        ungroup_3_in_7_bits_tab[i][1] = (i % 25) / 5;
        ungroup_3_in_7_bits_tab[i][2] = (i % 25) % 5;
    }

    /* generate grouped mantissa tables
       reference: Section 7.3.5 Ungrouping of Mantissas */
    for(i=0; i<32; i++) {
        /* bap=1 mantissas */
        b1_mantissas[i][0] = symmetric_dequant(ff_ac3_ungroup_3_in_5_bits_tab[i][0], 3);
        b1_mantissas[i][1] = symmetric_dequant(ff_ac3_ungroup_3_in_5_bits_tab[i][1], 3);
        b1_mantissas[i][2] = symmetric_dequant(ff_ac3_ungroup_3_in_5_bits_tab[i][2], 3);
    }
    for(i=0; i<128; i++) {
        /* bap=2 mantissas */
        b2_mantissas[i][0] = symmetric_dequant(ungroup_3_in_7_bits_tab[i][0], 5);
        b2_mantissas[i][1] = symmetric_dequant(ungroup_3_in_7_bits_tab[i][1], 5);
        b2_mantissas[i][2] = symmetric_dequant(ungroup_3_in_7_bits_tab[i][2], 5);

        /* bap=4 mantissas */
        b4_mantissas[i][0] = symmetric_dequant(i / 11, 11);
        b4_mantissas[i][1] = symmetric_dequant(i % 11, 11);
    }
    /* generate ungrouped mantissa tables
       reference: Tables 7.21 and 7.23 */
    for(i=0; i<7; i++) {
        /* bap=3 mantissas */
        b3_mantissas[i] = symmetric_dequant(i, 7);
    }
    for(i=0; i<15; i++) {
        /* bap=5 mantissas */
        b5_mantissas[i] = symmetric_dequant(i, 15);
    }

    /* generate dynamic range table
       reference: Section 7.7.1 Dynamic Range Control */
    for(i=0; i<256; i++) {
        int v = (i >> 5) - ((i >> 7) << 3) - 5;
        dynamic_range_tab[i] = powf(2.0f, v) * ((i & 0x1F) | 0x20);
    }
}


/**
 * AVCodec initialization
 */
static av_cold int ac3_decode_init(AVCodecContext *avctx)
{
    AC3DecodeContext *s = avctx->priv_data;
    s->avctx = avctx;

    ac3_common_init();
    ac3_tables_init();
    ff_mdct_init(&s->imdct_256, 8, 1);
    ff_mdct_init(&s->imdct_512, 9, 1);
    ff_kbd_window_init(s->window, 5.0, 256);
    dsputil_init(&s->dsp, avctx);
    av_lfg_init(&s->dith_state, 0);

    /* set bias values for float to int16 conversion */
    if(s->dsp.float_to_int16_interleave == ff_float_to_int16_interleave_c) {
        s->add_bias = 385.0f;
        s->mul_bias = 1.0f;
    } else {
        s->add_bias = 0.0f;
        s->mul_bias = 32767.0f;
    }

    /* allow downmixing to stereo or mono */
    if (avctx->channels > 0 && avctx->request_channels > 0 &&
            avctx->request_channels < avctx->channels &&
            avctx->request_channels <= 2) {
        avctx->channels = avctx->request_channels;
    }
    s->downmixed = 1;

    /* allocate context input buffer */
    if (avctx->error_recognition >= FF_ER_CAREFUL) {
        s->input_buffer = av_mallocz(AC3_FRAME_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
        if (!s->input_buffer)
            return AVERROR_NOMEM;
    }

    avctx->sample_fmt = SAMPLE_FMT_S16;
    return 0;
}

/**
 * Parse the 'sync info' and 'bit stream info' from the AC-3 bitstream.
 * GetBitContext within AC3DecodeContext must point to
 * the start of the synchronized AC-3 bitstream.
 */
static int ac3_parse_header(AC3DecodeContext *s)
{
    GetBitContext *gbc = &s->gbc;
    int i;

    /* read the rest of the bsi. read twice for dual mono mode. */
    i = !(s->channel_mode);
    do {
        skip_bits(gbc, 5); // skip dialog normalization
        if (get_bits1(gbc))
            skip_bits(gbc, 8); //skip compression
        if (get_bits1(gbc))
            skip_bits(gbc, 8); //skip language code
        if (get_bits1(gbc))
            skip_bits(gbc, 7); //skip audio production information
    } while (i--);

    skip_bits(gbc, 2); //skip copyright bit and original bitstream bit

    /* skip the timecodes (or extra bitstream information for Alternate Syntax)
       TODO: read & use the xbsi1 downmix levels */
    if (get_bits1(gbc))
        skip_bits(gbc, 14); //skip timecode1 / xbsi1
    if (get_bits1(gbc))
        skip_bits(gbc, 14); //skip timecode2 / xbsi2

    /* skip additional bitstream info */
    if (get_bits1(gbc)) {
        i = get_bits(gbc, 6);
        do {
            skip_bits(gbc, 8);
        } while(i--);
    }

    return 0;
}

/**
 * Common function to parse AC-3 or E-AC-3 frame header
 */
static int parse_frame_header(AC3DecodeContext *s)
{
    AC3HeaderInfo hdr;
    int err;

    err = ff_ac3_parse_header(&s->gbc, &hdr);
    if(err)
        return err;

    /* get decoding parameters from header info */
    s->bit_alloc_params.sr_code     = hdr.sr_code;
    s->channel_mode                 = hdr.channel_mode;
    s->lfe_on                       = hdr.lfe_on;
    s->bit_alloc_params.sr_shift    = hdr.sr_shift;
    s->sample_rate                  = hdr.sample_rate;
    s->bit_rate                     = hdr.bit_rate;
    s->channels                     = hdr.channels;
    s->fbw_channels                 = s->channels - s->lfe_on;
    s->lfe_ch                       = s->fbw_channels + 1;
    s->frame_size                   = hdr.frame_size;
    s->center_mix_level             = hdr.center_mix_level;
    s->surround_mix_level           = hdr.surround_mix_level;
    s->num_blocks                   = hdr.num_blocks;
    s->frame_type                   = hdr.frame_type;
    s->substreamid                  = hdr.substreamid;

    if(s->lfe_on) {
        s->start_freq[s->lfe_ch] = 0;
        s->end_freq[s->lfe_ch] = 7;
        s->num_exp_groups[s->lfe_ch] = 2;
        s->channel_in_cpl[s->lfe_ch] = 0;
    }

    if (hdr.bitstream_id <= 10) {
        s->eac3                  = 0;
        s->snr_offset_strategy   = 2;
        s->block_switch_syntax   = 1;
        s->dither_flag_syntax    = 1;
        s->bit_allocation_syntax = 1;
        s->fast_gain_syntax      = 0;
        s->first_cpl_leak        = 0;
        s->dba_syntax            = 1;
        s->skip_syntax           = 1;
        memset(s->channel_uses_aht, 0, sizeof(s->channel_uses_aht));
        return ac3_parse_header(s);
    } else {
        s->eac3 = 1;
        return ff_eac3_parse_header(s);
    }
}

/**
 * Set stereo downmixing coefficients based on frame header info.
 * reference: Section 7.8.2 Downmixing Into Two Channels
 */
static void set_downmix_coeffs(AC3DecodeContext *s)
{
    int i;
    float cmix = gain_levels[center_levels[s->center_mix_level]];
    float smix = gain_levels[surround_levels[s->surround_mix_level]];
    float norm0, norm1;

    for(i=0; i<s->fbw_channels; i++) {
        s->downmix_coeffs[i][0] = gain_levels[ac3_default_coeffs[s->channel_mode][i][0]];
        s->downmix_coeffs[i][1] = gain_levels[ac3_default_coeffs[s->channel_mode][i][1]];
    }
    if(s->channel_mode > 1 && s->channel_mode & 1) {
        s->downmix_coeffs[1][0] = s->downmix_coeffs[1][1] = cmix;
    }
    if(s->channel_mode == AC3_CHMODE_2F1R || s->channel_mode == AC3_CHMODE_3F1R) {
        int nf = s->channel_mode - 2;
        s->downmix_coeffs[nf][0] = s->downmix_coeffs[nf][1] = smix * LEVEL_MINUS_3DB;
    }
    if(s->channel_mode == AC3_CHMODE_2F2R || s->channel_mode == AC3_CHMODE_3F2R) {
        int nf = s->channel_mode - 4;
        s->downmix_coeffs[nf][0] = s->downmix_coeffs[nf+1][1] = smix;
    }

    /* renormalize */
    norm0 = norm1 = 0.0;
    for(i=0; i<s->fbw_channels; i++) {
        norm0 += s->downmix_coeffs[i][0];
        norm1 += s->downmix_coeffs[i][1];
    }
    norm0 = 1.0f / norm0;
    norm1 = 1.0f / norm1;
    for(i=0; i<s->fbw_channels; i++) {
        s->downmix_coeffs[i][0] *= norm0;
        s->downmix_coeffs[i][1] *= norm1;
    }

    if(s->output_mode == AC3_CHMODE_MONO) {
        for(i=0; i<s->fbw_channels; i++)
            s->downmix_coeffs[i][0] = (s->downmix_coeffs[i][0] + s->downmix_coeffs[i][1]) * LEVEL_MINUS_3DB;
    }
}

/**
 * Decode the grouped exponents according to exponent strategy.
 * reference: Section 7.1.3 Exponent Decoding
 */
static void decode_exponents(GetBitContext *gbc, int exp_strategy, int ngrps,
                             uint8_t absexp, int8_t *dexps)
{
    int i, j, grp, group_size;
    int dexp[256];
    int expacc, prevexp;

    /* unpack groups */
    group_size = exp_strategy + (exp_strategy == EXP_D45);
    for(grp=0,i=0; grp<ngrps; grp++) {
        expacc = get_bits(gbc, 7);
        dexp[i++] = ungroup_3_in_7_bits_tab[expacc][0];
        dexp[i++] = ungroup_3_in_7_bits_tab[expacc][1];
        dexp[i++] = ungroup_3_in_7_bits_tab[expacc][2];
    }

    /* convert to absolute exps and expand groups */
    prevexp = absexp;
    for(i=0; i<ngrps*3; i++) {
        prevexp = av_clip(prevexp + dexp[i]-2, 0, 24);
        for(j=0; j<group_size; j++) {
            dexps[(i*group_size)+j] = prevexp;
        }
    }
}

/**
 * Generate transform coefficients for each coupled channel in the coupling
 * range using the coupling coefficients and coupling coordinates.
 * reference: Section 7.4.3 Coupling Coordinate Format
 */
static void calc_transform_coeffs_cpl(AC3DecodeContext *s)
{
    int i, j, ch, bnd, subbnd;

    subbnd = -1;
    i = s->start_freq[CPL_CH];
    for(bnd=0; bnd<s->num_cpl_bands; bnd++) {
        do {
            subbnd++;
            for(j=0; j<12; j++) {
                for(ch=1; ch<=s->fbw_channels; ch++) {
                    if(s->channel_in_cpl[ch]) {
                        s->fixed_coeffs[ch][i] = ((int64_t)s->fixed_coeffs[CPL_CH][i] * (int64_t)s->cpl_coords[ch][bnd]) >> 23;
                        if (ch == 2 && s->phase_flags[bnd])
                            s->fixed_coeffs[ch][i] = -s->fixed_coeffs[ch][i];
                    }
                }
                i++;
            }
        } while(s->cpl_band_struct[subbnd]);
    }
}

/**
 * Grouped mantissas for 3-level 5-level and 11-level quantization
 */
typedef struct {
    int b1_mant[3];
    int b2_mant[3];
    int b4_mant[2];
    int b1ptr;
    int b2ptr;
    int b4ptr;
} mant_groups;

/**
 * Decode the transform coefficients for a particular channel
 * reference: Section 7.3 Quantization and Decoding of Mantissas
 */
static void ac3_decode_transform_coeffs_ch(AC3DecodeContext *s, int ch_index, mant_groups *m)
{
    GetBitContext *gbc = &s->gbc;
    int i, gcode, tbap, start, end;
    uint8_t *exps;
    uint8_t *bap;
    int *coeffs;

    exps = s->dexps[ch_index];
    bap = s->bap[ch_index];
    coeffs = s->fixed_coeffs[ch_index];
    start = s->start_freq[ch_index];
    end = s->end_freq[ch_index];

    for (i = start; i < end; i++) {
        tbap = bap[i];
        switch (tbap) {
            case 0:
                coeffs[i] = (av_lfg_get(&s->dith_state) & 0x7FFFFF) - 0x400000;
                break;

            case 1:
                if(m->b1ptr > 2) {
                    gcode = get_bits(gbc, 5);
                    m->b1_mant[0] = b1_mantissas[gcode][0];
                    m->b1_mant[1] = b1_mantissas[gcode][1];
                    m->b1_mant[2] = b1_mantissas[gcode][2];
                    m->b1ptr = 0;
                }
                coeffs[i] = m->b1_mant[m->b1ptr++];
                break;

            case 2:
                if(m->b2ptr > 2) {
                    gcode = get_bits(gbc, 7);
                    m->b2_mant[0] = b2_mantissas[gcode][0];
                    m->b2_mant[1] = b2_mantissas[gcode][1];
                    m->b2_mant[2] = b2_mantissas[gcode][2];
                    m->b2ptr = 0;
                }
                coeffs[i] = m->b2_mant[m->b2ptr++];
                break;

            case 3:
                coeffs[i] = b3_mantissas[get_bits(gbc, 3)];
                break;

            case 4:
                if(m->b4ptr > 1) {
                    gcode = get_bits(gbc, 7);
                    m->b4_mant[0] = b4_mantissas[gcode][0];
                    m->b4_mant[1] = b4_mantissas[gcode][1];
                    m->b4ptr = 0;
                }
                coeffs[i] = m->b4_mant[m->b4ptr++];
                break;

            case 5:
                coeffs[i] = b5_mantissas[get_bits(gbc, 4)];
                break;

            default: {
                /* asymmetric dequantization */
                int qlevel = quantization_tab[tbap];
                coeffs[i] = get_sbits(gbc, qlevel) << (24 - qlevel);
                break;
            }
        }
        coeffs[i] >>= exps[i];
    }
}

/**
 * Remove random dithering from coefficients with zero-bit mantissas
 * reference: Section 7.3.4 Dither for Zero Bit Mantissas (bap=0)
 */
static void remove_dithering(AC3DecodeContext *s) {
    int ch, i;
    int end=0;
    int *coeffs;
    uint8_t *bap;

    for(ch=1; ch<=s->fbw_channels; ch++) {
        if(!s->dither_flag[ch]) {
            coeffs = s->fixed_coeffs[ch];
            bap = s->bap[ch];
            if(s->channel_in_cpl[ch])
                end = s->start_freq[CPL_CH];
            else
                end = s->end_freq[ch];
            for(i=0; i<end; i++) {
                if(!bap[i])
                    coeffs[i] = 0;
            }
            if(s->channel_in_cpl[ch]) {
                bap = s->bap[CPL_CH];
                for(; i<s->end_freq[CPL_CH]; i++) {
                    if(!bap[i])
                        coeffs[i] = 0;
                }
            }
        }
    }
}

static void decode_transform_coeffs_ch(AC3DecodeContext *s, int blk, int ch,
                                    mant_groups *m)
{
    if (!s->channel_uses_aht[ch]) {
        ac3_decode_transform_coeffs_ch(s, ch, m);
    } else {
        /* if AHT is used, mantissas for all blocks are encoded in the first
           block of the frame. */
        int bin;
        if (!blk)
            ff_eac3_decode_transform_coeffs_aht_ch(s, ch);
        for (bin = s->start_freq[ch]; bin < s->end_freq[ch]; bin++) {
            s->fixed_coeffs[ch][bin] = s->pre_mantissa[ch][bin][blk] >> s->dexps[ch][bin];
        }
    }
}

/**
 * Decode the transform coefficients.
 */
static void decode_transform_coeffs(AC3DecodeContext *s, int blk)
{
    int ch, end;
    int got_cplchan = 0;
    mant_groups m;

    m.b1ptr = m.b2ptr = m.b4ptr = 3;

    for (ch = 1; ch <= s->channels; ch++) {
        /* transform coefficients for full-bandwidth channel */
        decode_transform_coeffs_ch(s, blk, ch, &m);
        /* tranform coefficients for coupling channel come right after the
           coefficients for the first coupled channel*/
        if (s->channel_in_cpl[ch])  {
            if (!got_cplchan) {
                decode_transform_coeffs_ch(s, blk, CPL_CH, &m);
                calc_transform_coeffs_cpl(s);
                got_cplchan = 1;
            }
            end = s->end_freq[CPL_CH];
        } else {
            end = s->end_freq[ch];
        }
        do
            s->fixed_coeffs[ch][end] = 0;
        while(++end < 256);
    }

    /* zero the dithered coefficients for appropriate channels */
    remove_dithering(s);
}

/**
 * Stereo rematrixing.
 * reference: Section 7.5.4 Rematrixing : Decoding Technique
 */
static void do_rematrixing(AC3DecodeContext *s)
{
    int bnd, i;
    int end, bndend;
    int tmp0, tmp1;

    end = FFMIN(s->end_freq[1], s->end_freq[2]);

    for(bnd=0; bnd<s->num_rematrixing_bands; bnd++) {
        if(s->rematrixing_flags[bnd]) {
            bndend = FFMIN(end, ff_ac3_rematrix_band_tab[bnd+1]);
            for(i=ff_ac3_rematrix_band_tab[bnd]; i<bndend; i++) {
                tmp0 = s->fixed_coeffs[1][i];
                tmp1 = s->fixed_coeffs[2][i];
                s->fixed_coeffs[1][i] = tmp0 + tmp1;
                s->fixed_coeffs[2][i] = tmp0 - tmp1;
            }
        }
    }
}

/**
 * Inverse MDCT Transform.
 * Convert frequency domain coefficients to time-domain audio samples.
 * reference: Section 7.9.4 Transformation Equations
 */
static inline void do_imdct(AC3DecodeContext *s, int channels)
{
    int ch;
    float add_bias = s->add_bias;
    if(s->out_channels==1 && channels>1)
        add_bias *= LEVEL_MINUS_3DB; // compensate for the gain in downmix

    for (ch=1; ch<=channels; ch++) {
        if (s->block_switch[ch]) {
            int i;
            float *x = s->tmp_output+128;
            for(i=0; i<128; i++)
                x[i] = s->transform_coeffs[ch][2*i];
            ff_imdct_half(&s->imdct_256, s->tmp_output, x);
            s->dsp.vector_fmul_window(s->output[ch-1], s->delay[ch-1], s->tmp_output, s->window, add_bias, 128);
            for(i=0; i<128; i++)
                x[i] = s->transform_coeffs[ch][2*i+1];
            ff_imdct_half(&s->imdct_256, s->delay[ch-1], x);
        } else {
            ff_imdct_half(&s->imdct_512, s->tmp_output, s->transform_coeffs[ch]);
            s->dsp.vector_fmul_window(s->output[ch-1], s->delay[ch-1], s->tmp_output, s->window, add_bias, 128);
            memcpy(s->delay[ch-1], s->tmp_output+128, 128*sizeof(float));
        }
    }
}

/**
 * Downmix the output to mono or stereo.
 */
void ff_ac3_downmix_c(float (*samples)[256], float (*matrix)[2], int out_ch, int in_ch, int len)
{
    int i, j;
    float v0, v1;
    if(out_ch == 2) {
        for(i=0; i<len; i++) {
            v0 = v1 = 0.0f;
            for(j=0; j<in_ch; j++) {
                v0 += samples[j][i] * matrix[j][0];
                v1 += samples[j][i] * matrix[j][1];
            }
            samples[0][i] = v0;
            samples[1][i] = v1;
        }
    } else if(out_ch == 1) {
        for(i=0; i<len; i++) {
            v0 = 0.0f;
            for(j=0; j<in_ch; j++)
                v0 += samples[j][i] * matrix[j][0];
            samples[0][i] = v0;
        }
    }
}

/**
 * Upmix delay samples from stereo to original channel layout.
 */
static void ac3_upmix_delay(AC3DecodeContext *s)
{
    int channel_data_size = sizeof(s->delay[0]);
    switch(s->channel_mode) {
        case AC3_CHMODE_DUALMONO:
        case AC3_CHMODE_STEREO:
            /* upmix mono to stereo */
            memcpy(s->delay[1], s->delay[0], channel_data_size);
            break;
        case AC3_CHMODE_2F2R:
            memset(s->delay[3], 0, channel_data_size);
        case AC3_CHMODE_2F1R:
            memset(s->delay[2], 0, channel_data_size);
            break;
        case AC3_CHMODE_3F2R:
            memset(s->delay[4], 0, channel_data_size);
        case AC3_CHMODE_3F1R:
            memset(s->delay[3], 0, channel_data_size);
        case AC3_CHMODE_3F:
            memcpy(s->delay[2], s->delay[1], channel_data_size);
            memset(s->delay[1], 0, channel_data_size);
            break;
    }
}

/**
 * Decode band structure for coupling, spectral extension, or enhanced coupling.
 * @param[in] gbc bit reader context
 * @param[in] blk block number
 * @param[in] eac3 flag to indicate E-AC-3
 * @param[in] ecpl flag to indicate enhanced coupling
 * @param[in] start_subband subband number for start of range
 * @param[in] end_subband subband number for end of range
 * @param[in] default_band_struct default band structure table
 * @param[out] band_struct decoded band structure
 * @param[out] num_subbands number of subbands (optionally NULL)
 * @param[out] num_bands number of bands (optionally NULL)
 * @param[out] band_sizes array containing the number of bins in each band (optionally NULL)
 */
static void decode_band_structure(GetBitContext *gbc, int blk, int eac3,
                                  int ecpl, int start_subband, int end_subband,
                                  const uint8_t *default_band_struct,
                                  uint8_t *band_struct, int *num_subbands,
                                  int *num_bands, uint8_t *band_sizes)
{
    int subbnd, bnd, n_subbands, n_bands=0;
    uint8_t bnd_sz[22];

    n_subbands = end_subband - start_subband;

    /* decode band structure from bitstream or use default */
    if (!eac3 || get_bits1(gbc)) {
        for (subbnd = 0; subbnd < n_subbands - 1; subbnd++) {
            band_struct[subbnd] = get_bits1(gbc);
        }
    } else if (!blk) {
        memcpy(band_struct,
               &default_band_struct[start_subband+1],
               n_subbands-1);
    }
    band_struct[n_subbands-1] = 0;

    /* calculate number of bands and band sizes based on band structure.
       note that the first 4 subbands in enhanced coupling span only 6 bins
       instead of 12. */
    if (num_bands || band_sizes ) {
        n_bands = n_subbands;
        bnd_sz[0] = ecpl ? 6 : 12;
        for (bnd = 0, subbnd = 1; subbnd < n_subbands; subbnd++) {
            int subbnd_size = (ecpl && subbnd < 4) ? 6 : 12;
            if (band_struct[subbnd-1]) {
                n_bands--;
                bnd_sz[bnd] += subbnd_size;
            } else {
                bnd_sz[++bnd] = subbnd_size;
            }
        }
    }

    /* set optional output params */
    if (num_subbands)
        *num_subbands = n_subbands;
    if (num_bands)
        *num_bands = n_bands;
    if (band_sizes)
        memcpy(band_sizes, bnd_sz, n_bands);
}

/**
 * Decode a single audio block from the AC-3 bitstream.
 */
static int decode_audio_block(AC3DecodeContext *s, int blk)
{
    int fbw_channels = s->fbw_channels;
    int channel_mode = s->channel_mode;
    int i, bnd, seg, ch;
    int different_transforms;
    int downmix_output;
    int cpl_in_use;
    GetBitContext *gbc = &s->gbc;
    uint8_t bit_alloc_stages[AC3_MAX_CHANNELS];

    memset(bit_alloc_stages, 0, AC3_MAX_CHANNELS);

    /* block switch flags */
    different_transforms = 0;
    if (s->block_switch_syntax) {
        for (ch = 1; ch <= fbw_channels; ch++) {
            s->block_switch[ch] = get_bits1(gbc);
            if(ch > 1 && s->block_switch[ch] != s->block_switch[1])
                different_transforms = 1;
        }
    }

    /* dithering flags */
    if (s->dither_flag_syntax) {
        for (ch = 1; ch <= fbw_channels; ch++) {
            s->dither_flag[ch] = get_bits1(gbc);
        }
    }

    /* dynamic range */
    i = !(s->channel_mode);
    do {
        if(get_bits1(gbc)) {
            s->dynamic_range[i] = ((dynamic_range_tab[get_bits(gbc, 8)]-1.0) *
                                  s->avctx->drc_scale)+1.0;
        } else if(blk == 0) {
            s->dynamic_range[i] = 1.0f;
        }
    } while(i--);

    /* spectral extension strategy */
    if (s->eac3 && (!blk || get_bits1(gbc))) {
        if (get_bits1(gbc)) {
            av_log_missing_feature(s->avctx, "Spectral extension", 1);
            return -1;
        }
        /* TODO: parse spectral extension strategy info */
    }

    /* TODO: spectral extension coordinates */

    /* coupling strategy */
    if (s->eac3 ? s->cpl_strategy_exists[blk] : get_bits1(gbc)) {
        memset(bit_alloc_stages, 3, AC3_MAX_CHANNELS);
        if (!s->eac3)
            s->cpl_in_use[blk] = get_bits1(gbc);
        if (s->cpl_in_use[blk]) {
            /* coupling in use */
            int cpl_start_subband, cpl_end_subband;

            if (channel_mode < AC3_CHMODE_STEREO) {
                av_log(s->avctx, AV_LOG_ERROR, "coupling not allowed in mono or dual-mono\n");
                return -1;
            }

            /* check for enhanced coupling */
            if (s->eac3 && get_bits1(gbc)) {
                /* TODO: parse enhanced coupling strategy info */
                av_log_missing_feature(s->avctx, "Enhanced coupling", 1);
                return -1;
            }

            /* determine which channels are coupled */
            if (s->eac3 && s->channel_mode == AC3_CHMODE_STEREO) {
                s->channel_in_cpl[1] = 1;
                s->channel_in_cpl[2] = 1;
            } else {
                for (ch = 1; ch <= fbw_channels; ch++)
                    s->channel_in_cpl[ch] = get_bits1(gbc);
            }

            /* phase flags in use */
            if (channel_mode == AC3_CHMODE_STEREO)
                s->phase_flags_in_use = get_bits1(gbc);

            /* coupling frequency range */
            /* TODO: modify coupling end freq if spectral extension is used */
            cpl_start_subband = get_bits(gbc, 4);
            cpl_end_subband   = get_bits(gbc, 4) + 3;
            s->num_cpl_subbands = cpl_end_subband - cpl_start_subband;
            if (s->num_cpl_subbands < 0) {
                av_log(s->avctx, AV_LOG_ERROR, "invalid coupling range (%d > %d)\n",
                       cpl_start_subband, cpl_end_subband);
                return -1;
            }
            s->start_freq[CPL_CH] = cpl_start_subband * 12 + 37;
            s->end_freq[CPL_CH]   = cpl_end_subband   * 12 + 37;

           decode_band_structure(gbc, blk, s->eac3, 0,
                                 cpl_start_subband, cpl_end_subband,
                                 ff_eac3_default_cpl_band_struct,
                                 s->cpl_band_struct, &s->num_cpl_subbands,
                                 &s->num_cpl_bands, NULL);
        } else {
            /* coupling not in use */
            for (ch = 1; ch <= fbw_channels; ch++) {
                s->channel_in_cpl[ch] = 0;
                s->first_cpl_coords[ch] = 1;
            }
            s->first_cpl_leak = s->eac3;
            s->phase_flags_in_use = 0;
        }
    } else if (!s->eac3) {
        if(!blk) {
            av_log(s->avctx, AV_LOG_ERROR, "new coupling strategy must be present in block 0\n");
            return -1;
        } else {
            s->cpl_in_use[blk] = s->cpl_in_use[blk-1];
        }
    }
    cpl_in_use = s->cpl_in_use[blk];

    /* coupling coordinates */
    if (cpl_in_use) {
        int cpl_coords_exist = 0;

        for (ch = 1; ch <= fbw_channels; ch++) {
            if (s->channel_in_cpl[ch]) {
                if ((s->eac3 && s->first_cpl_coords[ch]) || get_bits1(gbc)) {
                    int master_cpl_coord, cpl_coord_exp, cpl_coord_mant;
                    s->first_cpl_coords[ch] = 0;
                    cpl_coords_exist = 1;
                    master_cpl_coord = 3 * get_bits(gbc, 2);
                    for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
                        cpl_coord_exp = get_bits(gbc, 4);
                        cpl_coord_mant = get_bits(gbc, 4);
                        if (cpl_coord_exp == 15)
                            s->cpl_coords[ch][bnd] = cpl_coord_mant << 22;
                        else
                            s->cpl_coords[ch][bnd] = (cpl_coord_mant + 16) << 21;
                        s->cpl_coords[ch][bnd] >>= (cpl_coord_exp + master_cpl_coord);
                    }
                } else if (!blk) {
                    av_log(s->avctx, AV_LOG_ERROR, "new coupling coordinates must be present in block 0\n");
                    return -1;
                }
            } else {
                /* channel not in coupling */
                s->first_cpl_coords[ch] = 1;
            }
        }
        /* phase flags */
        if (channel_mode == AC3_CHMODE_STEREO && cpl_coords_exist) {
            for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
                s->phase_flags[bnd] = s->phase_flags_in_use? get_bits1(gbc) : 0;
            }
        }
    }

    /* stereo rematrixing strategy and band structure */
    if (channel_mode == AC3_CHMODE_STEREO) {
        if ((s->eac3 && !blk) || get_bits1(gbc)) {
            s->num_rematrixing_bands = 4;
            if(cpl_in_use && s->start_freq[CPL_CH] <= 61)
                s->num_rematrixing_bands -= 1 + (s->start_freq[CPL_CH] == 37);
            for(bnd=0; bnd<s->num_rematrixing_bands; bnd++)
                s->rematrixing_flags[bnd] = get_bits1(gbc);
        } else if (!blk) {
            av_log(s->avctx, AV_LOG_ERROR, "new rematrixing strategy must be present in block 0\n");
            return -1;
        }
    }

    /* exponent strategies for each channel */
    for (ch = !cpl_in_use; ch <= s->channels; ch++) {
        if (!s->eac3)
            s->exp_strategy[blk][ch] = get_bits(gbc, 2 - (ch == s->lfe_ch));
        if(s->exp_strategy[blk][ch] != EXP_REUSE)
            bit_alloc_stages[ch] = 3;
    }

    /* channel bandwidth */
    for (ch = 1; ch <= fbw_channels; ch++) {
        s->start_freq[ch] = 0;
        if (s->exp_strategy[blk][ch] != EXP_REUSE) {
            int group_size;
            int prev = s->end_freq[ch];
            if (s->channel_in_cpl[ch])
                s->end_freq[ch] = s->start_freq[CPL_CH];
            else {
                int bandwidth_code = get_bits(gbc, 6);
                if (bandwidth_code > 60) {
                    av_log(s->avctx, AV_LOG_ERROR, "bandwidth code = %d > 60\n", bandwidth_code);
                    return -1;
                }
                s->end_freq[ch] = bandwidth_code * 3 + 73;
            }
            group_size = 3 << (s->exp_strategy[blk][ch] - 1);
            s->num_exp_groups[ch] = (s->end_freq[ch]+group_size-4) / group_size;
            if(blk > 0 && s->end_freq[ch] != prev)
                memset(bit_alloc_stages, 3, AC3_MAX_CHANNELS);
        }
    }
    if (cpl_in_use && s->exp_strategy[blk][CPL_CH] != EXP_REUSE) {
        s->num_exp_groups[CPL_CH] = (s->end_freq[CPL_CH] - s->start_freq[CPL_CH]) /
                                    (3 << (s->exp_strategy[blk][CPL_CH] - 1));
    }

    /* decode exponents for each channel */
    for (ch = !cpl_in_use; ch <= s->channels; ch++) {
        if (s->exp_strategy[blk][ch] != EXP_REUSE) {
            s->dexps[ch][0] = get_bits(gbc, 4) << !ch;
            decode_exponents(gbc, s->exp_strategy[blk][ch],
                             s->num_exp_groups[ch], s->dexps[ch][0],
                             &s->dexps[ch][s->start_freq[ch]+!!ch]);
            if(ch != CPL_CH && ch != s->lfe_ch)
                skip_bits(gbc, 2); /* skip gainrng */
        }
    }

    /* bit allocation information */
    if (s->bit_allocation_syntax) {
        if (get_bits1(gbc)) {
            s->bit_alloc_params.slow_decay = ff_ac3_slow_decay_tab[get_bits(gbc, 2)] >> s->bit_alloc_params.sr_shift;
            s->bit_alloc_params.fast_decay = ff_ac3_fast_decay_tab[get_bits(gbc, 2)] >> s->bit_alloc_params.sr_shift;
            s->bit_alloc_params.slow_gain  = ff_ac3_slow_gain_tab[get_bits(gbc, 2)];
            s->bit_alloc_params.db_per_bit = ff_ac3_db_per_bit_tab[get_bits(gbc, 2)];
            s->bit_alloc_params.floor  = ff_ac3_floor_tab[get_bits(gbc, 3)];
            for(ch=!cpl_in_use; ch<=s->channels; ch++)
                bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
        } else if (!blk) {
            av_log(s->avctx, AV_LOG_ERROR, "new bit allocation info must be present in block 0\n");
            return -1;
        }
    }

    /* signal-to-noise ratio offsets and fast gains (signal-to-mask ratios) */
    if(!s->eac3 || !blk){
        if(s->snr_offset_strategy && get_bits1(gbc)) {
            int snr = 0;
            int csnr;
            csnr = (get_bits(gbc, 6) - 15) << 4;
            for (i = ch = !cpl_in_use; ch <= s->channels; ch++) {
                /* snr offset */
                if (ch == i || s->snr_offset_strategy == 2)
                    snr = (csnr + get_bits(gbc, 4)) << 2;
                /* run at least last bit allocation stage if snr offset changes */
                if(blk && s->snr_offset[ch] != snr) {
                    bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 1);
                }
                s->snr_offset[ch] = snr;

                /* fast gain (normal AC-3 only) */
                if (!s->eac3) {
                    int prev = s->fast_gain[ch];
                    s->fast_gain[ch] = ff_ac3_fast_gain_tab[get_bits(gbc, 3)];
                    /* run last 2 bit allocation stages if fast gain changes */
                    if(blk && prev != s->fast_gain[ch])
                        bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
                }
            }
        } else if (!s->eac3 && !blk) {
            av_log(s->avctx, AV_LOG_ERROR, "new snr offsets must be present in block 0\n");
            return -1;
        }
    }

    /* fast gain (E-AC-3 only) */
    if (s->fast_gain_syntax && get_bits1(gbc)) {
        for (ch = !cpl_in_use; ch <= s->channels; ch++) {
            int prev = s->fast_gain[ch];
            s->fast_gain[ch] = ff_ac3_fast_gain_tab[get_bits(gbc, 3)];
            /* run last 2 bit allocation stages if fast gain changes */
            if(blk && prev != s->fast_gain[ch])
                bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
        }
    } else if (s->eac3 && !blk) {
        for (ch = !cpl_in_use; ch <= s->channels; ch++)
            s->fast_gain[ch] = ff_ac3_fast_gain_tab[4];
    }

    /* E-AC-3 to AC-3 converter SNR offset */
    if (s->frame_type == EAC3_FRAME_TYPE_INDEPENDENT && get_bits1(gbc)) {
        skip_bits(gbc, 10); // skip converter snr offset
    }

    /* coupling leak information */
    if (cpl_in_use) {
        if (s->first_cpl_leak || get_bits1(gbc)) {
            int fl = get_bits(gbc, 3);
            int sl = get_bits(gbc, 3);
            /* run last 2 bit allocation stages for coupling channel if
               coupling leak changes */
            if(blk && (fl != s->bit_alloc_params.cpl_fast_leak ||
                       sl != s->bit_alloc_params.cpl_slow_leak)) {
                bit_alloc_stages[CPL_CH] = FFMAX(bit_alloc_stages[CPL_CH], 2);
            }
            s->bit_alloc_params.cpl_fast_leak = fl;
            s->bit_alloc_params.cpl_slow_leak = sl;
        } else if (!s->eac3 && !blk) {
            av_log(s->avctx, AV_LOG_ERROR, "new coupling leak info must be present in block 0\n");
            return -1;
        }
        s->first_cpl_leak = 0;
    }

    /* delta bit allocation information */
    if (s->dba_syntax && get_bits1(gbc)) {
        /* delta bit allocation exists (strategy) */
        for (ch = !cpl_in_use; ch <= fbw_channels; ch++) {
            s->dba_mode[ch] = get_bits(gbc, 2);
            if (s->dba_mode[ch] == DBA_RESERVED) {
                av_log(s->avctx, AV_LOG_ERROR, "delta bit allocation strategy reserved\n");
                return -1;
            }
            bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
        }
        /* channel delta offset, len and bit allocation */
        for (ch = !cpl_in_use; ch <= fbw_channels; ch++) {
            if (s->dba_mode[ch] == DBA_NEW) {
                s->dba_nsegs[ch] = get_bits(gbc, 3);
                for (seg = 0; seg <= s->dba_nsegs[ch]; seg++) {
                    s->dba_offsets[ch][seg] = get_bits(gbc, 5);
                    s->dba_lengths[ch][seg] = get_bits(gbc, 4);
                    s->dba_values[ch][seg] = get_bits(gbc, 3);
                }
                /* run last 2 bit allocation stages if new dba values */
                bit_alloc_stages[ch] = FFMAX(bit_alloc_stages[ch], 2);
            }
        }
    } else if(blk == 0) {
        for(ch=0; ch<=s->channels; ch++) {
            s->dba_mode[ch] = DBA_NONE;
        }
    }

    /* Bit allocation */
    for(ch=!cpl_in_use; ch<=s->channels; ch++) {
        if(bit_alloc_stages[ch] > 2) {
            /* Exponent mapping into PSD and PSD integration */
            ff_ac3_bit_alloc_calc_psd(s->dexps[ch],
                                      s->start_freq[ch], s->end_freq[ch],
                                      s->psd[ch], s->band_psd[ch]);
        }
        if(bit_alloc_stages[ch] > 1) {
            /* Compute excitation function, Compute masking curve, and
               Apply delta bit allocation */
            ff_ac3_bit_alloc_calc_mask(&s->bit_alloc_params, s->band_psd[ch],
                                       s->start_freq[ch], s->end_freq[ch],
                                       s->fast_gain[ch], (ch == s->lfe_ch),
                                       s->dba_mode[ch], s->dba_nsegs[ch],
                                       s->dba_offsets[ch], s->dba_lengths[ch],
                                       s->dba_values[ch], s->mask[ch]);
        }
        if(bit_alloc_stages[ch] > 0) {
            /* Compute bit allocation */
            const uint8_t *bap_tab = s->channel_uses_aht[ch] ?
                                     ff_eac3_hebap_tab : ff_ac3_bap_tab;
            ff_ac3_bit_alloc_calc_bap(s->mask[ch], s->psd[ch],
                                      s->start_freq[ch], s->end_freq[ch],
                                      s->snr_offset[ch],
                                      s->bit_alloc_params.floor,
                                      bap_tab, s->bap[ch]);
        }
    }

    /* unused dummy data */
    if (s->skip_syntax && get_bits1(gbc)) {
        int skipl = get_bits(gbc, 9);
        while(skipl--)
            skip_bits(gbc, 8);
    }

    /* unpack the transform coefficients
       this also uncouples channels if coupling is in use. */
    decode_transform_coeffs(s, blk);

    /* TODO: generate enhanced coupling coordinates and uncouple */

    /* TODO: apply spectral extension */

    /* recover coefficients if rematrixing is in use */
    if(s->channel_mode == AC3_CHMODE_STEREO)
        do_rematrixing(s);

    /* apply scaling to coefficients (headroom, dynrng) */
    for(ch=1; ch<=s->channels; ch++) {
        float gain = s->mul_bias / 4194304.0f;
        if(s->channel_mode == AC3_CHMODE_DUALMONO) {
            gain *= s->dynamic_range[ch-1];
        } else {
            gain *= s->dynamic_range[0];
        }
        s->dsp.int32_to_float_fmul_scalar(s->transform_coeffs[ch], s->fixed_coeffs[ch], gain, 256);
    }

    /* downmix and MDCT. order depends on whether block switching is used for
       any channel in this block. this is because coefficients for the long
       and short transforms cannot be mixed. */
    downmix_output = s->channels != s->out_channels &&
                     !((s->output_mode & AC3_OUTPUT_LFEON) &&
                     s->fbw_channels == s->out_channels);
    if(different_transforms) {
        /* the delay samples have already been downmixed, so we upmix the delay
           samples in order to reconstruct all channels before downmixing. */
        if(s->downmixed) {
            s->downmixed = 0;
            ac3_upmix_delay(s);
        }

        do_imdct(s, s->channels);

        if(downmix_output) {
            s->dsp.ac3_downmix(s->output, s->downmix_coeffs, s->out_channels, s->fbw_channels, 256);
        }
    } else {
        if(downmix_output) {
            s->dsp.ac3_downmix(s->transform_coeffs+1, s->downmix_coeffs, s->out_channels, s->fbw_channels, 256);
        }

        if(downmix_output && !s->downmixed) {
            s->downmixed = 1;
            s->dsp.ac3_downmix(s->delay, s->downmix_coeffs, s->out_channels, s->fbw_channels, 128);
        }

        do_imdct(s, s->out_channels);
    }

    return 0;
}

/**
 * Decode a single AC-3 frame.
 */
static int ac3_decode_frame(AVCodecContext * avctx, void *data, int *data_size,
                            const uint8_t *buf, int buf_size)
{
    AC3DecodeContext *s = avctx->priv_data;
    int16_t *out_samples = (int16_t *)data;
    int blk, ch, err;

    /* initialize the GetBitContext with the start of valid AC-3 Frame */
    if (s->input_buffer) {
        /* copy input buffer to decoder context to avoid reading past the end
           of the buffer, which can be caused by a damaged input stream. */
        memcpy(s->input_buffer, buf, FFMIN(buf_size, AC3_FRAME_BUFFER_SIZE));
        init_get_bits(&s->gbc, s->input_buffer, buf_size * 8);
    } else {
        init_get_bits(&s->gbc, buf, buf_size * 8);
    }

    /* parse the syncinfo */
    *data_size = 0;
    err = parse_frame_header(s);

    /* check that reported frame size fits in input buffer */
    if(s->frame_size > buf_size) {
        av_log(avctx, AV_LOG_ERROR, "incomplete frame\n");
        err = AC3_PARSE_ERROR_FRAME_SIZE;
    }

    /* check for crc mismatch */
    if(err != AC3_PARSE_ERROR_FRAME_SIZE && avctx->error_recognition >= FF_ER_CAREFUL) {
        if(av_crc(av_crc_get_table(AV_CRC_16_ANSI), 0, &buf[2], s->frame_size-2)) {
            av_log(avctx, AV_LOG_ERROR, "frame CRC mismatch\n");
            err = AC3_PARSE_ERROR_CRC;
        }
    }

    if(err && err != AC3_PARSE_ERROR_CRC) {
        switch(err) {
            case AC3_PARSE_ERROR_SYNC:
                av_log(avctx, AV_LOG_ERROR, "frame sync error\n");
                return -1;
            case AC3_PARSE_ERROR_BSID:
                av_log(avctx, AV_LOG_ERROR, "invalid bitstream id\n");
                break;
            case AC3_PARSE_ERROR_SAMPLE_RATE:
                av_log(avctx, AV_LOG_ERROR, "invalid sample rate\n");
                break;
            case AC3_PARSE_ERROR_FRAME_SIZE:
                av_log(avctx, AV_LOG_ERROR, "invalid frame size\n");
                break;
            case AC3_PARSE_ERROR_FRAME_TYPE:
                /* skip frame if CRC is ok. otherwise use error concealment. */
                /* TODO: add support for substreams and dependent frames */
                if(s->frame_type == EAC3_FRAME_TYPE_DEPENDENT || s->substreamid) {
                    av_log(avctx, AV_LOG_ERROR, "unsupported frame type : skipping frame\n");
                    return s->frame_size;
                } else {
                    av_log(avctx, AV_LOG_ERROR, "invalid frame type\n");
                }
                break;
            default:
                av_log(avctx, AV_LOG_ERROR, "invalid header\n");
                break;
        }
    }

    /* if frame is ok, set audio parameters */
    if (!err) {
        avctx->sample_rate = s->sample_rate;
        avctx->bit_rate = s->bit_rate;

        /* channel config */
        s->out_channels = s->channels;
        s->output_mode = s->channel_mode;
        if(s->lfe_on)
            s->output_mode |= AC3_OUTPUT_LFEON;
        if (avctx->request_channels > 0 && avctx->request_channels <= 2 &&
                avctx->request_channels < s->channels) {
            s->out_channels = avctx->request_channels;
            s->output_mode  = avctx->request_channels == 1 ? AC3_CHMODE_MONO : AC3_CHMODE_STEREO;
        }
        avctx->channels = s->out_channels;

        /* set downmixing coefficients if needed */
        if(s->channels != s->out_channels && !((s->output_mode & AC3_OUTPUT_LFEON) &&
                s->fbw_channels == s->out_channels)) {
            set_downmix_coeffs(s);
        }
    } else if (!s->out_channels) {
        s->out_channels = avctx->channels;
        if(s->out_channels < s->channels)
            s->output_mode  = s->out_channels == 1 ? AC3_CHMODE_MONO : AC3_CHMODE_STEREO;
    }

    /* decode the audio blocks */
    for (blk = 0; blk < s->num_blocks; blk++) {
        const float *output[s->out_channels];
        if (!err && decode_audio_block(s, blk)) {
            av_log(avctx, AV_LOG_ERROR, "error decoding the audio block\n");
            err = 1;
        }
        for (ch = 0; ch < s->out_channels; ch++)
            output[ch] = s->output[ch];
        s->dsp.float_to_int16_interleave(out_samples, output, 256, s->out_channels);
        out_samples += 256 * s->out_channels;
    }
    *data_size = s->num_blocks * 256 * avctx->channels * sizeof (int16_t);
    return s->frame_size;
}

/**
 * Uninitialize the AC-3 decoder.
 */
static av_cold int ac3_decode_end(AVCodecContext *avctx)
{
    AC3DecodeContext *s = avctx->priv_data;
    ff_mdct_end(&s->imdct_512);
    ff_mdct_end(&s->imdct_256);

    av_freep(&s->input_buffer);

    return 0;
}

AVCodec ac3_decoder = {
    .name = "ac3",
    .type = CODEC_TYPE_AUDIO,
    .id = CODEC_ID_AC3,
    .priv_data_size = sizeof (AC3DecodeContext),
    .init = ac3_decode_init,
    .close = ac3_decode_end,
    .decode = ac3_decode_frame,
    .long_name = NULL_IF_CONFIG_SMALL("ATSC A/52A (AC-3)"),
};

AVCodec eac3_decoder = {
    .name = "eac3",
    .type = CODEC_TYPE_AUDIO,
    .id = CODEC_ID_EAC3,
    .priv_data_size = sizeof (AC3DecodeContext),
    .init = ac3_decode_init,
    .close = ac3_decode_end,
    .decode = ac3_decode_frame,
    .long_name = NULL_IF_CONFIG_SMALL("ATSC A/52B (AC-3, E-AC-3)"),
};
