/*
 * Intel Indeo 2 codec
 * Copyright (c) 2005 Konstantin Shishkov
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
 * @file
 * Intel Indeo 2 decoder.
 */

#define BITSTREAM_READER_LE
#include "libavutil/attributes.h"
#include "avcodec.h"
#include "get_bits.h"
#include "indeo2data.h"
#include "mathops.h"

typedef struct Ir2Context{
    AVCodecContext *avctx;
    AVFrame picture;
    GetBitContext gb;
    int decode_delta;
} Ir2Context;

#define CODE_VLC_BITS 14
static VLC ir2_vlc;

/* Indeo 2 codes are in range 0x01..0x7F and 0x81..0x90 */
static inline int ir2_get_code(GetBitContext *gb)
{
    return get_vlc2(gb, ir2_vlc.table, CODE_VLC_BITS, 1) + 1;
}

static int ir2_decode_plane(Ir2Context *ctx, int width, int height, uint8_t *dst,
                            int stride, const uint8_t *table)
{
    int i;
    int j;
    int out = 0;
    int c;
    int t;

    if (width & 1)
        return AVERROR_INVALIDDATA;

    /* first line contain absolute values, other lines contain deltas */
    while (out < width) {
        c = ir2_get_code(&ctx->gb);
        if (c >= 0x80) { /* we have a run */
            c -= 0x7F;
            if (out + c*2 > width)
                return AVERROR_INVALIDDATA;
            for (i = 0; i < c * 2; i++)
                dst[out++] = 0x80;
        } else { /* copy two values from table */
            dst[out++] = table[c * 2];
            dst[out++] = table[(c * 2) + 1];
        }
    }
    dst += stride;

    for (j = 1; j < height; j++) {
        out = 0;
        while (out < width) {
            c = ir2_get_code(&ctx->gb);
            if (c >= 0x80) { /* we have a skip */
                c -= 0x7F;
                if (out + c*2 > width)
                    return AVERROR_INVALIDDATA;
                for (i = 0; i < c * 2; i++) {
                    dst[out] = dst[out - stride];
                    out++;
                }
            } else { /* add two deltas from table */
                t        = dst[out - stride] + (table[c * 2] - 128);
                t        = av_clip_uint8(t);
                dst[out] = t;
                out++;
                t        = dst[out - stride] + (table[(c * 2) + 1] - 128);
                t        = av_clip_uint8(t);
                dst[out] = t;
                out++;
            }
        }
        dst += stride;
    }
    return 0;
}

static int ir2_decode_plane_inter(Ir2Context *ctx, int width, int height, uint8_t *dst,
                                  int stride, const uint8_t *table)
{
    int j;
    int out = 0;
    int c;
    int t;

    if (width & 1)
        return AVERROR_INVALIDDATA;

    for (j = 0; j < height; j++) {
        out = 0;
        while (out < width) {
            c = ir2_get_code(&ctx->gb);
            if (c >= 0x80) { /* we have a skip */
                c   -= 0x7F;
                out += c * 2;
            } else { /* add two deltas from table */
                t        = dst[out] + (((table[c * 2] - 128)*3) >> 2);
                t        = av_clip_uint8(t);
                dst[out] = t;
                out++;
                t        = dst[out] + (((table[(c * 2) + 1] - 128)*3) >> 2);
                t        = av_clip_uint8(t);
                dst[out] = t;
                out++;
            }
        }
        dst += stride;
    }
    return 0;
}

static int ir2_decode_frame(AVCodecContext *avctx,
                        void *data, int *got_frame,
                        AVPacket *avpkt)
{
    Ir2Context * const s = avctx->priv_data;
    const uint8_t *buf   = avpkt->data;
    int buf_size         = avpkt->size;
    AVFrame *picture     = data;
    AVFrame * const p    = &s->picture;
    int start, ret;

    p->reference = 3;
    p->buffer_hints = FF_BUFFER_HINTS_VALID | FF_BUFFER_HINTS_PRESERVE | FF_BUFFER_HINTS_REUSABLE;
    if ((ret = avctx->reget_buffer(avctx, p)) < 0) {
        av_log(s->avctx, AV_LOG_ERROR, "reget_buffer() failed\n");
        return ret;
    }

    start = 48; /* hardcoded for now */

    if (start >= buf_size) {
        av_log(s->avctx, AV_LOG_ERROR, "input buffer size too small (%d)\n", buf_size);
        return AVERROR_INVALIDDATA;
    }

    s->decode_delta = buf[18];

    /* decide whether frame uses deltas or not */
#ifndef BITSTREAM_READER_LE
    for (i = 0; i < buf_size; i++)
        buf[i] = ff_reverse[buf[i]];
#endif

    init_get_bits(&s->gb, buf + start, (buf_size - start) * 8);

    if (s->decode_delta) { /* intraframe */
        if ((ret = ir2_decode_plane(s, avctx->width, avctx->height,
                                    s->picture.data[0], s->picture.linesize[0],
                                    ir2_luma_table)) < 0)
            return ret;

        /* swapped U and V */
        if ((ret = ir2_decode_plane(s, avctx->width >> 2, avctx->height >> 2,
                                    s->picture.data[2], s->picture.linesize[2],
                                    ir2_luma_table)) < 0)
            return ret;
        if ((ret = ir2_decode_plane(s, avctx->width >> 2, avctx->height >> 2,
                                    s->picture.data[1], s->picture.linesize[1],
                                    ir2_luma_table)) < 0)
            return ret;
    } else { /* interframe */
        if ((ret = ir2_decode_plane_inter(s, avctx->width, avctx->height,
                                          s->picture.data[0], s->picture.linesize[0],
                                          ir2_luma_table)) < 0)
            return ret;
        /* swapped U and V */
        if ((ret = ir2_decode_plane_inter(s, avctx->width >> 2, avctx->height >> 2,
                                          s->picture.data[2], s->picture.linesize[2],
                                          ir2_luma_table)) < 0)
            return ret;
        if ((ret = ir2_decode_plane_inter(s, avctx->width >> 2, avctx->height >> 2,
                                          s->picture.data[1], s->picture.linesize[1],
                                          ir2_luma_table)) < 0)
            return ret;
    }

    *picture   = s->picture;
    *got_frame = 1;

    return buf_size;
}

static av_cold int ir2_decode_init(AVCodecContext *avctx)
{
    Ir2Context * const ic = avctx->priv_data;
    static VLC_TYPE vlc_tables[1 << CODE_VLC_BITS][2];

    avcodec_get_frame_defaults(&ic->picture);
    ic->avctx = avctx;

    avctx->pix_fmt= AV_PIX_FMT_YUV410P;

    ir2_vlc.table = vlc_tables;
    ir2_vlc.table_allocated = 1 << CODE_VLC_BITS;
#ifdef BITSTREAM_READER_LE
        init_vlc(&ir2_vlc, CODE_VLC_BITS, IR2_CODES,
                 &ir2_codes[0][1], 4, 2,
                 &ir2_codes[0][0], 4, 2, INIT_VLC_USE_NEW_STATIC | INIT_VLC_LE);
#else
        init_vlc(&ir2_vlc, CODE_VLC_BITS, IR2_CODES,
                 &ir2_codes[0][1], 4, 2,
                 &ir2_codes[0][0], 4, 2, INIT_VLC_USE_NEW_STATIC);
#endif

    return 0;
}

static av_cold int ir2_decode_end(AVCodecContext *avctx)
{
    Ir2Context * const ic = avctx->priv_data;
    AVFrame *pic = &ic->picture;

    if (pic->data[0])
        avctx->release_buffer(avctx, pic);

    return 0;
}

AVCodec ff_indeo2_decoder = {
    .name           = "indeo2",
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_INDEO2,
    .priv_data_size = sizeof(Ir2Context),
    .init           = ir2_decode_init,
    .close          = ir2_decode_end,
    .decode         = ir2_decode_frame,
    .capabilities   = CODEC_CAP_DR1,
    .long_name      = NULL_IF_CONFIG_SMALL("Intel Indeo 2"),
};
